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

#define FMT_HEADER_ONLY
#include <fmt/core.h>

#include "ZapFR/Database.h"
#include "ZapFR/Helpers.h"
#include "ZapFR/Log.h"
#include "ZapFR/OPMLParser.h"
#include "ZapFR/base/Category.h"
#include "ZapFR/feed_handling/FeedFetcher.h"
#include "ZapFR/local/FeedLocal.h"
#include "ZapFR/local/FolderLocal.h"
#include "ZapFR/local/PostLocal.h"
#include "ZapFR/local/ScriptFolderLocal.h"
#include "ZapFR/local/ScriptLocal.h"
#include "ZapFR/local/SourceLocal.h"
#include "ZapFR/lua/ScriptLua.h"

using namespace Poco::Data::Keywords;

ZapFR::Engine::SourceLocal::SourceLocal(uint64_t id) : Source(id)
{
}

/* ************************** FEED STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceLocal::getFeeds(uint32_t fetchInfo)
{
    return FeedLocal::queryMultiple(this, {}, "ORDER BY sortOrder ASC", "", {}, fetchInfo);
}

std::optional<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceLocal::getFeed(uint64_t feedID, uint32_t fetchInfo)
{
    if ((fetchInfo & FetchInfo::Data) == FetchInfo::Data || (fetchInfo & FetchInfo::Statistics) == FetchInfo::Statistics ||
        (fetchInfo & FetchInfo::FeedUnreadCount) == FetchInfo::FeedUnreadCount || (fetchInfo & FetchInfo::UnreadThumbnailData) == FetchInfo::UnreadThumbnailData)
    {
        auto feed = FeedLocal::querySingle(this, {"feeds.id=?"}, {use(feedID, "id")}, fetchInfo);
        if (feed.has_value())
        {
            auto localFeed = dynamic_cast<FeedLocal*>(feed.value().get());
            if ((fetchInfo & FetchInfo::UnreadThumbnailData) == FetchInfo::UnreadThumbnailData)
            {
                localFeed->fetchThumbnailData();
            }
        }
        return feed;
    }
    else
    {
        return std::make_unique<FeedLocal>(feedID, this);
    }
}

std::optional<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceLocal::addFeed(const std::string& url, uint64_t folder)
{
    Log::log(LogLevel::Info, fmt::format("Adding feed at {}", url));

    try
    {
        return FeedLocal::create(this, url, url, folder);
    }
    catch (const Poco::Exception& e)
    {
        Log::log(LogLevel::Error, e.displayText());
    }
    catch (const std::runtime_error& e)
    {
        Log::log(LogLevel::Error, e.what());
    }
    catch (...)
    {
        Log::log(LogLevel::Error, "Unknown exception while adding feed");
    }

    return {};
}

std::unordered_map<uint64_t, uint64_t> ZapFR::Engine::SourceLocal::moveFeed(uint64_t feedID, uint64_t newFolder, uint64_t newSortOrder)
{
    return FeedLocal::move(feedID, newFolder, newSortOrder);
}

void ZapFR::Engine::SourceLocal::removeFeed(uint64_t feedID)
{
    FeedLocal::remove(this, feedID);
}

/* ************************** FOLDER STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Folder>> ZapFR::Engine::SourceLocal::getFolders(uint64_t parent, uint32_t fetchInfo)
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;

    whereClause.emplace_back("parent=?");
    bindings.emplace_back(useRef(parent, "parent"));

    std::function<void(Folder*)> populateSubfolders;
    populateSubfolders = [&](Folder* folder)
    {
        auto localFolder = dynamic_cast<FolderLocal*>(folder);
        localFolder->fetchSubfolders();

        for (const auto& subfolder : localFolder->subfolders())
        {
            populateSubfolders(subfolder.get());
        }
    };

    auto folders = FolderLocal::queryMultiple(this, whereClause, "ORDER BY folders.sortOrder ASC", "", bindings);
    if ((fetchInfo & FetchInfo::Subfolders) == FetchInfo::Subfolders)
    {
        for (const auto& folder : folders)
        {
            populateSubfolders(folder.get());
        }
    }
    return folders;
}

std::optional<std::unique_ptr<ZapFR::Engine::Folder>> ZapFR::Engine::SourceLocal::getFolder(uint64_t folderID, uint32_t fetchInfo)
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;

    whereClause.emplace_back("id=?");
    bindings.emplace_back(useRef(folderID, "folderID"));

    auto folder = FolderLocal::querySingle(this, whereClause, bindings);
    if (folder.has_value())
    {
        auto localFolder = dynamic_cast<FolderLocal*>(folder.value().get());
        if ((fetchInfo & FetchInfo::Statistics) == FetchInfo::Statistics)
        {
            localFolder->fetchStatistics();
        }
        if ((fetchInfo & FetchInfo::FolderFeedIDs) == FetchInfo::FolderFeedIDs)
        {
            localFolder->fetchFeedIDsInFoldersAndSubfolders();
        }
        if ((fetchInfo & FetchInfo::UnreadThumbnailData) == FetchInfo::UnreadThumbnailData)
        {
            localFolder->fetchThumbnailData();
        }
    }
    return folder;
}

std::tuple<uint64_t, uint64_t> ZapFR::Engine::SourceLocal::addFolder(const std::string& title, uint64_t parentID)
{
    return FolderLocal::create(parentID, title);
}

std::unordered_map<uint64_t, uint64_t> ZapFR::Engine::SourceLocal::moveFolder(uint64_t folderID, uint64_t newParent, uint64_t newSortOrder)
{
    return FolderLocal::move(this, folderID, newParent, newSortOrder);
}

void ZapFR::Engine::SourceLocal::removeFolder(uint64_t folderID)
{
    FolderLocal::remove(this, folderID);
}

uint64_t ZapFR::Engine::SourceLocal::createFolderHierarchy(uint64_t parentID, const std::vector<std::string>& folderHierarchy)
{
    return FolderLocal::createFolderHierarchy(this, parentID, folderHierarchy);
}

/* ************************** POST STUFF ************************** */
std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Post>>> ZapFR::Engine::SourceLocal::getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                                                                                             bool showUnreadPostsAtTop, const std::string& searchFilter,
                                                                                                             uint64_t categoryFilterID, FlagColor flagColor)
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindingsPostQuery;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindingsCountQuery;
    std::string wildcardSearchFilter = "%" + searchFilter + "%";
    auto fc = Flag::idForFlagColor(flagColor);

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

