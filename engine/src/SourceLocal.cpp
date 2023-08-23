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

#include "ZapFR/SourceLocal.h"
#include "ZapFR/Database.h"
#include "ZapFR/FeedFetcher.h"
#include "ZapFR/FeedLocal.h"
#include "ZapFR/FolderLocal.h"
#include "ZapFR/Helpers.h"
#include "ZapFR/Log.h"
#include "ZapFR/PostLocal.h"
#include "ZapFR/ScriptFolderLocal.h"
#include "ZapFR/ScriptLocal.h"
#include "ZapFR/ScriptLua.h"

using namespace Poco::Data::Keywords;

std::mutex ZapFR::Engine::SourceLocal::msCreateFolderHierarchyMutex{};
std::mutex ZapFR::Engine::SourceLocal::msAddFeedMutex{};
std::mutex ZapFR::Engine::SourceLocal::msAddFolderMutex{};

ZapFR::Engine::SourceLocal::SourceLocal(uint64_t id) : Source(id)
{
}

std::vector<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceLocal::getFeeds()
{
    std::vector<std::unique_ptr<Feed>> feeds;

    uint64_t id;
    std::string url;
    std::string iconURL;
    std::string iconHash;
    std::string iconLastFetched;
    uint64_t folder;
    std::string guid;
    std::string title;
    std::string subtitle;
    std::string link;
    std::string description;
    std::string language;
    std::string copyright;
    std::string lastChecked;
    Poco::Nullable<std::string> lastRefreshError;
    uint64_t sortOrder;

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT id"
                  ",url"
                  ",iconURL"
                  ",iconHash"
                  ",iconLastFetched"
                  ",folder"
                  ",guid"
                  ",title"
                  ",subtitle"
                  ",link"
                  ",description"
                  ",language"
                  ",copyright"
                  ",lastChecked"
                  ",lastRefreshError"
                  ",sortOrder"
                  " FROM feeds"
                  " ORDER BY sortOrder ASC",
        into(id), into(url), into(iconURL), into(iconHash), into(iconLastFetched), into(folder), into(guid), into(title), into(subtitle), into(link), into(description),
        into(language), into(copyright), into(lastChecked), into(lastRefreshError), into(sortOrder), range(0, 1);

    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            auto f = std::make_unique<FeedLocal>(id);
            f->setURL(url);
            f->setIconURL(iconURL);
            f->setIconHash(iconHash);
            f->setIconLastFetched(iconLastFetched);
            f->setFolder(folder);
            f->setGuid(guid);
            f->setTitle(title);
            f->setSubtitle(subtitle);
            f->setLink(link);
            f->setDescription(description);
            f->setLanguage(language);
            f->setCopyright(copyright);
            f->setLastChecked(lastChecked);
            if (!lastRefreshError.isNull())
            {
                f->setLastRefreshError(lastRefreshError);
            }
            f->setSortOrder(sortOrder);
            f->setDataFetched(true);

            // fetch the unread count
            uint64_t unreadCount{0};
            Poco::Data::Statement selectUnreadStmt(*(Database::getInstance()->session()));
            selectUnreadStmt << "SELECT COUNT(*) FROM posts WHERE feedID=? AND isRead=FALSE", use(id), into(unreadCount), now;
            f->setUnreadCount(unreadCount);

            feeds.emplace_back(std::move(f));
        }
    }
    return feeds;
}

