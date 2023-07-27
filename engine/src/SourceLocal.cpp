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

    Poco::Data::Statement selectStmt(*(msDatabase->session()));
    selectStmt << "SELECT id"
                  ",url"
                  ",iconURL"
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
        into(id), into(url), into(iconURL), into(iconLastFetched), into(folder), into(guid), into(title), into(subtitle), into(link), into(description), into(language),
        into(copyright), into(lastChecked), into(sortOrder), range(0, 1);

    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            auto f = std::make_unique<FeedLocal>(id);
            f->setURL(url);
            f->setIconURL(iconURL);
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
            Poco::Data::Statement selectUnreadStmt(*(msDatabase->session()));
            selectUnreadStmt << "SELECT COUNT(*) FROM posts WHERE feedID=? AND isRead=FALSE", use(id), into(unreadCount), now;
            f->setUnreadCount(unreadCount);

            feeds.emplace_back(std::move(f));
        }
    }
    return feeds;
}

std::optional<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceLocal::getFeed(uint64_t feedID)
{
    uint64_t id{0};
    std::string url;
    std::string iconURL;
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

    Poco::Data::Statement selectStmt(*(msDatabase->session()));
    selectStmt << "SELECT id"
                  ",url"
                  ",iconURL"
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
        use(feedID), into(id), into(url), into(iconURL), into(iconLastFetched), into(folder), into(guid), into(title), into(subtitle), into(link), into(description),
        into(language), into(copyright), into(lastChecked), into(sortOrder), now;

    auto rs = Poco::Data::RecordSet(selectStmt);
    if (rs.rowCount() == 1)
    {
        auto f = std::make_unique<FeedLocal>(id);
        f->setURL(url);
        f->setIconURL(iconURL);
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

uint64_t ZapFR::Engine::SourceLocal::addFeed(const std::string& url, uint64_t folder)
{

    FeedFetcher ff;
    auto parsedFeed = ff.parse(url);

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
        Poco::Data::Statement insertStmt(*(msDatabase->session()));
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
        Poco::Data::Statement selectStmt(*(msDatabase->session()));
        selectStmt << "SELECT last_insert_rowid()", into(feedID), range(0, 1);
        selectStmt.execute();
    }

    auto feed = getFeed(feedID);
    if (feed.has_value())
    {
        feed.value()->refresh();
    }
    return feedID;
}

uint64_t ZapFR::Engine::SourceLocal::getNextFeedSortOrder(uint64_t folder) const
{
    uint64_t sortOrder{0};
    Poco::Data::Statement selectStmt(*(msDatabase->session()));
    selectStmt << "SELECT MAX(sortOrder) FROM feeds WHERE folder=?", into(sortOrder), use(folder), now;
    return sortOrder + 10;
}

uint64_t ZapFR::Engine::SourceLocal::getNextFolderSortOrder(uint64_t folder) const
{
    uint64_t sortOrder{0};
    Poco::Data::Statement selectStmt(*(msDatabase->session()));
    selectStmt << "SELECT MAX(sortOrder) FROM folders WHERE parent=?", into(sortOrder), use(folder), now;
    return sortOrder + 10;
}

void ZapFR::Engine::SourceLocal::moveFeed(uint64_t feedID, uint64_t newFolder, uint64_t newSortOrder)
{
    uint64_t oldFolder{0};
    Poco::Data::Statement selectStmt(*(msDatabase->session()));
    selectStmt << "SELECT folder FROM feeds WHERE id=?", use(feedID), into(oldFolder), now;

    Poco::Data::Statement updateStmt(*(msDatabase->session()));
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
        Poco::Data::Statement updateStmt(*(msDatabase->session()));
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
    Poco::Data::Statement selectStmt(*(msDatabase->session()));
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
        Poco::Data::Statement updateStmt(*(msDatabase->session()));
        updateStmt << "UPDATE feeds SET sortOrder=? WHERE id=?", use(sortOrder), use(f), now;
        sortOrder += 10;
    }
}

void ZapFR::Engine::SourceLocal::resortFolders(uint64_t folder) const
{
    std::vector<uint64_t> folderIDs;

    uint64_t folderID{0};
    Poco::Data::Statement selectStmt(*(msDatabase->session()));
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
        Poco::Data::Statement updateStmt(*(msDatabase->session()));
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
            Poco::Data::Statement deleteStmt(*(msDatabase->session()));
            deleteStmt << "DELETE FROM feeds WHERE id=?", use(feedID), now;
        }

        {
            Poco::Data::Statement deleteStmt(*(msDatabase->session()));
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

        // get all the ID's of the chosen folder and its subfolders
        std::vector<uint64_t> folderIDs;
        getSubfolderIDs(folder, folderIDs, folder != 0);
        std::stringstream ss;
        std::copy(folderIDs.begin(), folderIDs.end(), std::ostream_iterator<int>(ss, ","));
        auto joinedFolderIDs = ss.str();
        joinedFolderIDs = joinedFolderIDs.substr(0, joinedFolderIDs.length() - 1);

        // get all feeds that are in the (sub)folders
        std::vector<std::string> affectedFeedIDs;
        auto selectFeedsSQL = Poco::format("SELECT id FROM feeds WHERE folder IN (%s)", joinedFolderIDs);
        uint64_t feedID{0};
        Poco::Data::Statement selectStmt(*(msDatabase->session()));
        selectStmt << selectFeedsSQL, into(feedID), range(0, 1);
        while (!selectStmt.done())
        {
            if (selectStmt.execute() > 0)
            {
                affectedFeedIDs.emplace_back(std::to_string(feedID));

                auto feed = FeedLocal(feedID);
                feed.removeIcon();
            }
        }

        // remove feeds and their posts
        if (affectedFeedIDs.size() > 0)
        {
            auto joinedFeedIDs = Helpers::joinString(affectedFeedIDs, ",");

            // remove all posts from the affected feeds
            auto deletePostsSQL = Poco::format("DELETE FROM posts WHERE feedID IN (%s)", joinedFeedIDs);
            Poco::Data::Statement deletePostsStmt(*(msDatabase->session()));
            deletePostsStmt << deletePostsSQL, now;

            // remove all affected feeds
            auto deleteFeedsSQL = Poco::format("DELETE FROM feeds WHERE id IN (%s)", joinedFeedIDs);
            Poco::Data::Statement deleteFeedsStmt(*(msDatabase->session()));
            deleteFeedsStmt << deleteFeedsSQL, now;
        }

        // remove folders
        auto deleteFoldersSQL = Poco::format("DELETE FROM folders WHERE id IN (%s)", joinedFolderIDs);
        Poco::Data::Statement deleteFoldersStmt(*(msDatabase->session()));
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

    Poco::Data::Statement selectStmt(*(msDatabase->session()));
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

    Poco::Data::Statement selectStmt(*(msDatabase->session()));
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
    Poco::Data::Statement selectStmt(*(msDatabase->session()));
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
    Poco::Data::Statement insertStmt(*(msDatabase->session()));
    insertStmt << "INSERT INTO folders (parent,title,sortOrder) VALUES (?, ?, ?)", use(parentID), useRef(title), use(sortOrder);
    const std::lock_guard<std::mutex> lock(msAddFolderMutex);
    insertStmt.execute();
    uint64_t newFolderID{0};
    Poco::Data::Statement selectStmt(*(msDatabase->session()));
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