void ZapFR::Engine::SourceLocal::markAsRead(uint64_t maxPostID)
{
    if (maxPostID == std::numeric_limits<uint64_t>::max())
    {
        PostLocal::updateIsRead(true, {}, {});
    }
    else
    {
        PostLocal::updateIsRead(true, {"posts.id <= ?"}, {use(maxPostID, "maxPostID")});
    }
}

void ZapFR::Engine::SourceLocal::setPostsReadStatus(bool markAsRead, const std::vector<std::tuple<uint64_t, uint64_t>>& feedsAndPostIDs)
{
    for (const auto& [feedID, posts] : remapFeedPostTuplesToMap(feedsAndPostIDs))
    {
        auto feed = getFeed(feedID, ZapFR::Engine::Source::FetchInfo::None);
        if (feed.has_value())
        {
            for (const auto& postID : posts)
            {
                auto post = feed.value()->getPost(postID);
                if (post.has_value())
                {
                    auto localPost = dynamic_cast<PostLocal*>(post.value().get());
                    if (markAsRead)
                    {
                        localPost->markAsRead();
                    }
                    else
                    {
                        localPost->markAsUnread();
                    }
                }
            }
        }
    }
}

void ZapFR::Engine::SourceLocal::setPostsFlagStatus(bool markFlagged, const std::unordered_set<FlagColor>& flagColors,
                                                    const std::vector<std::tuple<uint64_t, uint64_t>>& feedsAndPostIDs)
{
    for (const auto& [feedID, posts] : remapFeedPostTuplesToMap(feedsAndPostIDs))
    {
        auto feed = getFeed(feedID, ZapFR::Engine::Source::FetchInfo::None);
        if (feed.has_value())
        {
            for (const auto& postID : posts)
            {
                auto post = feed.value()->getPost(postID);
                if (post.has_value())
                {
                    for (const auto& fc : flagColors)
                    {
                        auto localPost = dynamic_cast<PostLocal*>(post.value().get());
                        if (markFlagged)
                        {
                            localPost->markFlagged(fc);
                        }
                        else
                        {
                            localPost->markUnflagged(fc);
                        }
                    }
                }
            }
        }
    }
}