std::optional<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceLocal::getFeed(uint64_t feedID, bool fetchData)
{
    if (fetchData)
    {
        uint64_t id{0};
        std::string url;
        std::string iconURL;
        std::string iconHash;
        std::string iconLastFetched;
        uint64_t folder;
        std::string guid;
        std::string title;
        std::string subtitle;
        std::string link;
        std::string description;
        std::string language;
        std::string copyright;
        std::string lastChecked;
        Poco::Nullable<std::string> lastRefreshError;
        uint64_t sortOrder;

        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << "SELECT id"
                      ",url"
                      ",iconURL"
                      ",iconHash"
                      ",iconLastFetched"
                      ",folder"
                      ",guid"
                      ",title"
                      ",subtitle"
                      ",link"
                      ",description"
                      ",language"
                      ",copyright"
                      ",lastChecked"
                      ",lastRefreshError"
                      ",sortOrder"
                      " FROM feeds"
                      " WHERE id=?",
            use(feedID), into(id), into(url), into(iconURL), into(iconHash), into(iconLastFetched), into(folder), into(guid), into(title), into(subtitle), into(link),
            into(description), into(language), into(copyright), into(lastChecked), into(lastRefreshError), into(sortOrder), now;

        auto rs = Poco::Data::RecordSet(selectStmt);
        if (rs.rowCount() == 1)
        {
            auto f = std::make_unique<FeedLocal>(id);
            f->setURL(url);
            f->setIconURL(iconURL);
            f->setIconHash(iconHash);
            f->setIconLastFetched(iconLastFetched);
            f->setFolder(folder);
            f->setGuid(guid);
            f->setTitle(title);
            f->setSubtitle(subtitle);
            f->setLink(link);
            f->setDescription(description);
            f->setLanguage(language);
            f->setCopyright(copyright);
            f->setLastChecked(lastChecked);
            if (!lastRefreshError.isNull())
            {
                f->setLastRefreshError(lastRefreshError);
            }
            f->setSortOrder(sortOrder);
            f->setDataFetched(true);

            // fetch the unread count
            uint64_t unreadCount{0};
            Poco::Data::Statement selectUnreadStmt(*(Database::getInstance()->session()));
            selectUnreadStmt << "SELECT COUNT(*) FROM posts WHERE feedID=? AND isRead=FALSE", use(id), into(unreadCount), now;
            f->setUnreadCount(unreadCount);

            return f;
        }

        return {};
    }
    else
    {
        return std::make_unique<FeedLocal>(feedID);
    }
}

