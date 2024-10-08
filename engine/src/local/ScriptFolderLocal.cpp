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
                                                                                                                   bool showUnreadPostsAtTop, const std::string& searchFilter,
                                                                                                                   uint64_t categoryFilterID, FlagColor flagColor)
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
    if (categoryFilterID != 0)
    {
        const auto& cat = Category::querySingle({"categories.id=?"}, {use(categoryFilterID, "id")});
        if (cat.has_value())
        {
            const auto& catIDs = Category::getMatchingCategories(cat.value()->title());
            if (!catIDs.empty())
            {
                auto joinedCatIDs = Helpers::joinIDNumbers(catIDs, ",");
                whereClause.emplace_back(Poco::format("posts.id IN (SELECT DISTINCT(postID) FROM post_categories WHERE categoryID IN (%s))", joinedCatIDs));
            }
        }
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

    std::string orderClause = "ORDER BY posts.datePublished DESC";
    if (showUnreadPostsAtTop)
    {
        orderClause = "ORDER BY posts.isRead ASC, posts.datePublished DESC";
    }

    auto posts = PostLocal::queryMultiple(whereClause, orderClause, "LIMIT ? OFFSET ?", bindingsPostQuery);
    auto count = PostLocal::queryCount(whereClause, bindingsCountQuery);
    return std::make_tuple(count, std::move(posts));
}

std::vector<std::unique_ptr<ZapFR::Engine::Category>> ZapFR::Engine::ScriptFolderLocal::getCategories()
{
    std::vector<uint64_t> catIDsForPosts;
    uint64_t catID;
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT DISTINCT(categoryID) FROM post_categories WHERE postID IN( SELECT postID FROM scriptfolder_posts WHERE scriptfolder_posts.scriptFolderID=?)",
        use(mID), into(catID), range(0, 1);
    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            catIDsForPosts.emplace_back(catID);
        }
    }

    if (!catIDsForPosts.empty())
    {
        auto joinedCatIDs = Helpers::joinIDNumbers(catIDsForPosts, ",");
        return Category::queryMultiple(true, {Poco::format("categories.id IN (%s)", joinedCatIDs)}, "ORDER BY categories.title ASC", "", {});
    }
    return {};
}

void ZapFR::Engine::ScriptFolderLocal::update(const std::string& title, bool showTotal, bool showUnread)
{
    Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
    updateStmt << "UPDATE scriptfolders SET title=?, showTotal=?, showUnread=? WHERE id=?", useRef(title), use(showTotal), use(showUnread), use(mID), now;
}

void ZapFR::Engine::ScriptFolderLocal::fetchThumbnailData()
{
    mThumbnailData.clear();

    // first get all feed IDs in this script folder that have unread posts with thumbnails
    std::vector<uint64_t> feedIDs;
    uint64_t fID{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT DISTINCT(feedID) FROM posts WHERE posts.thumbnail NOT NULL AND posts.isRead=FALSE AND posts.id IN (SELECT DISTINCT(postID) FROM scriptfolder_posts "
                  "WHERE scriptfolder_posts.scriptfolderID=?)",
        into(fID), use(mID), range(0, 1);
    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            feedIDs.emplace_back(fID);
        }
    }
    if (feedIDs.size() == 0)
    {
        return;
    }

    // then follow the same procedure as for a folder, with the found feedIDs
    for (auto feedID : feedIDs)
    {
        std::vector<std::string> whereClause;

        whereClause.emplace_back("posts.feedID=?");
        whereClause.emplace_back("posts.isRead=FALSE");
        whereClause.emplace_back("posts.thumbnail NOT NULL");
        whereClause.emplace_back("posts.id IN (SELECT DISTINCT(postID) FROM scriptfolder_posts WHERE scriptfolder_posts.scriptfolderID=?)");
        auto posts = PostLocal::queryMultiple(whereClause, "ORDER BY posts.datePublished DESC", "LIMIT 10",
                                              {use(feedID, "feedID"), use(mID, "scriptFolderID")}); // TODO: this limit amount needs to be configurable

        auto totalUnreadPostCount = PostLocal::queryCount(whereClause, {use(feedID, "feedID"), use(mID, "scriptFolderID")});

        if (posts.size() > 0)
        {
            ThumbnailData td;
            td.feedID = feedID;
            td.totalPostCount = totalUnreadPostCount;

            for (const auto& post : posts)
            {
                if (td.feedTitle.empty())
                {
                    td.feedTitle = post->feedTitle();
                }
                if (td.feedLink.empty())
                {
                    td.feedLink = post->feedLink();
                }

                td.posts.emplace_back(post->id(), post->title(), post->thumbnail(), post->link());
            }
            mThumbnailData.emplace_back(td);
        }
    }

    // sort by title of the feed
    std::sort(mThumbnailData.begin(), mThumbnailData.end(), [](const ThumbnailData& a, const ThumbnailData& b) { return Poco::icompare(a.feedTitle, b.feedTitle) < 0; });
}

std::vector<uint64_t> ZapFR::Engine::ScriptFolderLocal::markAsRead(uint64_t maxPostID)
{
    // see what feed IDs are affected
    auto affectedFeedIDs = getFeedIDs(maxPostID);

    // mark the posts in the script folder as read
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;

    whereClause.emplace_back("posts.id IN (SELECT DISTINCT(postID) FROM scriptfolder_posts WHERE scriptfolder_posts.scriptfolderID=?)");
    bindings.emplace_back(useRef(mID, "scriptFolderID"));

    if (maxPostID != std::numeric_limits<uint64_t>::max())
    {
        whereClause.emplace_back("posts.id <= ?");
        bindings.emplace_back(useRef(maxPostID, "maxPostID"));
    }

    PostLocal::updateIsRead(true, whereClause, bindings);

    return affectedFeedIDs;
}

std::vector<uint64_t> ZapFR::Engine::ScriptFolderLocal::getFeedIDs(uint64_t maxPostID) const
{
    std::vector<uint64_t> affectedFeedIDs;
    uint64_t feedID{0};

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT DISTINCT(posts.feedID)"
          " FROM posts"
          " WHERE posts.id IN (SELECT DISTINCT(postID) FROM scriptfolder_posts WHERE scriptfolder_posts.scriptfolderID=?)";
    if (maxPostID != std::numeric_limits<uint64_t>::max())
    {
        ss << "   AND posts.id <= ?";
    }

    selectStmt << ss.str(), range(0, 1);

    uint64_t id = mID;
    selectStmt.addBind(use(id));
    if (maxPostID != std::numeric_limits<uint64_t>::max())
    {
        selectStmt.addBind(use(maxPostID));
    }

    selectStmt.addExtract(into(feedID));

    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            affectedFeedIDs.emplace_back(feedID);
        }
    }
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
