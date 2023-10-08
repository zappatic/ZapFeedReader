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
#include "ZapFR/Log.h"
#include "ZapFR/local/FeedLocal.h"
#include "ZapFR/local/FolderLocal.h"
#include "ZapFR/local/PostLocal.h"

using namespace Poco::Data::Keywords;

std::mutex ZapFR::Engine::FolderLocal::msCreateFolderMutex{};
std::mutex ZapFR::Engine::FolderLocal::msCreateFolderHierarchyMutex{};

ZapFR::Engine::FolderLocal::FolderLocal(uint64_t id, uint64_t parentFolderID, Source* parentSource) : Folder(id, parentFolderID, parentSource)
{
}

void ZapFR::Engine::FolderLocal::fetchSubfolders()
{
    if (!mSubfoldersFetched)
    {
        std::vector<std::string> whereClause;
        std::vector<Poco::Data::AbstractBinding::Ptr> bindings;
        whereClause.emplace_back("parent=?");
        bindings.emplace_back(useRef(mID, "parent"));
        mSubfolders = queryMultiple(mParentSource, whereClause, "ORDER BY folders.sortOrder ASC", "", bindings);
        mSubfoldersFetched = true;
    }
}

void ZapFR::Engine::FolderLocal::fetchStatistics()
{
    mStatistics.clear();

    auto feedIDs = feedIDsInFoldersAndSubfolders();
    auto joinedFeedIDs = Helpers::joinIDNumbers(feedIDs, ",");
    if (joinedFeedIDs.empty())
    {
        mStatistics[Statistic::FeedCount] = "0";
        mStatistics[Statistic::PostCount] = "0";
        mStatistics[Statistic::FlaggedPostCount] = "0";
        mStatistics[Statistic::OldestPost] = "";
        mStatistics[Statistic::NewestPost] = "";
        return;
    }

    // total feed count
    {
        mStatistics[Statistic::FeedCount] = std::to_string(feedIDs.size());
    }

    // total post count
    {
        uint64_t totalPostCount{0};
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << Poco::format("SELECT COUNT(*) FROM posts WHERE feedID IN (%s)", joinedFeedIDs), into(totalPostCount), now;
        mStatistics[Statistic::PostCount] = std::to_string(totalPostCount);
    }

    // total flagged post count
    {
        uint64_t totalFlaggedPostCount{0};
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << Poco::format("SELECT COUNT(*)"
                                   " FROM flags"
                                   " LEFT JOIN posts ON posts.id = flags.postID"
                                   " WHERE posts.feedID IN (%s)",
                                   joinedFeedIDs),
            into(totalFlaggedPostCount), use(mID), now;
        mStatistics[Statistic::FlaggedPostCount] = std::to_string(totalFlaggedPostCount);
    }

    // oldest post
    {
        Poco::Nullable<std::string> oldestPost{};
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << Poco::format("SELECT MIN(datePublished) FROM posts WHERE feedID IN (%s)", joinedFeedIDs), into(oldestPost), use(mID), now;
        mStatistics[Statistic::OldestPost] = oldestPost.isNull() ? "" : oldestPost.value();
    }

    // newest post
    {
        Poco::Nullable<std::string> newestPost{};
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << Poco::format("SELECT MAX(datePublished) FROM posts WHERE feedID IN (%s)", joinedFeedIDs), into(newestPost), use(mID), now;
        mStatistics[Statistic::NewestPost] = newestPost.isNull() ? "" : newestPost.value();
    }
}

std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Post>>>
ZapFR::Engine::FolderLocal::getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter, uint64_t categoryFilterID, FlagColor flagColor)
{
    auto joinedFeedIDs = Helpers::joinIDNumbers(feedIDsInFoldersAndSubfolders(), ",");
    if (joinedFeedIDs.empty())
    {
        return {};
    }

    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindingsPostQuery;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindingsCountQuery;
    std::string wildcardSearchFilter = "%" + searchFilter + "%";
    auto fc = Flag::idForFlagColor(flagColor);

    whereClause.emplace_back(Poco::format("posts.feedID IN (%s)", joinedFeedIDs));

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

    auto posts = PostLocal::queryMultiple(whereClause, "ORDER BY posts.datePublished DESC", "LIMIT ? OFFSET ?", bindingsPostQuery);
    auto count = PostLocal::queryCount(whereClause, bindingsCountQuery);
    return std::make_tuple(count, std::move(posts));
}