uint64_t ZapFR::Engine::SourceLocal::addFeed(const std::string& url, uint64_t folder)
{
    Log::log(LogLevel::Info, fmt::format("Adding feed at {}", url));

    uint64_t feedID{0};
    try
    {
        // create the record in the database first, so we have a feed ID, before actually parsing the feed XML
        {
            // scope for insert feed mutex lock
            auto sortOrder = getNextFeedSortOrder(folder);
            auto nowDate = Poco::DateTimeFormatter::format(Poco::DateTime(), Poco::DateTimeFormat::ISO8601_FORMAT);

            Poco::Data::Statement insertStmt(*(Database::getInstance()->session()));
            insertStmt << "INSERT INTO feeds ("
                          " url"
                          ",title"
                          ",folder"
                          ",sortOrder"
                          ",lastChecked"
                          ",title"
                          ") VALUES (?, ?, ?, ?, ?, '')",
                useRef(url), useRef(url), use(folder), use(sortOrder), useRef(nowDate);
            const std::lock_guard<std::mutex> lock(msAddFeedMutex);
            insertStmt.execute();
            Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
            selectStmt << "SELECT last_insert_rowid()", into(feedID), now;
        }

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

        Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
        updateStmt << "UPDATE feeds SET "
                      " iconURL=?"
                      ",guid=?"
                      ",title=?"
                      ",subtitle=?"
                      ",link=?"
                      ",description=?"
                      ",language=?"
                      ",copyright=?"
                      " WHERE id=?",
            useRef(iconURL), useRef(guid), useRef(title), useRef(subtitle), useRef(link), useRef(description), useRef(language), useRef(copyright), use(feedID), now;

        auto feed = getFeed(feedID);
        if (feed.has_value())
        {
            feed.value()->refresh(ff.xml());
        }
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

uint64_t ZapFR::Engine::SourceLocal::getNextFeedSortOrder(uint64_t folder) const
{
    uint64_t sortOrder{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT MAX(sortOrder) FROM feeds WHERE folder=?", into(sortOrder), use(folder), now;
    return sortOrder + 10;
}

uint64_t ZapFR::Engine::SourceLocal::getNextFolderSortOrder(uint64_t folder) const
{
    uint64_t sortOrder{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT MAX(sortOrder) FROM folders WHERE parent=?", into(sortOrder), use(folder), now;
    return sortOrder + 10;
}

void ZapFR::Engine::SourceLocal::moveFeed(uint64_t feedID, uint64_t newFolder, uint64_t newSortOrder)
{
    uint64_t oldFolder{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT folder FROM feeds WHERE id=?", use(feedID), into(oldFolder), now;

    Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
    updateStmt << "UPDATE feeds SET folder=?, sortOrder=? WHERE id=?", use(newFolder), use(newSortOrder), use(feedID), now;

    resortFeeds(newFolder);
    if (newFolder != oldFolder) // check in case we are moving within the same folder
    {
        resortFeeds(oldFolder);
    }
}

void ZapFR::Engine::SourceLocal::moveFolder(uint64_t folderID, uint64_t newParent, uint64_t newSortOrder)
{
    auto f = getFolder(folderID);
    if (f.has_value())
    {
        Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
        updateStmt << "UPDATE folders SET parent=?, sortOrder=? WHERE id=?", use(newParent), use(newSortOrder), use(folderID), now;

        auto oldParent = f.value()->parentID();
        resortFolders(oldParent);
        if (newParent != oldParent) // check in case we are moving within the same folder
        {
            resortFolders(newParent);
        }
    }
}

void ZapFR::Engine::SourceLocal::resortFeeds(uint64_t folder) const
{
    std::vector<uint64_t> feedIDs;

    uint64_t feedID{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT id FROM feeds WHERE folder=? ORDER BY sortOrder ASC", use(folder), into(feedID), range(0, 1);
    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            feedIDs.emplace_back(feedID);
        }
    }

    uint64_t sortOrder = 10;
    for (auto f : feedIDs)
    {
        Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
        updateStmt << "UPDATE feeds SET sortOrder=? WHERE id=?", use(sortOrder), use(f), now;
        sortOrder += 10;
    }
}

void ZapFR::Engine::SourceLocal::resortFolders(uint64_t folder) const
{
    std::vector<uint64_t> folderIDs;

    uint64_t folderID{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT id FROM folders WHERE parent=? ORDER BY sortOrder ASC", use(folder), into(folderID), range(0, 1);
    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            folderIDs.emplace_back(folderID);
        }
    }
    uint64_t sortOrder = 10;
    for (auto f : folderIDs)
    {
        Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
        updateStmt << "UPDATE folders SET sortOrder=? WHERE id=?", use(sortOrder), use(f), now;
        sortOrder += 10;
    }
}

void ZapFR::Engine::SourceLocal::removeFeed(uint64_t feedID)
{
    auto feed = getFeed(feedID);
    if (feed.has_value())
    {
        auto folder = feed.value()->folder();
        feed.value()->removeIcon();

        {
            Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
            deleteStmt << "DELETE FROM feeds WHERE id=?", use(feedID), now;
        }

        {
            Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
            deleteStmt << "DELETE FROM posts WHERE feedID=?", use(feedID), now;
        }

        resortFeeds(folder);
    }
}

void ZapFR::Engine::SourceLocal::removeFolder(uint64_t folder)
{
    // get the parent id for this folder
    auto f = getFolder(folder);
    if (f.has_value())
    {
        auto folderParent = f.value()->parentID();

        auto feedIDs = f.value()->feedIDsInFoldersAndSubfolders();
        auto folderIDs = f.value()->folderAndSubfolderIDs();

        // remove the icons from the cache
        for (const auto& feedID : feedIDs)
        {
            auto feed = FeedLocal(feedID);
            feed.removeIcon();
        }

        // remove feeds and their posts
        if (feedIDs.size() > 0)
        {
            auto joinedFeedIDs = Helpers::joinIDNumbers(feedIDs, ",");

            // remove all posts from the affected feeds
            auto deletePostsSQL = Poco::format("DELETE FROM posts WHERE feedID IN (%s)", joinedFeedIDs);
            Poco::Data::Statement deletePostsStmt(*(Database::getInstance()->session()));
            deletePostsStmt << deletePostsSQL, now;

            // remove all affected feeds
            auto deleteFeedsSQL = Poco::format("DELETE FROM feeds WHERE id IN (%s)", joinedFeedIDs);
            Poco::Data::Statement deleteFeedsStmt(*(Database::getInstance()->session()));
            deleteFeedsStmt << deleteFeedsSQL, now;
        }

        // remove folders
        auto joinedFolderIDs = Helpers::joinIDNumbers(folderIDs, ",");
        auto deleteFoldersSQL = Poco::format("DELETE FROM folders WHERE id IN (%s)", joinedFolderIDs);
        Poco::Data::Statement deleteFoldersStmt(*(Database::getInstance()->session()));
        deleteFoldersStmt << deleteFoldersSQL, now;
        resortFolders(folderParent);
    }
}

std::vector<std::unique_ptr<ZapFR::Engine::Folder>> ZapFR::Engine::SourceLocal::getFolders(uint64_t parent)
{
    std::vector<std::unique_ptr<Folder>> folders;

    uint64_t id{0};
    uint64_t sortOrder{0};
    std::string title{""};

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT id"
                  ",title"
                  ",sortOrder"
                  " FROM folders"
                  " WHERE parent=?"
                  " ORDER BY sortOrder ASC",
        use(parent), into(id), into(title), into(sortOrder), range(0, 1);

    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            auto f = std::make_unique<FolderLocal>(id, parent);
            f->setTitle(title);
            f->setSortOrder(sortOrder);
            f->setDataFetched(true);
            folders.emplace_back(std::move(f));
        }
    }
    return folders;
}

std::optional<std::unique_ptr<ZapFR::Engine::Folder>> ZapFR::Engine::SourceLocal::getFolder(uint64_t folderID)
{
    uint64_t parent{0};
    std::string title{""};
    uint64_t sortOrder{0};

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT"
                  " parent"
                  ",title"
                  ",sortOrder"
                  " FROM folders"
                  " WHERE id=?",
        use(folderID), into(parent), into(title), into(sortOrder), now;

    auto rs = Poco::Data::RecordSet(selectStmt);
    if (rs.rowCount() == 1)
    {
        auto f = std::make_unique<FolderLocal>(folderID, parent);
        f->setTitle(title);
        f->setSortOrder(sortOrder);
        f->setDataFetched(true);
        return f;
    }

    return {};
}

void ZapFR::Engine::SourceLocal::getSubfolderIDs(uint64_t parent, std::vector<uint64_t>& ids, bool includeParent)
{
    if (includeParent)
    {
        ids.emplace_back(parent);
    }
    uint64_t id{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT id"
                  " FROM folders"
                  " WHERE parent=?",
        use(parent), into(id), range(0, 1);
    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            getSubfolderIDs(id, ids, true);
        }
    }
}

