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

std::vector<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::FolderLocal::getPosts(uint64_t perPage, uint64_t page)
{
    // fetch all folderIDs of all the subfolders of this folder
    std::unordered_set<uint64_t> folderIDs{};
    std::function<void(uint64_t)> fetchSubfolderIDs;
    fetchSubfolderIDs = [&](uint64_t parentFolderID)
    {
        folderIDs.insert(parentFolderID);
        uint64_t folderID{0};
        Poco::Data::Statement selectStmt(*(msDatabase->session()));
        selectStmt << "SELECT id FROM folders WHERE parent=?", use(parentFolderID), into(folderID), range(0, 1);
        while (!selectStmt.done())
        {
            if (selectStmt.execute() > 0)
            {
                fetchSubfolderIDs(folderID);
            }
        }
    };
    fetchSubfolderIDs(mID);
    if (folderIDs.size() == 0)
    {
        return {};
    }

    std::stringstream ss;
    std::copy(folderIDs.begin(), folderIDs.end(), std::ostream_iterator<int>(ss, ","));
    auto joinedFolderIDs = ss.str();
    joinedFolderIDs = joinedFolderIDs.substr(0, joinedFolderIDs.length() - 1);

    // fetch all the feed ID's that are within the queried subfolders
    std::vector<std::string> feedIDs;
    auto selectFeedsSQL = Poco::format("SELECT id FROM feeds WHERE folder IN (%s)", joinedFolderIDs);
    uint64_t feedID{0};
    Poco::Data::Statement selectFeedsStmt(*(msDatabase->session()));
    selectFeedsStmt << selectFeedsSQL, into(feedID), range(0, 1);
    while (!selectFeedsStmt.done())
    {
        if (selectFeedsStmt.execute() > 0)
        {
            feedIDs.emplace_back(std::to_string(feedID));
        }
    }
    if (feedIDs.size() == 0)
    {
        return {};
    }
    auto joinedFeedIDs = Helpers::joinString(feedIDs, ",");

    // fetch the posts for all the queried feeds
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
                               " ORDER BY datePublished DESC"
                               " LIMIT ? OFFSET ?",
                               joinedFeedIDs),
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