std::vector<uint64_t> ZapFR::Engine::FolderLocal::markAsRead(uint64_t maxPostID)
{
    auto feedIDs = feedIDsInFoldersAndSubfolders();
    auto joinedFeedIDs = Helpers::joinIDNumbers(feedIDs, ",");
    if (joinedFeedIDs.empty())
    {
        return {};
    }

    if (maxPostID == std::numeric_limits<uint64_t>::max())
    {
        PostLocal::updateIsRead(true, {Poco::format("posts.feedID IN (%s)", joinedFeedIDs)}, {});
    }
    else
    {
        PostLocal::updateIsRead(true, {Poco::format("posts.feedID IN (%s)", joinedFeedIDs), "posts.id <= ?"}, {use(maxPostID, "maxPostID")});
    }

    return feedIDs;
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

const std::vector<uint64_t>& ZapFR::Engine::FolderLocal::feedIDsInFoldersAndSubfolders()
{
    if (!mFeedIDsFetched)
    {
        fetchFeedIDsInFoldersAndSubfolders();
    }
    return mFeedIDs;
}

void ZapFR::Engine::FolderLocal::fetchFeedIDsInFoldersAndSubfolders()
{
    mFeedIDs.clear();

    auto folderIDs = folderAndSubfolderIDs();
    if (folderIDs.size() == 0)
    {
        return;
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
            mFeedIDs.emplace_back(feedID);
        }
    }
}

void ZapFR::Engine::FolderLocal::fetchThumbnailData()
{
    mThumbnailData.clear();

    auto joinedFeedIDs = Helpers::joinIDNumbers(feedIDsInFoldersAndSubfolders(), ",");
    if (joinedFeedIDs.empty())
    {
        return;
    }

    std::vector<std::string> whereClause;

    whereClause.emplace_back(Poco::format("posts.feedID IN (%s)", joinedFeedIDs));
    whereClause.emplace_back("posts.isRead=FALSE");
    whereClause.emplace_back("posts.thumbnail NOT NULL");

    auto posts = PostLocal::queryMultiple(whereClause, "", "LIMIT 250", {});

    std::unordered_map<uint64_t, std::tuple<std::string, std::string>> feedIDToTitleAndLinkMap;
    std::unordered_map<uint64_t, std::vector<Post*>> feedIDToPosts;
    for (const auto& post : posts)
    {
        auto feedID = post->feedID();
        feedIDToTitleAndLinkMap[feedID] = std::make_tuple(post->feedTitle(), post->feedLink());
        if (!feedIDToPosts.contains(feedID))
        {
            feedIDToPosts[feedID] = {};
        }
        feedIDToPosts.at(feedID).emplace_back(post.get());
    }

    // sort by title of the feed
    std::vector<std::tuple<uint64_t, std::string>> feedIDToTitleVector;
    for (const auto& [feedID, feedData] : feedIDToTitleAndLinkMap)
    {
        feedIDToTitleVector.emplace_back(feedID, std::get<0>(feedData));
    }
    std::sort(feedIDToTitleVector.begin(), feedIDToTitleVector.end(),
              [](const std::tuple<uint64_t, std::string>& a, const std::tuple<uint64_t, std::string>& b) { return Poco::icompare(std::get<1>(a), std::get<1>(b)) < 0; });

    for (const auto& [feedID, dummy] : feedIDToTitleVector)
    {
        ThumbnailData td;
        td.feedID = feedID;
        const auto& [feedTitle, feedLink] = feedIDToTitleAndLinkMap.at(feedID);
        td.feedTitle = feedTitle;
        td.feedLink = feedLink;
        for (const auto& post : feedIDToPosts.at(feedID))
        {
            Poco::DateTime datePublished{};
            int32_t tzd{0};
            Poco::DateTimeParser::tryParse(post->datePublished(), datePublished, tzd);
            auto timestamp = datePublished.timestamp().epochTime();

            td.posts.emplace_back(post->id(), post->title(), post->thumbnail(), post->link(), timestamp);
        }
        std::sort(td.posts.begin(), td.posts.end(), [](const ThumbnailDataPost& a, const ThumbnailDataPost& b) { return (std::difftime(a.timestamp, b.timestamp) > 0); });
        mThumbnailData.emplace_back(td);
    }
}