uint64_t ZapFR::Engine::SourceLocal::addFolder(const std::string& title, uint64_t parentID)
{
    auto sortOrder = getNextFolderSortOrder(parentID);
    Poco::Data::Statement insertStmt(*(Database::getInstance()->session()));
    insertStmt << "INSERT INTO folders (parent,title,sortOrder) VALUES (?, ?, ?)", use(parentID), useRef(title), use(sortOrder);
    const std::lock_guard<std::mutex> lock(msAddFolderMutex);
    insertStmt.execute();
    uint64_t newFolderID{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT last_insert_rowid()", into(newFolderID), range(0, 1);
    selectStmt.execute();
    return newFolderID;
}

uint64_t ZapFR::Engine::SourceLocal::createFolderHierarchy(uint64_t parentID, const std::vector<std::string> folderHierarchy)
{

    std::function<uint64_t(ZapFR::Engine::Folder*, const std::string&)> getSubfolderWithTitle;
    getSubfolderWithTitle = [&](ZapFR::Engine::Folder* parent, const std::string& folderTitle) -> uint64_t
    {
        auto existingSubFolderFound{false};
        uint64_t existingSubfolderID{0};
        uint64_t pID{0};

        if (parent == nullptr)
        {
            for (const auto& subfolder : getFolders(0))
            {
                if (subfolder->title() == folderTitle)
                {
                    existingSubfolderID = subfolder->id();
                    existingSubFolderFound = true;
                    break;
                }
            }
        }
        else
        {
            pID = parent->id();
            for (const auto& subfolder : parent->subfolders())
            {
                if (subfolder->title() == folderTitle)
                {
                    existingSubfolderID = subfolder->id();
                    existingSubFolderFound = true;
                    break;
                }
            }
        }

        if (!existingSubFolderFound)
        {
            existingSubfolderID = addFolder(folderTitle, pID);
        }
        return existingSubfolderID;
    };

    const std::lock_guard<std::mutex> lock(msCreateFolderHierarchyMutex);
    if (parentID == 0)
    {
        std::unique_ptr<Folder> currentParent{};
        for (const auto& subfolder : folderHierarchy)
        {
            auto subfolderID = getSubfolderWithTitle(currentParent.get(), subfolder);
            auto subfolderInstance = getFolder(subfolderID);
            if (subfolderInstance.has_value())
            {
                currentParent = std::move(subfolderInstance.value());
            }
        }
        return currentParent->id();
    }
    else
    {
        auto rootFolder = getFolder(parentID);
        if (rootFolder.has_value())
        {
            auto currentParent = std::move(rootFolder.value());
            for (const auto& subfolder : folderHierarchy)
            {
                auto subfolderID = getSubfolderWithTitle(currentParent.get(), subfolder);
                auto subfolderInstance = getFolder(subfolderID);
                if (subfolderInstance.has_value())
                {
                    currentParent = std::move(subfolderInstance.value());
                }
            }
            return currentParent->id();
        }
    }
    return 0;
}

std::vector<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::SourceLocal::getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread)
{
    std::vector<std::unique_ptr<Post>> posts;

    auto offset = perPage * (page - 1);

    uint64_t id{0};
    uint64_t postFeedID{0};
    bool isRead{false};
    std::string title{""};
    std::string link{""};
    std::string description{""};
    std::string author{""};
    std::string commentsURL{""};
    std::string enclosureURL{""};
    std::string enclosureLength{""};
    std::string enclosureMimeType{""};
    std::string guid{""};
    bool guidIsPermalink{false};
    std::string datePublished{""};
    std::string sourceURL{""};
    std::string sourceTitle{""};
    std::string feedTitle{""};

    std::string whereClause = showOnlyUnread ? "WHERE posts.isRead=FALSE" : "";

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << Poco::format("SELECT posts.id"
                               ",posts.feedID"
                               ",posts.isRead"
                               ",posts.title"
                               ",posts.link"
                               ",posts.description"
                               ",posts.author"
                               ",posts.commentsURL"
                               ",posts.enclosureURL"
                               ",posts.enclosureLength"
                               ",posts.enclosureMimeType"
                               ",posts.guid"
                               ",posts.guidIsPermalink"
                               ",posts.datePublished"
                               ",posts.sourceURL"
                               ",posts.sourceTitle"
                               ",feeds.title"
                               " FROM posts"
                               " LEFT JOIN feeds ON feeds.id = posts.feedID"
                               " %s"
                               " ORDER BY posts.datePublished DESC"
                               " LIMIT ? OFFSET ?",
                               whereClause),
        use(perPage), use(offset), into(id), into(postFeedID), into(isRead), into(title), into(link), into(description), into(author), into(commentsURL), into(enclosureURL),
        into(enclosureLength), into(enclosureMimeType), into(guid), into(guidIsPermalink), into(datePublished), into(sourceURL), into(sourceTitle), into(feedTitle),
        range(0, 1);

    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            auto p = std::make_unique<PostLocal>(id);
            p->setIsRead(isRead);
            p->setFeedID(postFeedID);
            p->setFeedTitle(feedTitle);
            p->setTitle(title);
            p->setLink(link);
            p->setDescription(description);
            p->setAuthor(author);
            p->setCommentsURL(commentsURL);
            p->setEnclosureURL(enclosureURL);
            p->setEnclosureLength(enclosureLength);
            p->setEnclosureMimeType(enclosureMimeType);
            p->setGuid(guid);
            p->setGuidIsPermalink(guidIsPermalink);
            p->setDatePublished(datePublished);
            p->setSourceURL(sourceURL);
            p->setSourceTitle(sourceTitle);

            // query flags
            std::unordered_set<FlagColor> flags;
            uint8_t flagID{0};
            Poco::Data::Statement selectFlagsStmt(*(Database::getInstance()->session()));
            selectFlagsStmt << "SELECT DISTINCT(flagID) FROM flags WHERE postID=?", use(id), into(flagID), range(0, 1);
            while (!selectFlagsStmt.done())
            {
                if (selectFlagsStmt.execute() > 0)
                {
                    flags.insert(Flag::flagColorForID(flagID));
                }
            }
            p->setFlagColors(flags);

            posts.emplace_back(std::move(p));
        }
    }
    return posts;
}