void ZapFR::Engine::SourceLocal::assignPostsToScriptFolder(uint64_t scriptFolderID, bool assign, const std::vector<std::tuple<uint64_t, uint64_t>>& feedsAndPostIDs)
{
    auto scriptFolder = getScriptFolder(scriptFolderID, Source::FetchInfo::Data); // fetch data to ensure script folder exists
    if (!scriptFolder.has_value())
    {
        return;
    }

    for (const auto& [feedID, posts] : remapFeedPostTuplesToMap(feedsAndPostIDs))
    {
        auto feed = getFeed(feedID, ZapFR::Engine::Source::FetchInfo::None);
        for (const auto& postID : posts)
        {
            auto post = feed.value()->getPost(postID);
            if (post.has_value())
            {
                auto localPost = dynamic_cast<PostLocal*>(post.value().get());
                if (assign)
                {
                    localPost->assignToScriptFolder(scriptFolderID);
                }
                else
                {
                    localPost->unassignFromScriptFolder(scriptFolderID);
                }
            }
        }
    }
}

std::unordered_map<uint64_t, std::vector<uint64_t>>
ZapFR::Engine::SourceLocal::remapFeedPostTuplesToMap(const std::vector<std::tuple<uint64_t, uint64_t>>& feedsAndPostIDs) const
{
    std::unordered_map<uint64_t, std::vector<uint64_t>> feedsWithPostsMap;
    for (const auto& [feedID, postID] : feedsAndPostIDs)
    {
        if (feedsWithPostsMap.contains(feedID))
        {
            feedsWithPostsMap.at(feedID).emplace_back(postID);
        }
        else
        {
            std::vector<uint64_t> vec;
            vec.emplace_back(postID);
            feedsWithPostsMap[feedID] = vec;
        }
    }
    return feedsWithPostsMap;
}

std::unordered_map<uint64_t, uint64_t> ZapFR::Engine::SourceLocal::getUnreadCounts()
{
    std::unordered_map<uint64_t, uint64_t> unreadCounts;

    uint64_t feedID{0};
    uint64_t count{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT posts.feedID,COUNT(*) FROM posts WHERE posts.isRead=FALSE GROUP BY posts.feedID", into(feedID), into(count), range(0, 1);
    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            unreadCounts[feedID] = count;
        }
    }
    return unreadCounts;
}

std::unordered_map<uint64_t, std::string> ZapFR::Engine::SourceLocal::getFeedErrors()
{
    std::unordered_map<uint64_t, std::string> feedErrors;

    uint64_t feedID{0};
    std::string error{""};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT id, lastRefreshError FROM feeds WHERE lastRefreshError NOT NULL", into(feedID), into(error), range(0, 1);
    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            feedErrors[feedID] = error;
        }
    }
    return feedErrors;
}

Poco::JSON::Object ZapFR::Engine::SourceLocal::getStatus()
{
    Poco::JSON::Object o;

    const auto& unreadCounts = getUnreadCounts();
    Poco::JSON::Array unreadCountsArr;
    for (const auto& [feedID, unreadCount] : unreadCounts)
    {
        Poco::JSON::Object ucObj;
        ucObj.set(JSON::SourceStatus::FeedID, feedID);
        ucObj.set(JSON::SourceStatus::UnreadCount, unreadCount);
        unreadCountsArr.add(ucObj);
    }
    o.set(JSON::SourceStatus::UnreadCounts, unreadCountsArr);

    const auto& feedErrors = getFeedErrors();
    Poco::JSON::Array feedErrorsArr;
    for (const auto& [feedID, error] : feedErrors)
    {
        Poco::JSON::Object feObj;
        feObj.set(JSON::SourceStatus::FeedID, feedID);
        feObj.set(JSON::SourceStatus::FeedError, error);
        feedErrorsArr.add(feObj);
    }
    o.set(JSON::SourceStatus::FeedErrors, feedErrorsArr);

    o.set(JSON::SourceStatus::HighestPostID, PostLocal::highestID());

    return o;
}

