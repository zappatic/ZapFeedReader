/*
    ZapFeedReader - RSS/Atom feed reader
    Copyright (C) 2023-present  Kasper Nauwelaerts (zapfr at zappatic dot net)

    ZapFeedReader is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ZapFeedReader is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ZapFeedReader.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <Poco/Data/RecordSet.h>

#include "ZapFR/Database.h"
#include "ZapFR/Helpers.h"
#include "ZapFR/local/PostLocal.h"
#include "ZapFR/local/ScriptFolderLocal.h"

using namespace Poco::Data::Keywords;

ZapFR::Engine::ScriptFolderLocal::ScriptFolderLocal(uint64_t id, Source* parentSource) : ScriptFolder(id, parentSource)
{
}

std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Post>>> ZapFR::Engine::ScriptFolderLocal::getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                                                                                                   const std::string& searchFilter, FlagColor flagColor)
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindingsPostQuery;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindingsCountQuery;
    std::string wildcardSearchFilter = "%" + searchFilter + "%";
    auto fc = Flag::idForFlagColor(flagColor);

    whereClause.emplace_back("posts.id IN (SELECT DISTINCT(postID) FROM scriptfolder_posts WHERE scriptfolder_posts.scriptfolderID=?)");
    bindingsPostQuery.emplace_back(useRef(mID, "scriptFolderID"));
    bindingsCountQuery.emplace_back(useRef(mID, "scriptFolderID"));

    if (showOnlyUnread)
    {
        whereClause.emplace_back("posts.isRead=FALSE");
    }
    if (!searchFilter.empty())
    {
        whereClause.emplace_back("(posts.title LIKE ? OR posts.content LIKE ?)");
        bindingsPostQuery.emplace_back(useRef(wildcardSearchFilter, "searchFilter"));
        bindingsPostQuery.emplace_back(useRef(wildcardSearchFilter, "searchFilter"));
        bindingsCountQuery.emplace_back(useRef(wildcardSearchFilter, "searchFilter"));
        bindingsCountQuery.emplace_back(useRef(wildcardSearchFilter, "searchFilter"));
    }
    if (flagColor != FlagColor::Gray)
    {
        whereClause.emplace_back("posts.id IN (SELECT DISTINCT(postID) FROM flags WHERE flagID=?)");
        bindingsPostQuery.emplace_back(use(fc, "flagColor"));
        bindingsCountQuery.emplace_back(use(fc, "flagColor"));
    }

    auto offset = perPage * (page - 1);
    bindingsPostQuery.emplace_back(use(perPage, "perPage"));
    bindingsPostQuery.emplace_back(use(offset, "offset"));

    auto posts = PostLocal::queryMultiple(whereClause, "ORDER BY posts.datePublished DESC", "LIMIT ? OFFSET ?", bindingsPostQuery);
    auto count = PostLocal::queryCount(whereClause, bindingsCountQuery);
    return std::make_tuple(count, std::move(posts));
}

void ZapFR::Engine::ScriptFolderLocal::update(const std::string& title, bool showTotal, bool showUnread)
{
    Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
    updateStmt << "UPDATE scriptfolders SET title=?, showTotal=?, showUnread=? WHERE id=?", useRef(title), use(showTotal), use(showUnread), use(mID), now;
}

void ZapFR::Engine::ScriptFolderLocal::fetchThumbnailData()
{
    mThumbnailData.clear();

    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;

    whereClause.emplace_back("posts.id IN (SELECT DISTINCT(postID) FROM scriptfolder_posts WHERE scriptfolder_posts.scriptfolderID=?)");
    whereClause.emplace_back("posts.isRead=FALSE");
    whereClause.emplace_back("posts.thumbnail NOT NULL");

    bindings.emplace_back(useRef(mID, "scriptFolderID"));

    auto posts = PostLocal::queryMultiple(whereClause, "", "LIMIT 250", bindings);

    std::unordered_map<uint64_t, std::string> feedIDToTitleMap;
    std::unordered_map<uint64_t, std::vector<Post*>> feedIDToPosts;
    for (const auto& post : posts)
    {
        auto feedID = post->feedID();
        feedIDToTitleMap[feedID] = post->feedTitle();
        if (!feedIDToPosts.contains(feedID))
        {
            feedIDToPosts[feedID] = {};
        }
        feedIDToPosts.at(feedID).emplace_back(post.get());
    }

    // sort by title of the feed
    std::vector<std::tuple<uint64_t, std::string>> feedIDToTitleVector;
    for (const auto& [feedID, feedTitle] : feedIDToTitleMap)
    {
        feedIDToTitleVector.emplace_back(feedID, feedTitle);
    }
    std::sort(feedIDToTitleVector.begin(), feedIDToTitleVector.end(),
              [](const std::tuple<uint64_t, std::string>& a, const std::tuple<uint64_t, std::string>& b) { return Poco::icompare(std::get<1>(a), std::get<1>(b)) < 0; });

    for (const auto& [feedID, feedTitle] : feedIDToTitleVector)
    {
        ThumbnailData td;
        td.feedID = feedID;
        td.feedTitle = feedTitle;
        for (const auto& post : feedIDToPosts.at(feedID))
        {
            Poco::DateTime datePublished{};
            int32_t tzd{0};
            Poco::DateTimeParser::tryParse(post->datePublished(), datePublished, tzd);
            auto timestamp = datePublished.timestamp().epochTime();

            td.posts.emplace_back(post->id(), post->title(), post->thumbnail(), post->link(), timestamp);
        }
        std::sort(td.posts.begin(), td.posts.end(), [](const ThumbnailDataPost& a, const ThumbnailDataPost& b) { return (std::difftime(a.timestamp, b.timestamp) > 0); });
        mThumbnailData.emplace_back(td);
    }
}

std::unordered_set<uint64_t> ZapFR::Engine::ScriptFolderLocal::markAsRead()
{
    // see what feed IDs are affected
    std::unordered_set<uint64_t> affectedFeedIDs;
    uint64_t feedID{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT DISTINCT(posts.feedID) FROM posts WHERE posts.id IN (SELECT DISTINCT(postID) FROM scriptfolder_posts WHERE scriptfolder_posts.scriptfolderID=?)",
        use(mID), into(feedID), range(0, 1);
    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            affectedFeedIDs.insert(feedID);
        }
    }

    // mark the posts in the script folder as read
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;

    whereClause.emplace_back("posts.id IN (SELECT DISTINCT(postID) FROM scriptfolder_posts WHERE scriptfolder_posts.scriptfolderID=?)");
    bindings.emplace_back(useRef(mID, "scriptFolderID"));

    PostLocal::updateIsRead(true, whereClause, bindings);

    return affectedFeedIDs;
}

std::vector<std::unique_ptr<ZapFR::Engine::ScriptFolder>> ZapFR::Engine::ScriptFolderLocal::queryMultiple(Source* parentSource, const std::vector<std::string>& whereClause,
                                                                                                          const std::string& orderClause, const std::string& limitClause,
                                                                                                          const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings)
{
    std::vector<std::unique_ptr<ScriptFolder>> scriptFolders;

    uint64_t id{0};
    std::string title{""};
    bool showTotal{false};
    bool showUnread{false};

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT scriptfolders.id"
          ",scriptfolders.title"
          ",scriptfolders.showTotal"
          ",scriptfolders.showUnread"
          " FROM scriptfolders";
    if (!whereClause.empty())
    {
        ss << " WHERE ";
        ss << Helpers::joinString(whereClause, " AND ");
    }
    ss << " " << orderClause << " " << limitClause;

    auto sql = ss.str();

    selectStmt << sql, range(0, 1);

    for (const auto& binding : bindings)
    {
        selectStmt.addBind(binding);
    }

    selectStmt.addExtract(into(id));
    selectStmt.addExtract(into(title));
    selectStmt.addExtract(into(showTotal));
    selectStmt.addExtract(into(showUnread));

    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            auto sfl = std::make_unique<ScriptFolderLocal>(id, parentSource);
            sfl->setTitle(title);
            sfl->setShowTotal(showTotal);
            sfl->setShowUnread(showUnread);

            sfl->setTotalPostCount(PostLocal::queryCount({"posts.id IN (SELECT DISTINCT(postID) FROM scriptfolder_posts WHERE scriptfolder_posts.scriptfolderID=?)"},
                                                         {useRef(id, "scriptFolderID")}));
            sfl->setTotalUnreadCount(
                PostLocal::queryCount({"posts.id IN (SELECT DISTINCT(postID) FROM scriptfolder_posts WHERE scriptfolder_posts.scriptfolderID=?)", "posts.isRead=FALSE"},
                                      {useRef(id, "scriptFolderID")}));

            scriptFolders.emplace_back(std::move(sfl));
        }
    }
    return scriptFolders;
}

std::optional<std::unique_ptr<ZapFR::Engine::ScriptFolder>> ZapFR::Engine::ScriptFolderLocal::querySingle(Source* parentSource, const std::vector<std::string>& whereClause,
                                                                                                          const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings)
{
    uint64_t id{0};
    std::string title{""};
    bool showTotal{false};
    bool showUnread{false};

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT scriptfolders.id"
          ",scriptfolders.title"
          ",scriptfolders.showTotal"
          ",scriptfolders.showUnread"
          " FROM scriptfolders";
    if (!whereClause.empty())
    {
        ss << " WHERE ";
        ss << Helpers::joinString(whereClause, " AND ");
    }

    auto sql = ss.str();

    selectStmt << sql, range(0, 1);

    for (const auto& binding : bindings)
    {
        selectStmt.addBind(binding);
    }

    selectStmt.addExtract(into(id));
    selectStmt.addExtract(into(title));
    selectStmt.addExtract(into(showTotal));
    selectStmt.addExtract(into(showUnread));

    selectStmt.execute();

    auto rs = Poco::Data::RecordSet(selectStmt);
    if (rs.rowCount() == 1)
    {
        auto sfl = std::make_unique<ScriptFolderLocal>(id, parentSource);
        sfl->setTitle(title);
        sfl->setShowTotal(showTotal);
        sfl->setShowUnread(showUnread);

        sfl->setTotalPostCount(PostLocal::queryCount({"posts.id IN (SELECT DISTINCT(postID) FROM scriptfolder_posts WHERE scriptfolder_posts.scriptfolderID=?)"},
                                                     {useRef(id, "scriptFolderID")}));
        sfl->setTotalUnreadCount(
            PostLocal::queryCount({"posts.id IN (SELECT DISTINCT(postID) FROM scriptfolder_posts WHERE scriptfolder_posts.scriptfolderID=?)", "posts.isRead=FALSE"},
                                  {useRef(id, "scriptFolderID")}));
        return sfl;
    }
    return {};
}

void ZapFR::Engine::ScriptFolderLocal::create(const std::string& title, bool showTotal, bool showUnread)
{
    Poco::Data::Statement insertStmt(*(Database::getInstance()->session()));
    insertStmt << "INSERT INTO scriptfolders (title, showTotal, showUnread) VALUES (?, ?, ?)", useRef(title), use(showTotal), use(showUnread), now;
}

void ZapFR::Engine::ScriptFolderLocal::remove(uint64_t scriptFolderID)
{
    {
        Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
        deleteStmt << "DELETE FROM scriptfolders WHERE id=?", use(scriptFolderID), now;
    }

    {
        Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
        deleteStmt << "DELETE FROM scriptfolder_posts WHERE scriptFolderID=?", use(scriptFolderID), now;
    }
}