void ZapFR::Engine::SourceLocal::markAllAsRead()
{
    Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
    updateStmt << "UPDATE posts SET isRead=TRUE", now;
    updateStmt.execute();
}

uint64_t ZapFR::Engine::SourceLocal::getTotalPostCount(bool showOnlyUnread)
{
    uint64_t postCount;
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    std::string sql = "SELECT COUNT(*) FROM posts";
    if (showOnlyUnread)
    {
        sql = "SELECT COUNT(*) FROM posts WHERE isRead=FALSE";
    }
    selectStmt << sql, into(postCount), now;
    return postCount;
}

std::vector<std::unique_ptr<ZapFR::Engine::Log>> ZapFR::Engine::SourceLocal::getLogs(uint64_t perPage, uint64_t page)
{
    std::vector<std::unique_ptr<Log>> logs;

    auto offset = perPage * (page - 1);

    uint64_t id{0};
    std::string timestamp{""};
    uint64_t level;
    std::string message{""};
    Poco::Nullable<uint64_t> feedID{0};
    Poco::Nullable<std::string> feedTitle{};

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT logs.id"
                  ",logs.timestamp"
                  ",logs.level"
                  ",logs.message"
                  ",logs.feedID"
                  ",feeds.title"
                  " FROM logs"
                  " LEFT JOIN feeds ON feeds.id = logs.feedID"
                  " ORDER BY logs.id DESC"
                  " LIMIT ? OFFSET ?",
        use(perPage), use(offset), into(id), into(timestamp), into(level), into(message), into(feedID), into(feedTitle), range(0, 1);

    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            auto l = std::make_unique<Log>(id);
            l->setTimestamp(timestamp);
            l->setLevel(level);
            l->setMessage(message);
            if (!feedID.isNull())
            {
                l->setFeedID(feedID.value());
            }
            if (!feedTitle.isNull())
            {
                l->setFeedTitle(feedTitle.value());
            }
            logs.emplace_back(std::move(l));
        }
    }

    return logs;
}

