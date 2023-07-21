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
#include "Helpers.h"

using namespace Poco::Data::Keywords;

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
    std::string folderHierarchy;
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
                  ",folderHierarchy"
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
        into(id), into(url), into(iconURL), into(iconLastFetched), into(folderHierarchy), into(guid), into(title), into(subtitle), into(link), into(description),
        into(language), into(copyright), into(lastChecked), into(sortOrder), range(0, 1);

    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            auto f = std::make_unique<FeedLocal>(id);
            f->setURL(url);
            f->setIconURL(iconURL);
            f->setIconLastFetched(iconLastFetched);
            f->setFolderHierarchy(folderHierarchy);
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
    std::string folderHierarchy;
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
                  ",folderHierarchy"
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
        use(feedID), into(id), into(url), into(iconURL), into(iconLastFetched), into(folderHierarchy), into(guid), into(title), into(subtitle), into(link), into(description),
        into(language), into(copyright), into(lastChecked), into(sortOrder), now;

    auto rs = Poco::Data::RecordSet(selectStmt);
    if (rs.rowCount() == 1)
    {
        auto f = std::make_unique<FeedLocal>(id);
        f->setURL(url);
        f->setIconURL(iconURL);
        f->setIconLastFetched(iconLastFetched);
        f->setFolderHierarchy(folderHierarchy);
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

void ZapFR::Engine::SourceLocal::addFeed(const std::string& url, const std::string& folderHierarchy)
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
    auto sortOrder = getNextFeedSortOrder("");

    // scope for insert feed mutex lock
    {
        Poco::Data::Statement insertStmt(*(msDatabase->session()));
        insertStmt << "INSERT INTO feeds ("
                      " url"
                      ",iconURL"
                      ",folderHierarchy"
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
            useRef(url), useRef(iconURL), useRef(folderHierarchy), useRef(guid), useRef(title), useRef(subtitle), useRef(link), useRef(description), useRef(language),
            useRef(copyright), use(sortOrder);
        const std::lock_guard<std::mutex> lock(mInsertFeedMutex);
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
}

uint64_t ZapFR::Engine::SourceLocal::getNextFeedSortOrder(const std::string& folderHierarchy) const
{
    uint64_t sortOrder{0};
    Poco::Data::Statement selectStmt(*(msDatabase->session()));
    selectStmt << "SELECT MAX(sortOrder) FROM feeds WHERE folderHierarchy=?", into(sortOrder), useRef(folderHierarchy), now;
    return sortOrder + 10;
}

void ZapFR::Engine::SourceLocal::moveFeed(uint64_t feedID, const std::string& newFolderHierarchy, uint64_t newSortOrder)
{
    Poco::Data::Statement updateStmt(*(msDatabase->session()));
    updateStmt << "UPDATE feeds SET folderHierarchy=?, sortOrder=? WHERE id=?", useRef(newFolderHierarchy), use(newSortOrder), use(feedID), now;
    resort(newFolderHierarchy);
}

void ZapFR::Engine::SourceLocal::resort(const std::string& folderHierarchy) const
{
    std::vector<uint64_t> feedIDs;

    uint64_t feedID{0};
    Poco::Data::Statement selectStmt(*(msDatabase->session()));
    selectStmt << "SELECT id FROM feeds WHERE folderHierarchy=? ORDER BY sortOrder ASC", useRef(folderHierarchy), into(feedID), range(0, 1);
    auto rs = Poco::Data::RecordSet(selectStmt);
    if (rs.rowCount() > 0)
    {
        while (!selectStmt.done())
        {
            selectStmt.execute();
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

void ZapFR::Engine::SourceLocal::removeFeed(uint64_t feedID)
{
    auto feed = getFeed(feedID);
    if (feed.has_value())
    {
        auto folderHierarchy = feed.value()->folderHierarchy();

        {
            Poco::Data::Statement deleteStmt(*(msDatabase->session()));
            deleteStmt << "DELETE FROM feeds WHERE id=?", use(feedID), now;
        }

        {
            Poco::Data::Statement deleteStmt(*(msDatabase->session()));
            deleteStmt << "DELETE FROM posts WHERE feedID=?", use(feedID), now;
        }

        resort(folderHierarchy);
    }
}

void ZapFR::Engine::SourceLocal::removeFolder(const std::string& folderHierarchy)
{
    // find all the feed ID's for the respective folder hierarchy, so we can remove all the posts of those ID's
    std::vector<std::string> affectedFeedIDs;
    {
        auto sql = Poco::format("SELECT id FROM feeds WHERE folderHierarchy LIKE '%s'", Poco::replace(folderHierarchy, "'", "''"));

        uint64_t feedID{0};
        Poco::Data::Statement selectStmt(*(msDatabase->session()));
        selectStmt << sql, into(feedID), range(0, 1);
        while (!selectStmt.done())
        {
            if (selectStmt.execute() > 0)
            {
                affectedFeedIDs.emplace_back(std::to_string(feedID));
            }
        }
    }
    if (affectedFeedIDs.size() > 0)
    {
        auto feedIDs = Helpers::joinString(affectedFeedIDs, ",");

        // remove all posts from the affected feeds
        auto deletePostsSQL = Poco::format("DELETE FROM posts WHERE feedID IN (%s)", feedIDs);
        Poco::Data::Statement deletePostsStmt(*(msDatabase->session()));
        deletePostsStmt << deletePostsSQL, now;

        // remove all affected feeds
        auto deleteFeedsSQL = Poco::format("DELETE FROM feeds WHERE id IN (%s)", feedIDs);
        Poco::Data::Statement deleteFeedsStmt(*(msDatabase->session()));
        deleteFeedsStmt << deleteFeedsSQL, now;
    }
}