std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Log>>> ZapFR::Engine::FolderLocal::getLogs(uint64_t perPage, uint64_t page)
{
    auto joinedFeedIDs = Helpers::joinIDNumbers(feedIDsInFoldersAndSubfolders(), ",");
    if (joinedFeedIDs.empty())
    {
        return {};
    }

    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;

    whereClause.emplace_back(Poco::format("logs.feedID IN (%s)", joinedFeedIDs));

    auto offset = perPage * (page - 1);
    bindings.emplace_back(use(perPage, "perPage"));
    bindings.emplace_back(use(offset, "offset"));

    auto logs = Log::queryMultiple(whereClause, "ORDER BY logs.id DESC", "LIMIT ? OFFSET ?", bindings);
    auto logCount = Log::queryCount(whereClause, {});
    return std::make_tuple(logCount, std::move(logs));
}

void ZapFR::Engine::FolderLocal::clearLogs()
{
    auto joinedFeedIDs = Helpers::joinIDNumbers(feedIDsInFoldersAndSubfolders(), ",");
    if (joinedFeedIDs.empty())
    {
        return;
    }

    Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
    deleteStmt << Poco::format("DELETE FROM logs WHERE feedID IN (%s)", joinedFeedIDs), now;
}

std::vector<std::unique_ptr<ZapFR::Engine::Category>> ZapFR::Engine::FolderLocal::getCategories()
{
    auto joinedFeedIDs = Helpers::joinIDNumbers(feedIDsInFoldersAndSubfolders(), ",");
    if (joinedFeedIDs.empty())
    {
        return {};
    }

    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;

    whereClause.emplace_back(Poco::format("categories.feedID IN (%s)", joinedFeedIDs));
    return Category::queryMultiple(true, whereClause, "ORDER BY categories.title ASC", "", {});
}