uint64_t ZapFR::Engine::SourceLocal::getTotalLogCount()
{
    uint64_t logCount;
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT COUNT(*) FROM logs", into(logCount), now;
    return logCount;
}

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

std::vector<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::SourceLocal::getFlaggedPosts(FlagColor flagColor, uint64_t perPage, uint64_t page, bool showOnlyUnread)
{
    std::vector<std::unique_ptr<Post>> posts;

    auto offset = perPage * (page - 1);
    auto fc = Flag::idForFlagColor(flagColor);

    uint64_t id{0};
    uint64_t postFeedID{0};
    bool isRead{false};
    std::string title{""};
    std::string link{""};
    std::string description{""};
    std::string author{""};
    std::string commentsURL{""};
    std::string enclosureURL{""};
    std::string enclosureLength{""};
    std::string enclosureMimeType{""};
    std::string guid{""};
    bool guidIsPermalink{false};
    std::string datePublished{""};
    std::string sourceURL{""};
    std::string sourceTitle{""};
    std::string feedTitle{""};

    std::string whereClause = showOnlyUnread ? "AND posts.isRead=FALSE" : "";

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << Poco::format("SELECT posts.id"
                               ",posts.feedID"
                               ",posts.isRead"
                               ",posts.title"
                               ",posts.link"
                               ",posts.description"
                               ",posts.author"
                               ",posts.commentsURL"
                               ",posts.enclosureURL"
                               ",posts.enclosureLength"
                               ",posts.enclosureMimeType"
                               ",posts.guid"
                               ",posts.guidIsPermalink"
                               ",posts.datePublished"
                               ",posts.sourceURL"
                               ",posts.sourceTitle"
                               ",feeds.title"
                               " FROM posts"
                               " LEFT JOIN feeds ON feeds.id = posts.feedID"
                               " WHERE posts.id IN (SELECT DISTINCT(postID) FROM flags WHERE flagID=?)"
                               " %s"
                               " ORDER BY posts.datePublished DESC"
                               " LIMIT ? OFFSET ?",
                               whereClause),
        use(fc), use(perPage), use(offset), into(id), into(postFeedID), into(isRead), into(title), into(link), into(description), into(author), into(commentsURL),
        into(enclosureURL), into(enclosureLength), into(enclosureMimeType), into(guid), into(guidIsPermalink), into(datePublished), into(sourceURL), into(sourceTitle),
        into(feedTitle), range(0, 1);

    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            auto p = std::make_unique<PostLocal>(id);
            p->setIsRead(isRead);
            p->setFeedID(postFeedID);
            p->setFeedTitle(feedTitle);
            p->setTitle(title);
            p->setLink(link);
            p->setDescription(description);
            p->setAuthor(author);
            p->setCommentsURL(commentsURL);
            p->setEnclosureURL(enclosureURL);
            p->setEnclosureLength(enclosureLength);
            p->setEnclosureMimeType(enclosureMimeType);
            p->setGuid(guid);
            p->setGuidIsPermalink(guidIsPermalink);
            p->setDatePublished(datePublished);
            p->setSourceURL(sourceURL);
            p->setSourceTitle(sourceTitle);

            // query flags
            std::unordered_set<FlagColor> flags;
            uint8_t flagID{0};
            Poco::Data::Statement selectFlagsStmt(*(Database::getInstance()->session()));
            selectFlagsStmt << "SELECT DISTINCT(flagID) FROM flags WHERE postID=?", use(id), into(flagID), range(0, 1);
            while (!selectFlagsStmt.done())
            {
                if (selectFlagsStmt.execute() > 0)
                {
                    flags.insert(Flag::flagColorForID(flagID));
                }
            }
            p->setFlagColors(flags);

            posts.emplace_back(std::move(p));
        }
    }
    return posts;
}

