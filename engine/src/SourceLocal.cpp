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

#include "SourceLocal.h"
#include "Database.h"
#include "FeedFetcher.h"
#include "FeedLocal.h"
#include "FolderLocal.h"
#include "Helpers.h"
#include "Log.h"
#include "Post.h"

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
                  ",sortOrder"
                  " FROM feeds"
                  " ORDER BY sortOrder ASC",
        into(id), into(url), into(iconURL), into(iconHash), into(iconLastFetched), into(folder), into(guid), into(title), into(subtitle), into(link), into(description),
        into(language), into(copyright), into(lastChecked), into(sortOrder), range(0, 1);

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
                      ",sortOrder"
                      " FROM feeds"
                      " WHERE id=?",
            use(feedID), into(id), into(url), into(iconURL), into(iconHash), into(iconLastFetched), into(folder), into(guid), into(title), into(subtitle), into(link),
            into(description), into(language), into(copyright), into(lastChecked), into(sortOrder), now;

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
            f->setSortOrder(sortOrder);
            f->setDataFetched(true);
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

    FeedFetcher ff;
    auto parsedFeed = ff.parseURL(url);

    auto guid = parsedFeed->guid();
    auto title = parsedFeed->title();
    auto subtitle = parsedFeed->subtitle();
    auto link = parsedFeed->link();
    auto description = parsedFeed->description();
    auto language = parsedFeed->language();
    auto copyright = parsedFeed->copyright();
    auto iconURL = parsedFeed->iconURL();
    uint64_t feedID{0};
    auto sortOrder = getNextFeedSortOrder(folder);

    // scope for insert feed mutex lock
    {
        Poco::Data::Statement insertStmt(*(Database::getInstance()->session()));
        insertStmt << "INSERT INTO feeds ("
                      " url"
                      ",iconURL"
                      ",folder"
                      ",guid"
                      ",title"
                      ",subtitle"
                      ",link"
                      ",description"
                      ",language"
                      ",copyright"
                      ",sortOrder"
                      ",lastChecked"
                      ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP)",
            useRef(url), useRef(iconURL), use(folder), useRef(guid), useRef(title), useRef(subtitle), useRef(link), useRef(description), useRef(language), useRef(copyright),
            use(sortOrder);
        const std::lock_guard<std::mutex> lock(msAddFeedMutex);
        insertStmt.execute();
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << "SELECT last_insert_rowid()", into(feedID), range(0, 1);
        selectStmt.execute();
    }

    auto feed = getFeed(feedID);
    if (feed.has_value())
    {
        feed.value()->refresh(ff.xml());
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
        uint64_t parentID{0};

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
            parentID = parent->id();
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
            existingSubfolderID = addFolder(folderTitle, parentID);
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

    std::string whereClause = showOnlyUnread ? "WHERE isRead=FALSE" : "";

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << Poco::format("SELECT id"
                               ",feedID"
                               ",isRead"
                               ",title"
                               ",link"
                               ",description"
                               ",author"
                               ",commentsURL"
                               ",enclosureURL"
                               ",enclosureLength"
                               ",enclosureMimeType"
                               ",guid"
                               ",guidIsPermalink"
                               ",datePublished"
                               ",sourceURL"
                               ",sourceTitle"
                               " FROM posts"
                               " %s"
                               " ORDER BY datePublished DESC"
                               " LIMIT ? OFFSET ?",
                               whereClause),
        use(perPage), use(offset), into(id), into(postFeedID), into(isRead), into(title), into(link), into(description), into(author), into(commentsURL), into(enclosureURL),
        into(enclosureLength), into(enclosureMimeType), into(guid), into(guidIsPermalink), into(datePublished), into(sourceURL), into(sourceTitle), range(0, 1);

    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            auto p = std::make_unique<Post>(id);
            p->setIsRead(isRead);
            p->setFeedID(postFeedID);
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