uint64_t ZapFR::Engine::FolderLocal::nextSortOrder(uint64_t folderID)
{
    uint64_t sortOrder{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT MAX(sortOrder) FROM folders WHERE parent=?", into(sortOrder), use(folderID), now;
    return sortOrder + 10;
}

std::tuple<uint64_t, uint64_t> ZapFR::Engine::FolderLocal::create(uint64_t parentID, const std::string& title)
{
    auto sortOrder = nextSortOrder(parentID);
    Poco::Data::Statement insertStmt(*(Database::getInstance()->session()));
    insertStmt << "INSERT INTO folders (parent,title,sortOrder) VALUES (?, ?, ?)", use(parentID), useRef(title), use(sortOrder);
    const std::lock_guard<std::mutex> lock(msCreateFolderMutex);
    insertStmt.execute();
    uint64_t newFolderID{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT last_insert_rowid()", into(newFolderID), range(0, 1);
    selectStmt.execute();
    return std::make_tuple(newFolderID, sortOrder);
}

void ZapFR::Engine::FolderLocal::remove(Source* parentSource, uint64_t folderID)
{
    // get the parent id for this folder
    auto f = querySingle(parentSource, {"folders.id=?"}, {useRef(folderID, "folderID")});
    if (f.has_value())
    {
        auto fLocal = dynamic_cast<FolderLocal*>(f.value().get());
        auto folderParent = f.value()->parentID();

        auto feedIDs = f.value()->feedIDsInFoldersAndSubfolders();
        auto folderIDs = fLocal->folderAndSubfolderIDs();

        // remove the icons from the cache
        for (const auto& feedID : feedIDs)
        {
            auto feed = FeedLocal(feedID, parentSource);
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
        resort(folderParent);
    }
}

void ZapFR::Engine::FolderLocal::resort(uint64_t parentID)
{
    std::vector<uint64_t> folderIDs;

    uint64_t folderID{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT id FROM folders WHERE parent=? ORDER BY sortOrder ASC", use(parentID), into(folderID), range(0, 1);
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

uint64_t ZapFR::Engine::FolderLocal::createFolderHierarchy(Source* parentSource, uint64_t parentID, const std::vector<std::string> folderHierarchy)
{
    std::function<uint64_t(ZapFR::Engine::Folder*, const std::string&)> getSubfolderWithTitle;
    getSubfolderWithTitle = [&](ZapFR::Engine::Folder* parent, const std::string& folderTitle) -> uint64_t
    {
        auto existingSubFolderFound{false};
        uint64_t existingSubfolderID{0};
        uint64_t pID{0};

        if (parent == nullptr)
        {
            uint64_t rootParentID{0};
            for (const auto& subfolder : queryMultiple(parentSource, {"folders.parent=?"}, "", "", {use(rootParentID, "parent")}))
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
            dynamic_cast<FolderLocal*>(parent)->fetchSubfolders();
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
            uint64_t sortOrder;
            std::tie(existingSubfolderID, sortOrder) = create(pID, folderTitle);
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
            auto subfolderInstance = querySingle(parentSource, {"folders.id=?"}, {use(subfolderID, "id")});
            if (subfolderInstance.has_value())
            {
                currentParent = std::move(subfolderInstance.value());
            }
        }
        return currentParent->id();
    }
    else
    {
        auto rootFolder = querySingle(parentSource, {"folders.id=?"}, {use(parentID, "id")});
        if (rootFolder.has_value())
        {
            auto currentParent = std::move(rootFolder.value());
            for (const auto& subfolder : folderHierarchy)
            {
                auto subfolderID = getSubfolderWithTitle(currentParent.get(), subfolder);
                auto subfolderInstance = querySingle(parentSource, {"folders.id=?"}, {use(subfolderID, "id")});
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

std::unordered_map<uint64_t, uint64_t> ZapFR::Engine::FolderLocal::move(Source* parentSource, uint64_t folderID, uint64_t newParent, uint64_t newSortOrder)
{
    std::unordered_map<uint64_t, uint64_t> affectedFolderIDs;

    auto f = querySingle(parentSource, {"folders.id=?"}, {use(folderID, "id")});
    if (f.has_value())
    {
        Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
        updateStmt << "UPDATE folders SET parent=?, sortOrder=? WHERE id=?", use(newParent), use(newSortOrder), use(folderID), now;

        auto oldParent = f.value()->parentID();

        std::vector<uint64_t> parentFoldersToQuery{oldParent};
        resort(oldParent);
        if (newParent != oldParent) // check in case we are moving within the same folder
        {
            parentFoldersToQuery.emplace_back(newParent);
            resort(newParent);
        }

        auto parentFolderIDs = Helpers::joinIDNumbers(parentFoldersToQuery, ",");
        uint64_t affectedFolderID{0};
        uint64_t affectedSortOrder{0};
        Poco::Data::Statement affectedSelectStmt(*(Database::getInstance()->session()));
        affectedSelectStmt << Poco::format("SELECT id,sortOrder FROM folders WHERE parent IN (%s)", parentFolderIDs), into(affectedFolderID), into(affectedSortOrder),
            range(0, 1);
        while (!affectedSelectStmt.done())
        {
            if (affectedSelectStmt.execute() > 0)
            {
                affectedFolderIDs[affectedFolderID] = affectedSortOrder;
            }
        }
    }

    return affectedFolderIDs;
}

void ZapFR::Engine::FolderLocal::update(const std::string& newTitle)
{
    Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
    updateStmt << "UPDATE folders SET title=? WHERE id=?", useRef(newTitle), use(mID), now;
}

std::tuple<const std::unordered_map<uint64_t, uint64_t>, const std::unordered_map<uint64_t, uint64_t>> ZapFR::Engine::FolderLocal::sort(SortMethod sortMethod)
{
    struct Sortable
    {
        Sortable(uint64_t sortableID, const std::string& sortableTitle) : id(sortableID), title(sortableTitle) {}
        uint64_t id;
        std::string title;
    };

    // sort feeds
    std::unordered_map<uint64_t, uint64_t> feedSortOrders{};
    std::vector<Sortable> feeds{};
    {
        uint64_t feedID{0};
        std::string title{""};
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << "SELECT id,title FROM feeds WHERE folder=?", use(mID), into(feedID), into(title), range(0, 1);
        while (!selectStmt.done())
        {
            if (selectStmt.execute() > 0)
            {
                feeds.emplace_back(feedID, title);
            }
        }
    }

    if (feeds.size() > 0)
    {
        switch (sortMethod)
        {
            case SortMethod::AlphabeticallyAscending:
            {
                std::sort(feeds.begin(), feeds.end(), [](const Sortable& a, const Sortable& b) { return Poco::icompare(a.title, b.title) < 0; });
                break;
            }
        }

        uint64_t sortOrder{10};
        for (const auto& sortable : feeds)
        {
            uint64_t feedID = sortable.id;
            Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
            updateStmt << "UPDATE feeds SET sortOrder=? WHERE id=?", use(sortOrder), use(feedID), now;
            feedSortOrders[feedID] = sortOrder;
            sortOrder += 10;
        }
    }

    // sort folders
    std::unordered_map<uint64_t, uint64_t> folderSortOrders{};
    std::vector<Sortable> folders{};
    {
        uint64_t folderID{0};
        std::string title{""};
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << "SELECT id,title FROM folders WHERE parent=?", use(mID), into(folderID), into(title), range(0, 1);
        while (!selectStmt.done())
        {
            if (selectStmt.execute() > 0)
            {
                folders.emplace_back(folderID, title);
            }
        }
    }

    if (folders.size() > 0)
    {
        switch (sortMethod)
        {
            case SortMethod::AlphabeticallyAscending:
            {
                std::sort(folders.begin(), folders.end(), [](const Sortable& a, const Sortable& b) { return Poco::icompare(a.title, b.title) < 0; });
                break;
            }
        }

        uint64_t sortOrder{10};
        for (const auto& sortable : folders)
        {
            uint64_t folderID = sortable.id;
            Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
            updateStmt << "UPDATE folders SET sortOrder=? WHERE id=?", use(sortOrder), use(folderID), now;
            folderSortOrders[folderID] = sortOrder;
            sortOrder += 10;
        }
    }

    return std::make_tuple(folderSortOrders, feedSortOrders);
}

std::vector<std::unique_ptr<ZapFR::Engine::Folder>> ZapFR::Engine::FolderLocal::queryMultiple(Source* parentSource, const std::vector<std::string>& whereClause,
                                                                                              const std::string& orderClause, const std::string& limitClause,
                                                                                              const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings)
{
    std::vector<std::unique_ptr<Folder>> folders;

    uint64_t id{0};
    uint64_t parentFolderID{0};
    std::string title{""};
    uint64_t sortOrder{0};

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT folders.id"
          ",folders.parent"
          ",folders.title"
          ",folders.sortOrder"
          " FROM folders";
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
    selectStmt.addExtract(into(parentFolderID));
    selectStmt.addExtract(into(title));
    selectStmt.addExtract(into(sortOrder));

    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            auto f = std::make_unique<FolderLocal>(id, parentFolderID, parentSource);
            f->setTitle(title);
            f->setSortOrder(sortOrder);
            f->setDataFetched(true);
            folders.emplace_back(std::move(f));
        }
    }

    return folders;
}

std::optional<std::unique_ptr<ZapFR::Engine::Folder>> ZapFR::Engine::FolderLocal::querySingle(Source* parentSource, const std::vector<std::string>& whereClause,
                                                                                              const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings)
{
    uint64_t id{0};
    uint64_t parentFolderID{0};
    std::string title{""};
    uint64_t sortOrder{0};

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT folders.id"
          ",folders.parent"
          ",folders.title"
          ",folders.sortOrder"
          " FROM folders";
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
    selectStmt.addExtract(into(parentFolderID));
    selectStmt.addExtract(into(title));
    selectStmt.addExtract(into(sortOrder));

    selectStmt.execute();

    auto rs = Poco::Data::RecordSet(selectStmt);
    if (rs.rowCount() == 1)
    {
        auto f = std::make_unique<FolderLocal>(id, parentFolderID, parentSource);
        f->setTitle(title);
        f->setSortOrder(sortOrder);
        f->setDataFetched(true);
        return f;
    }
    return {};
}