uint64_t ZapFR::Engine::SourceLocal::getTotalFlaggedPostCount(FlagColor flagColor, bool showOnlyUnread)
{
    auto fc = Flag::idForFlagColor(flagColor);
    uint64_t postCount;
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    std::string sql = "SELECT COUNT(*) FROM flags WHERE flagID=?";
    if (showOnlyUnread)
    {
        sql = "SELECT COUNT(*) FROM flags LEFT JOIN posts ON flags.postID=posts.id WHERE flags.flagID=? AND posts.isRead=FALSE";
    }
    selectStmt << sql, use(fc), into(postCount), now;
    return postCount;
}

std::vector<std::unique_ptr<ZapFR::Engine::ScriptFolder>> ZapFR::Engine::SourceLocal::getScriptFolders()
{
    std::vector<std::unique_ptr<ScriptFolder>> scriptFolders;

    uint64_t id{0};
    std::string title{""};

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT id, title FROM scriptfolders ORDER BY id DESC", into(id), into(title), range(0, 1);
    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            auto sfl = std::make_unique<ScriptFolderLocal>(id);
            sfl->setTitle(title);
            scriptFolders.emplace_back(std::move(sfl));
        }
    }
    return scriptFolders;
}

std::optional<std::unique_ptr<ZapFR::Engine::ScriptFolder>> ZapFR::Engine::SourceLocal::getScriptFolder(uint64_t id)
{
    std::string title{""};

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT id, title FROM scriptfolders WHERE id=?", use(id), into(title), now;
    auto rs = Poco::Data::RecordSet(selectStmt);
    if (rs.rowCount() == 1)
    {
        auto sf = std::make_unique<ScriptFolderLocal>(id);
        sf->setTitle(title);
        return sf;
    }
    return {};
}

void ZapFR::Engine::SourceLocal::addScriptFolder(const std::string& title)
{
    Poco::Data::Statement insertStmt(*(Database::getInstance()->session()));
    insertStmt << "INSERT INTO scriptfolders (title) VALUES (?)", useRef(title), now;
}

void ZapFR::Engine::SourceLocal::removeScriptFolder(uint64_t scriptFolderID)
{
    Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
    deleteStmt << "DELETE FROM scriptfolders WHERE id=?", use(scriptFolderID), now;

    Poco::Data::Statement deleteStmt2(*(Database::getInstance()->session()));
    deleteStmt2 << "DELETE FROM scriptfolder_posts WHERE scriptFolderID=?", use(scriptFolderID), now;
}

