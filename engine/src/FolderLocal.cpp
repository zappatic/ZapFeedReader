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

#include "FolderLocal.h"
#include "Helpers.h"
#include "Log.h"
#include "Post.h"

using namespace Poco::Data::Keywords;

ZapFR::Engine::FolderLocal::FolderLocal(uint64_t id, uint64_t parent) : Folder(id, parent)
{
}

bool ZapFR::Engine::FolderLocal::fetchData()
{
    if (!mDataFetched)
    {
        Poco::Data::Statement selectStmt(*(msDatabase->session()));
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

        Poco::Data::Statement selectStmt(*(msDatabase->session()));
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

std::vector<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::FolderLocal::getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread)
{
    auto joinedFeedIDs = Helpers::joinIDNumbers(feedIDsInFoldersAndSubfolders(), ",");
    if (joinedFeedIDs.empty())
    {
        return {};
    }

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
    std::string whereClause = showOnlyUnread ? "AND isRead=FALSE" : "";

    Poco::Data::Statement selectStmt(*(msDatabase->session()));
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
                               " WHERE feedID IN (%s)"
                               "       %s"
                               " ORDER BY datePublished DESC"
                               " LIMIT ? OFFSET ?",
                               joinedFeedIDs, whereClause),
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

void ZapFR::Engine::FolderLocal::markAllAsRead()
{
    auto joinedFeedIDs = Helpers::joinIDNumbers(feedIDsInFoldersAndSubfolders(), ",");
    if (joinedFeedIDs.empty())
    {
        return;
    }

    Poco::Data::Statement updateStmt(*(msDatabase->session()));
    updateStmt << Poco::format("UPDATE posts SET isRead=TRUE WHERE feedID IN (%s)", joinedFeedIDs), now;
    updateStmt.execute();
}

std::vector<uint64_t> ZapFR::Engine::FolderLocal::folderAndSubfolderIDs() const
{
    std::vector<uint64_t> folderIDs{};
    std::function<void(uint64_t)> fetchSubfolderIDs;
    fetchSubfolderIDs = [&](uint64_t parent)
    {
        folderIDs.emplace_back(parent);
        uint64_t folderID{0};
        Poco::Data::Statement selectStmt(*(msDatabase->session()));
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
    Poco::Data::Statement selectFeedsStmt(*(msDatabase->session()));
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

uint64_t ZapFR::Engine::FolderLocal::getTotalPostCount(bool showOnlyUnread)
{
    auto joinedFeedIDs = Helpers::joinIDNumbers(feedIDsInFoldersAndSubfolders(), ",");
    if (joinedFeedIDs.empty())
    {
        return 0;
    }

    std::string whereClause = showOnlyUnread ? " AND isRead=FALSE" : "";

    uint64_t postCount;
    Poco::Data::Statement selectStmt(*(msDatabase->session()));
    selectStmt << Poco::format("SELECT COUNT(*) FROM posts WHERE feedID IN (%s) %s", joinedFeedIDs, whereClause), into(postCount), now;
    return postCount;
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

    Poco::Data::Statement selectStmt(*(msDatabase->session()));
    selectStmt << Poco::format("SELECT id"
                               ",timestamp"
                               ",level"
                               ",message"
                               ",feedID"
                               " FROM logs"
                               " WHERE feedID IN (%s)"
                               " ORDER BY id DESC"
                               " LIMIT ? OFFSET ?",
                               joinedFeedIDs),
        use(perPage), use(offset), into(id), into(timestamp), into(level), into(message), into(feedID), range(0, 1);

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
            logs.emplace_back(std::move(l));
        }
    }

    return logs;
}