void ZapFR::Engine::SourceLocal::fetchThumbnailData()
{
    mThumbnailData.clear();

    // first get all feed IDs that have unread posts with thumbnails
    std::vector<uint64_t> feedIDs;
    uint64_t fID{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT DISTINCT(feedID) FROM posts WHERE posts.thumbnail NOT NULL AND posts.isRead=FALSE", into(fID), range(0, 1);
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
        auto posts = PostLocal::queryMultiple(whereClause, "ORDER BY posts.datePublished DESC", "LIMIT 10",
                                              {use(feedID, "feedID")}); // TODO: this limit amount needs to be configurable

        auto totalUnreadPostCount = PostLocal::queryCount(whereClause, {use(feedID, "feedID")});

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

/* ************************** LOGS STUFF ************************** */
std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Log>>> ZapFR::Engine::SourceLocal::getLogs(uint64_t perPage, uint64_t page)
{
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;

    auto offset = perPage * (page - 1);
    bindings.emplace_back(use(perPage, "perPage"));
    bindings.emplace_back(use(offset, "offset"));

    auto logs = Log::queryMultiple({}, "ORDER BY logs.id DESC", "LIMIT ? OFFSET ?", bindings);
    auto logCount = Log::queryCount({}, {});
    return std::make_tuple(logCount, std::move(logs));
}

/* ************************** FLAG STUFF ************************** */
std::unordered_set<ZapFR::Engine::FlagColor> ZapFR::Engine::SourceLocal::getUsedFlagColors()
{
    std::unordered_set<FlagColor> flags;

    uint64_t fc{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT DISTINCT(flagID) FROM flags", into(fc), range(0, 1);
    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            try
            {
                auto flagColor = Flag::flagColorForID(static_cast<uint8_t>(fc));
                flags.insert(flagColor);
            }
            catch (...)
            {
                Log::log(LogLevel::Debug, fmt::format("Invalid flag color ID requested: {}", fc), {});
                // ignore non existent flag colors
            }
        }
    }

    return flags;
}

/* ************************** CATEGORY STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Category>> ZapFR::Engine::SourceLocal::getCategories()
{
    return Category::queryMultiple(true, {}, "ORDER BY categories.title ASC", "", {});
}

/* ************************** SCRIPT FOLDER STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::ScriptFolder>> ZapFR::Engine::SourceLocal::getScriptFolders()
{
    return ScriptFolderLocal::queryMultiple(this, {}, "ORDER BY scriptfolders.id DESC", "", {});
}

std::optional<std::unique_ptr<ZapFR::Engine::ScriptFolder>> ZapFR::Engine::SourceLocal::getScriptFolder(uint64_t id, uint32_t fetchInfo)
{
    if ((fetchInfo & FetchInfo::Data) == FetchInfo::Data || (fetchInfo & FetchInfo::UnreadThumbnailData) == FetchInfo::UnreadThumbnailData)
    {
        auto scriptFolder = ScriptFolderLocal::querySingle(this, {"scriptfolders.id=?"}, {use(id, "id")});
        if ((fetchInfo & FetchInfo::UnreadThumbnailData) == FetchInfo::UnreadThumbnailData)
        {
            if (scriptFolder.has_value())
            {
                auto scriptFolderLocal = dynamic_cast<ScriptFolderLocal*>(scriptFolder.value().get());
                scriptFolderLocal->fetchThumbnailData();
            }
        }
        return scriptFolder;
    }
    else
    {
        return std::make_unique<ScriptFolderLocal>(id, this);
    }
}

void ZapFR::Engine::SourceLocal::addScriptFolder(const std::string& title, bool showTotal, bool showUnread)
{
    ScriptFolderLocal::create(title, showTotal, showUnread);
}

void ZapFR::Engine::SourceLocal::removeScriptFolder(uint64_t scriptFolderID)
{
    ScriptFolderLocal::remove(scriptFolderID);
}

/* ************************** SCRIPT STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Script>> ZapFR::Engine::SourceLocal::getScripts()
{
    return ScriptLocal::queryMultiple(this, {}, "ORDER BY scripts.id DESC", "", {});
}

std::optional<std::unique_ptr<ZapFR::Engine::Script>> ZapFR::Engine::SourceLocal::getScript(uint64_t scriptID, uint32_t fetchInfo)
{
    if ((fetchInfo & FetchInfo::Data) == FetchInfo::Data)
    {
        return ScriptLocal::querySingle(this, {"scripts.id=?"}, {use(scriptID, "id")});
    }
    else
    {
        return std::make_unique<ScriptLocal>(scriptID, this);
    }
}

void ZapFR::Engine::SourceLocal::removeScript(uint64_t scriptID)
{
    ScriptLocal::remove(scriptID);
}

void ZapFR::Engine::SourceLocal::addScript(Script::Type type, const std::string& title, bool enabled, const std::unordered_set<Script::Event>& events,
                                           const std::optional<std::unordered_set<uint64_t>>& feedIDs, const std::string& script)
{
    ScriptLocal::create(type, title, enabled, events, feedIDs, script);
}

/* ************************** SOURCE STUFF ************************** */
void ZapFR::Engine::SourceLocal::fetchStatistics()
{
    mStatistics.clear();

    // total feed count
    {
        uint64_t totalFeedCount{0};
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << "SELECT COUNT(*) FROM feeds", into(totalFeedCount), now;
        mStatistics[Statistic::FeedCount] = std::to_string(totalFeedCount);
    }

    // total post count
    {
        uint64_t totalPostCount{0};
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << "SELECT COUNT(*) FROM posts", into(totalPostCount), now;
        mStatistics[Statistic::PostCount] = std::to_string(totalPostCount);
    }

    // total flagged post count
    {
        uint64_t totalFlaggedPostCount{0};
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << "SELECT COUNT(*) FROM flags", into(totalFlaggedPostCount), now;
        mStatistics[Statistic::FlaggedPostCount] = std::to_string(totalFlaggedPostCount);
    }

    // oldest post
    {
        Poco::Nullable<std::string> oldestPost{};
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << "SELECT MIN(datePublished) FROM posts", into(oldestPost), now;
        mStatistics[Statistic::OldestPost] = oldestPost.isNull() ? "" : oldestPost.value();
    }

    // newest post
    {
        Poco::Nullable<std::string> newestPost{};
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << "SELECT MAX(datePublished) FROM posts", into(newestPost), now;
        mStatistics[Statistic::NewestPost] = newestPost.isNull() ? "" : newestPost.value();
    }
}

std::unordered_set<uint64_t> ZapFR::Engine::SourceLocal::importOPML(const std::string& opml, uint64_t parentFolderID)
{
    std::unordered_set<uint64_t> feedIDs;

    std::vector<OPMLEntry> feedEntries;
    try
    {
        feedEntries = ZapFR::Engine::OPMLParser::parse(opml);
    }
    catch (...)
    {
        return feedIDs;
    }

    for (const auto& feedEntry : feedEntries)
    {
        auto subfolderID = createFolderHierarchy(parentFolderID, feedEntry.folderHierarchy);
        const auto& feed = addFeed(feedEntry.url, subfolderID);
        if (feed.has_value())
        {
            feedIDs.insert(feed.value()->id());
        }
    }

    return feedIDs;
}

void ZapFR::Engine::SourceLocal::clearLogs()
{
    Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
    deleteStmt << "DELETE FROM logs", now;
}
