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

#include "ZapFR/local/SourceLocal.h"
#include "ZapFR/Database.h"
#include "ZapFR/FeedFetcher.h"
#include "ZapFR/Helpers.h"
#include "ZapFR/Log.h"
#include "ZapFR/ScriptLua.h"
#include "ZapFR/local/FeedLocal.h"
#include "ZapFR/local/FolderLocal.h"
#include "ZapFR/local/PostLocal.h"
#include "ZapFR/local/ScriptFolderLocal.h"
#include "ZapFR/local/ScriptLocal.h"

using namespace Poco::Data::Keywords;

ZapFR::Engine::SourceLocal::SourceLocal(uint64_t id) : Source(id)
{
}

/* ************************** FEED STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceLocal::getFeeds()
{
    return FeedLocal::queryMultiple(this, {}, "ORDER BY sortOrder ASC", "", {});
}

std::optional<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceLocal::getFeed(uint64_t feedID, uint32_t fetchInfo)
{
    if ((fetchInfo & FetchInfo::Data) == FetchInfo::Data)
    {
        auto feed = FeedLocal::querySingle(this, {"feeds.id=?"}, {use(feedID, "id")});
        if (feed.has_value() && (fetchInfo & FetchInfo::Statistics) == FetchInfo::Statistics)
        {
            auto localFeed = dynamic_cast<FeedLocal*>(feed.value().get());
            localFeed->fetchStatistics();
        }
        return feed;
    }
    else
    {
        return std::make_unique<FeedLocal>(feedID, this);
    }
}

uint64_t ZapFR::Engine::SourceLocal::addFeed(const std::string& url, uint64_t folder)
{
    Log::log(LogLevel::Info, fmt::format("Adding feed at {}", url));

    uint64_t feedID{0};
    try
    {
        auto feed = FeedLocal::create(this, url, url, folder);
        FeedFetcher ff;
        auto parsedFeed = ff.parseURL(url, feedID);
        auto guid = parsedFeed->guid();
        auto title = parsedFeed->title();
        auto subtitle = parsedFeed->subtitle();
        auto link = parsedFeed->link();
        auto description = parsedFeed->description();
        auto language = parsedFeed->language();
        auto copyright = parsedFeed->copyright();
        auto iconURL = parsedFeed->iconURL();

        feed->update(iconURL, guid, title, subtitle, link, description, language, copyright);
        feed->refresh(ff.xml());
    }
    catch (const Poco::Exception& e)
    {
        Log::log(LogLevel::Error, e.displayText(), feedID);
    }
    catch (const std::runtime_error& e)
    {
        Log::log(LogLevel::Error, e.what(), feedID);
    }
    catch (...)
    {
        Log::log(LogLevel::Error, "Unknown exception while adding feed", feedID);
    }

    return feedID;
}

void ZapFR::Engine::SourceLocal::moveFeed(uint64_t feedID, uint64_t newFolder, uint64_t newSortOrder)
{
    FeedLocal::move(feedID, newFolder, newSortOrder);
}

void ZapFR::Engine::SourceLocal::removeFeed(uint64_t feedID)
{
    FeedLocal::remove(this, feedID);
}

/* ************************** FOLDER STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Folder>> ZapFR::Engine::SourceLocal::getFolders(uint64_t parent)
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;

    whereClause.emplace_back("parent=?");
    bindings.emplace_back(useRef(parent, "parent"));

    return FolderLocal::queryMultiple(this, whereClause, "ORDER BY folders.sortOrder ASC", "", bindings);
}

std::optional<std::unique_ptr<ZapFR::Engine::Folder>> ZapFR::Engine::SourceLocal::getFolder(uint64_t folderID, uint32_t folderFetchInfo)
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;

    whereClause.emplace_back("id=?");
    bindings.emplace_back(useRef(folderID, "folderID"));

    auto folder = FolderLocal::querySingle(this, whereClause, bindings);
    if ((folderFetchInfo & FetchInfo::Statistics) == FetchInfo::Statistics)
    {
        auto localFolder = dynamic_cast<FolderLocal*>(folder.value().get());
        localFolder->fetchStatistics();
    }
    return folder;
}

uint64_t ZapFR::Engine::SourceLocal::addFolder(const std::string& title, uint64_t parentID)
{
    return FolderLocal::create(parentID, title);
}

void ZapFR::Engine::SourceLocal::moveFolder(uint64_t folderID, uint64_t newParent, uint64_t newSortOrder)
{
    FolderLocal::move(this, folderID, newParent, newSortOrder);
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
std::vector<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::SourceLocal::getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter,
                                                                                       FlagColor flagColor)
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;
    std::string wildcardSearchFilter = "%" + searchFilter + "%";
    auto fc = Flag::idForFlagColor(flagColor);

    if (showOnlyUnread)
    {
        whereClause.emplace_back("posts.isRead=FALSE");
    }
    if (!searchFilter.empty())
    {
        whereClause.emplace_back("(posts.title LIKE ? OR posts.description LIKE ?)");
        bindings.emplace_back(useRef(wildcardSearchFilter, "searchFilter"));
        bindings.emplace_back(useRef(wildcardSearchFilter, "searchFilter"));
    }
    if (flagColor != FlagColor::Gray)
    {
        whereClause.emplace_back("posts.id IN (SELECT DISTINCT(postID) FROM flags WHERE flagID=?)");
        bindings.emplace_back(use(fc, "flagColor"));
    }

    auto offset = perPage * (page - 1);
    bindings.emplace_back(use(perPage, "perPage"));
    bindings.emplace_back(use(offset, "offset"));

    return PostLocal::queryMultiple(whereClause, "ORDER BY posts.datePublished DESC", "LIMIT ? OFFSET ?", bindings);
}

uint64_t ZapFR::Engine::SourceLocal::getTotalPostCount(bool showOnlyUnread, const std::string& searchFilter, FlagColor flagColor)
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;
    std::string wildcardSearchFilter = "%" + searchFilter + "%";
    auto fc = Flag::idForFlagColor(flagColor);

    if (showOnlyUnread)
    {
        whereClause.emplace_back("posts.isRead=FALSE");
    }
    if (!searchFilter.empty())
    {
        whereClause.emplace_back("(posts.title LIKE ? OR posts.description LIKE ?)");
        bindings.emplace_back(useRef(wildcardSearchFilter, "searchFilter"));
        bindings.emplace_back(useRef(wildcardSearchFilter, "searchFilter"));
    }
    if (flagColor != FlagColor::Gray)
    {
        whereClause.emplace_back("posts.id IN (SELECT DISTINCT(postID) FROM flags WHERE flagID=?)");
        bindings.emplace_back(use(fc, "flagColor"));
    }

    return PostLocal::queryCount(whereClause, bindings);
}

void ZapFR::Engine::SourceLocal::markAllAsRead()
{
    PostLocal::updateIsRead(true, {}, {});
}

/* ************************** LOGS STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Log>> ZapFR::Engine::SourceLocal::getLogs(uint64_t perPage, uint64_t page)
{
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;

    auto offset = perPage * (page - 1);
    bindings.emplace_back(use(perPage, "perPage"));
    bindings.emplace_back(use(offset, "offset"));

    return Log::queryMultiple({}, "ORDER BY logs.id DESC", "LIMIT ? OFFSET ?", bindings);
}

uint64_t ZapFR::Engine::SourceLocal::getTotalLogCount()
{
    return Log::queryCount({}, {});
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

/* ************************** SCRIPT FOLDER STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::ScriptFolder>> ZapFR::Engine::SourceLocal::getScriptFolders()
{
    return ScriptFolderLocal::queryMultiple({}, "ORDER BY scriptfolders.id DESC", "", {});
}

std::optional<std::unique_ptr<ZapFR::Engine::ScriptFolder>> ZapFR::Engine::SourceLocal::getScriptFolder(uint64_t id)
{
    return ScriptFolderLocal::querySingle({"scriptfolders.id=?"}, {use(id, "id")});
}

void ZapFR::Engine::SourceLocal::addScriptFolder(const std::string& title)
{
    ScriptFolderLocal::create(title);
}

void ZapFR::Engine::SourceLocal::removeScriptFolder(uint64_t scriptFolderID)
{
    ScriptFolderLocal::remove(scriptFolderID);
}

/* ************************** SCRIPT STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Script>> ZapFR::Engine::SourceLocal::getScripts()
{
    return ScriptLocal::queryMultiple({}, "ORDER BY scripts.id DESC", "", {});
}

std::optional<std::unique_ptr<ZapFR::Engine::Script>> ZapFR::Engine::SourceLocal::getScript(uint64_t scriptID)
{
    return ScriptLocal::querySingle({"scripts.id=?"}, {use(scriptID, "id")});
}

void ZapFR::Engine::SourceLocal::removeScript(uint64_t scriptID)
{
    ScriptLocal::remove(scriptID);
}

void ZapFR::Engine::SourceLocal::addScript(Script::Type type, const std::string& filename, bool enabled, const std::unordered_set<Script::Event>& events,
                                           const std::optional<std::unordered_set<uint64_t>>& feedIDs)
{
    ScriptLocal::create(type, filename, enabled, events, feedIDs);
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
