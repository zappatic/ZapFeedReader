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

#include "ZapFR/FolderLocal.h"
#include "ZapFR/Helpers.h"
#include "ZapFR/Log.h"
#include "ZapFR/PostLocal.h"

using namespace Poco::Data::Keywords;

ZapFR::Engine::FolderLocal::FolderLocal(uint64_t id, uint64_t parent) : Folder(id, parent)
{
}

bool ZapFR::Engine::FolderLocal::fetchData()
{
    if (!mDataFetched)
    {
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << "SELECT title"
                      ",sortOrder"
                      " FROM folders"
                      " WHERE id=?",
            use(mID), into(mTitle), into(mSortOrder), now;

        mDataFetched = true;
        auto rs = Poco::Data::RecordSet(selectStmt);
        return (rs.rowCount() == 1);
    }
    return true;
}

void ZapFR::Engine::FolderLocal::fetchSubfolders()
{
    if (!mSubfoldersFetched)
    {
        uint64_t subID;
        uint64_t subSortOrder;
        std::string subTitle;

        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << "SELECT id"
                      ",title"
                      ",sortOrder"
                      " FROM folders"
                      " WHERE parent=?"
                      " ORDER BY sortOrder ASC",
            use(mID), into(subID), into(subTitle), into(subSortOrder), range(0, 1);

        while (!selectStmt.done())
        {
            if (selectStmt.execute() > 0)
            {
                auto f = std::make_unique<FolderLocal>(subID, mID);
                f->setTitle(subTitle);
                f->setSortOrder(subSortOrder);
                f->setDataFetched(true);
                mSubfolders.emplace_back(std::move(f));
            }
        }

        mSubfoldersFetched = true;
    }
}

std::vector<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::FolderLocal::getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter,
                                                                                       FlagColor flagColor)
{
    auto joinedFeedIDs = Helpers::joinIDNumbers(feedIDsInFoldersAndSubfolders(), ",");
    if (joinedFeedIDs.empty())
    {
        return {};
    }

    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;
    std::string wildcardSearchFilter = "%" + searchFilter + "%";
    auto fc = Flag::idForFlagColor(flagColor);

    whereClause.emplace_back(Poco::format("posts.feedID IN (%s)", joinedFeedIDs));

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

uint64_t ZapFR::Engine::FolderLocal::getTotalPostCount(bool showOnlyUnread, const std::string& searchFilter, FlagColor flagColor)
{
    auto joinedFeedIDs = Helpers::joinIDNumbers(feedIDsInFoldersAndSubfolders(), ",");
    if (joinedFeedIDs.empty())
    {
        return 0;
    }

    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;
    std::string wildcardSearchFilter = "%" + searchFilter + "%";
    auto fc = Flag::idForFlagColor(flagColor);

    whereClause.emplace_back(Poco::format("posts.feedID IN (%s)", joinedFeedIDs));

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

std::unordered_set<uint64_t> ZapFR::Engine::FolderLocal::markAllAsRead()
{
    auto feedIDs = feedIDsInFoldersAndSubfolders();
    auto joinedFeedIDs = Helpers::joinIDNumbers(feedIDs, ",");
    if (joinedFeedIDs.empty())
    {
        return {};
    }

    Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
    updateStmt << Poco::format("UPDATE posts SET isRead=TRUE WHERE feedID IN (%s)", joinedFeedIDs), now;
    updateStmt.execute();

    return std::unordered_set<uint64_t>(feedIDs.begin(), feedIDs.end());
}

std::vector<uint64_t> ZapFR::Engine::FolderLocal::folderAndSubfolderIDs() const
{
    std::vector<uint64_t> folderIDs{};
    std::function<void(uint64_t)> fetchSubfolderIDs;
    fetchSubfolderIDs = [&](uint64_t parent)
    {
        folderIDs.emplace_back(parent);
        uint64_t folderID{0};
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << "SELECT id FROM folders WHERE parent=?", use(parent), into(folderID), range(0, 1);
        while (!selectStmt.done())
        {
            if (selectStmt.execute() > 0)
            {
                fetchSubfolderIDs(folderID);
            }
        }
    };
    fetchSubfolderIDs(mID);
    return folderIDs;
}

std::vector<uint64_t> ZapFR::Engine::FolderLocal::feedIDsInFoldersAndSubfolders() const
{
    auto folderIDs = folderAndSubfolderIDs();
    if (folderIDs.size() == 0)
    {
        return {};
    }

    auto joinedFolderIDs = Helpers::joinIDNumbers(folderIDs, ",");

    std::vector<uint64_t> feedIDs;
    auto selectFeedsSQL = Poco::format("SELECT id FROM feeds WHERE folder IN (%s)", joinedFolderIDs);
    uint64_t feedID{0};
    Poco::Data::Statement selectFeedsStmt(*(Database::getInstance()->session()));
    selectFeedsStmt << selectFeedsSQL, into(feedID), range(0, 1);
    while (!selectFeedsStmt.done())
    {
        if (selectFeedsStmt.execute() > 0)
        {
            feedIDs.emplace_back(feedID);
        }
    }
    return feedIDs;
}

std::vector<std::unique_ptr<ZapFR::Engine::Log>> ZapFR::Engine::FolderLocal::getLogs(uint64_t perPage, uint64_t page)
{
    auto joinedFeedIDs = Helpers::joinIDNumbers(feedIDsInFoldersAndSubfolders(), ",");
    if (joinedFeedIDs.empty())
    {
        return {};
    }

    std::vector<std::unique_ptr<Log>> logs;

    auto offset = perPage * (page - 1);

    uint64_t id{0};
    std::string timestamp{""};
    uint64_t level;
    std::string message{""};
    Poco::Nullable<uint64_t> feedID{0};
    Poco::Nullable<std::string> feedTitle{};

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << Poco::format("SELECT logs.id"
                               ",logs.timestamp"
                               ",logs.level"
                               ",logs.message"
                               ",logs.feedID"
                               ",feeds.title"
                               " FROM logs"
                               " LEFT JOIN feeds ON feeds.id = logs.feedID"
                               " WHERE logs.feedID IN (%s)"
                               " ORDER BY logs.id DESC"
                               " LIMIT ? OFFSET ?",
                               joinedFeedIDs),
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

uint64_t ZapFR::Engine::FolderLocal::getTotalLogCount()
{
    auto joinedFeedIDs = Helpers::joinIDNumbers(feedIDsInFoldersAndSubfolders(), ",");
    if (joinedFeedIDs.empty())
    {
        return 0;
    }
    uint64_t logCount;
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << Poco::format("SELECT COUNT(*) FROM logs WHERE feedID IN (%s)", joinedFeedIDs), into(logCount), now;
    return logCount;
}