std::vector<std::unique_ptr<ZapFR::Engine::Script>> ZapFR::Engine::SourceLocal::getScripts()
{
    std::vector<std::unique_ptr<Script>> scripts;

    uint64_t id{0};
    std::string type{""};
    std::string filename{""};
    bool isEnabled{false};
    std::string runOnEvents{""};
    Poco::Nullable<std::string> runOnFeedIDs{};

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT id"
                  ",type"
                  ",filename"
                  ",isEnabled"
                  ",runOnEvents"
                  ",runOnFeedIDs"
                  " FROM scripts"
                  " ORDER BY id DESC",
        into(id), into(type), into(filename), into(isEnabled), into(runOnEvents), into(runOnFeedIDs), range(0, 1);
    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            if (type == Script::msTypeLuaIdentifier) // force lua for now
            {
                auto s = std::make_unique<ScriptLocal>(id);
                s->setType(Script::Type::Lua);
                s->setFilename(filename);
                s->setIsEnabled(isEnabled);
                s->parseRunOnEvents(runOnEvents);
                if (!runOnFeedIDs.isNull())
                {
                    s->parseRunOnFeedIDs(runOnFeedIDs.value());
                }
                scripts.emplace_back(std::move(s));
            }
        }
    }
    return scripts;
}

std::optional<std::unique_ptr<ZapFR::Engine::Script>> ZapFR::Engine::SourceLocal::getScript(uint64_t scriptID)
{
    uint64_t id{0};
    std::string type{""};
    std::string filename{""};
    bool isEnabled{false};
    std::string runOnEvents{""};
    Poco::Nullable<std::string> runOnFeedIDs{};

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT id"
                  ",type"
                  ",filename"
                  ",isEnabled"
                  ",runOnEvents"
                  ",runOnFeedIDs"
                  " FROM scripts"
                  " WHERE id=?",
        use(scriptID), into(id), into(type), into(filename), into(isEnabled), into(runOnEvents), into(runOnFeedIDs), now;

    auto rs = Poco::Data::RecordSet(selectStmt);
    if (rs.rowCount() == 1)
    {
        if (type == Script::msTypeLuaIdentifier) // force lua for now
        {
            auto s = std::make_unique<ScriptLocal>(id);
            s->setType(Script::Type::Lua);
            s->setFilename(filename);
            s->setIsEnabled(isEnabled);
            s->parseRunOnEvents(runOnEvents);
            if (!runOnFeedIDs.isNull())
            {
                s->parseRunOnFeedIDs(runOnFeedIDs.value());
            }
            return s;
        }
    }

    return {};
}

void ZapFR::Engine::SourceLocal::removeScript(uint64_t scriptID)
{
    Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
    deleteStmt << "DELETE FROM scripts WHERE id=?", use(scriptID), now;
}

void ZapFR::Engine::SourceLocal::addScript(Script::Type type, const std::string& filename, bool enabled, const std::unordered_set<Script::Event>& events,
                                           const std::optional<std::unordered_set<uint64_t>>& feedIDs)
{
    std::string typeStr;
    switch (type)
    {
        case Script::Type::Lua:
        {
            typeStr = Script::msTypeLuaIdentifier;
            break;
        }
    }

    // join all the events into a comma separated identifier string
    std::vector<std::string> eventStrings;
    if (events.contains(Script::Event::NewPost))
    {
        eventStrings.emplace_back(Script::msEventNewPostIdentifier);
    }
    if (events.contains(Script::Event::UpdatePost))
    {
        eventStrings.emplace_back(Script::msEventUpdatePostIdentifier);
    }
    auto joinedEvents = Helpers::joinString(eventStrings, ",");

    // join all the selected feedIDs into a comma separated string
    Poco::Nullable<std::string> joinedFeedIDs;
    if (feedIDs.has_value())
    {
        std::vector<uint64_t> f;
        for (const auto& feedID : feedIDs.value())
        {
            f.emplace_back(feedID);
        }
        joinedFeedIDs = Helpers::joinIDNumbers(f, ",");
    }

    Poco::Data::Statement insertStmt(*(Database::getInstance()->session()));
    insertStmt << "INSERT INTO scripts"
                  " (type,filename,isEnabled,runOnEvents,runOnFeedIDs)"
                  " VALUES"
                  " (?,?,?,?,?)",
        useRef(typeStr), useRef(filename), use(enabled), useRef(joinedEvents), use(joinedFeedIDs), now;
}
