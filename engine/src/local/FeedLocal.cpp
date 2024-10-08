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

#include <Poco/Data/RecordSet.h>
#include <Poco/DigestStream.h>
#include <Poco/FileStream.h>
#include <Poco/MD5Engine.h>
#include <Poco/Path.h>
#include <Poco/StreamCopier.h>

#include "ZapFR/Database.h"
#include "ZapFR/Helpers.h"
#include "ZapFR/Log.h"
#include "ZapFR/base/Script.h"
#include "ZapFR/feed_handling/FavIconParser.h"
#include "ZapFR/feed_handling/FeedFetcher.h"
#include "ZapFR/feed_handling/FeedParser.h"
#include "ZapFR/local/FeedLocal.h"
#include "ZapFR/local/PostLocal.h"
#include "ZapFR/local/ScriptLocal.h"
#include "ZapFR/local/SourceLocal.h"
#include "ZapFR/lua/ScriptLua.h"

using namespace Poco::Data::Keywords;

std::string ZapFR::Engine::FeedLocal::msIconDir{""};
std::mutex ZapFR::Engine::FeedLocal::msCreateFeedMutex{};

ZapFR::Engine::FeedLocal::FeedLocal(uint64_t id, Source* parentSource) : Feed(id, parentSource)
{
}

std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Post>>> ZapFR::Engine::FeedLocal::getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                                                                                           bool showUnreadPostsAtTop, const std::string& searchFilter,
                                                                                                           uint64_t categoryFilterID, FlagColor flagColor)
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindingsPostQuery;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindingsCountQuery;
    std::string wildcardSearchFilter = "%" + searchFilter + "%";
    auto fc = Flag::idForFlagColor(flagColor);

    whereClause.emplace_back("posts.feedID = ?");
    bindingsPostQuery.emplace_back(use(mID, "feedID"));
    bindingsCountQuery.emplace_back(use(mID, "feedID"));

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
        whereClause.emplace_back("posts.id IN (SELECT DISTINCT(postID) FROM post_categories WHERE categoryID=?)");
        bindingsPostQuery.emplace_back(useRef(categoryFilterID, "catFilter"));
        bindingsCountQuery.emplace_back(useRef(categoryFilterID, "catFilter"));
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

std::optional<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::FeedLocal::getPost(uint64_t postID)
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;

    whereClause.emplace_back("posts.feedID=?");
    bindings.emplace_back(use(mID, "feedID"));
    whereClause.emplace_back("posts.id=?");
    bindings.emplace_back(use(postID, "postID"));

    return PostLocal::querySingle(whereClause, bindings);
}

std::optional<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::FeedLocal::getPostByGuid(const std::string& guid)
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;

    whereClause.emplace_back("posts.feedID=?");
    bindings.emplace_back(use(mID, "feedID"));
    whereClause.emplace_back("posts.guid=?");
    bindings.emplace_back(useRef(guid, "guid"));

    return PostLocal::querySingle(whereClause, bindings);
}

void ZapFR::Engine::FeedLocal::fetchData()
{
    if (!mDataFetched)
    {
        Poco::Nullable<std::string> lastRefreshError;
        Poco::Nullable<std::string> cgi;
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << "SELECT url"
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
                      ",conditionalGETInfo"
                      " FROM feeds"
                      " WHERE id=?",
            use(mID), into(mURL), into(mFolderID), into(mGuid), into(mTitle), into(mSubtitle), into(mLink), into(mDescription), into(mLanguage), into(mCopyright),
            into(mLastChecked), into(lastRefreshError), into(mSortOrder), into(cgi), now;
        mDataFetched = true;
        if (!lastRefreshError.isNull())
        {
            mLastRefreshError = lastRefreshError.value();
        }
        if (!cgi.isNull())
        {
            mConditionalGETInfo = cgi.value();
        }
    }
}

void ZapFR::Engine::FeedLocal::refresh()
{
    Log::log(LogLevel::Info, "Refreshing feed", mID);
    fetchData();

    {
        auto nowISO = Poco::DateTimeFormatter::format(Poco::DateTime(), Poco::DateTimeFormat::ISO8601_FORMAT);
        Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
        updateStmt << "UPDATE feeds SET lastRefreshError=NULL, lastChecked=? WHERE id=?", useRef(nowISO), use(mID), now;
        setLastRefreshError({});
    }

    try
    {
        FeedFetcher ff;
        const auto& parsedFeed = ff.parseURL(mURL, mID, mConditionalGETInfo);
        if (parsedFeed.has_value())
        {
            if (parsedFeed.value() == nullptr)
            {
                throw std::runtime_error("Feed type not recognized");
            }

            const auto& guid = parsedFeed.value()->guid();
            const auto& title = parsedFeed.value()->title();
            const auto& subtitle = parsedFeed.value()->subtitle();
            const auto& link = parsedFeed.value()->link();
            const auto& description = parsedFeed.value()->description();
            const auto& language = parsedFeed.value()->language();
            const auto& copyright = parsedFeed.value()->copyright();
            const auto& iconURL = parsedFeed.value()->iconURL();

            update(iconURL, guid, title, subtitle, link, description, language, copyright, ff.conditionalGETInfo());
            processItems(parsedFeed.value().get());

            refreshIcon();
            fetchUnreadCount();
        }
    }
    catch (const Poco::Exception& e)
    {
        updateAndLogLastRefreshError(e.displayText());
    }
    catch (const std::runtime_error& e)
    {
        updateAndLogLastRefreshError(e.what());
    }
    catch (...)
    {
        updateAndLogLastRefreshError("Unknown exception");
    }
}

void ZapFR::Engine::FeedLocal::processItems(FeedParser* parsedFeed)
{
    // see if we have to execute a script for each item
    std::vector<std::string> scriptsRanOnNewPost{};
    std::vector<std::string> scriptsRanOnUpdatePost{};
    auto source = SourceLocal(0); // we make a dummy source here, as the source ID isn't really referenced, nor any data within the source, just the scripts
    auto scripts = source.getScripts();
    for (const auto& script : scripts)
    {
        if (script->isEnabled() && !script->script().empty())
        {
            // check if we run this script on this feed ID
            auto runOnFeedIDs = script->runOnFeedIDs();
            if (runOnFeedIDs.has_value() && !runOnFeedIDs.value().contains(mID))
            {
                continue;
            }

            // check if we run this script on NewPost or UpdatePost
            auto scriptLocal = dynamic_cast<ScriptLocal*>(script.get());
            auto events = script->runOnEvents();
            if (events.contains(Script::Event::NewPost))
            {
                scriptsRanOnNewPost.emplace_back(scriptLocal->script());
            }
            if (events.contains(Script::Event::UpdatePost))
            {
                scriptsRanOnUpdatePost.emplace_back(scriptLocal->script());
            }
        }
    }

    for (const auto& item : parsedFeed->items())
    {
        // see if it already exists
        auto existingPost = getPostByGuid(item.guid);
        if (existingPost.has_value()) // UPDATE in case it does
        {
            dynamic_cast<PostLocal*>(existingPost.value().get())
                ->update(item.title, item.link, item.content, item.author, item.commentsURL, item.guid, item.datePublished, item.thumbnail, item.enclosures, item.categories);

            if (scriptsRanOnUpdatePost.size() > 0)
            {
                // Only trigger the update script(s) in case one of the fields is different
                auto isDifferent{false};
                // clang-format off
                if (!isDifferent && (existingPost.value()->title() != item.title)) { isDifferent = true; }
                if (!isDifferent && (existingPost.value()->link() != item.link)) { isDifferent = true; }
                if (!isDifferent && (existingPost.value()->content() != item.content)) { isDifferent = true; }
                if (!isDifferent && (existingPost.value()->author() != item.author)) { isDifferent = true; }
                if (!isDifferent && (existingPost.value()->commentsURL() != item.commentsURL)) { isDifferent = true; }
                if (!isDifferent && (existingPost.value()->datePublished() != item.datePublished)) { isDifferent = true; }
                if (!isDifferent && (existingPost.value()->thumbnail() != item.thumbnail)) { isDifferent = true; }
                // clang-format on

                // TODO:  enclosures!

                // check if categories differ
                if (!isDifferent)
                {
                    if (item.categories.size() != existingPost.value()->categories().size())
                    {
                        isDifferent = true;
                    }
                    else
                    {
                        for (const auto& newCat : item.categories)
                        {
                            if (!existingPost.value()->hasCategory(newCat))
                            {
                                isDifferent = true;
                                break;
                            }
                        }
                    }
                }

                if (isDifferent)
                {
                    auto updatedPost = getPost(existingPost.value()->id());
                    if (updatedPost.has_value())
                    {
                        for (const auto& script : scriptsRanOnUpdatePost)
                        {
                            ZapFR::Engine::ScriptLua::getInstance()->runPostScript(script, mParentSource, this, updatedPost.value().get());
                        }
                    }
                }
            }
        }
        else // INSERT in case it doesn't
        {
            auto post = PostLocal::create(mID, mTitle, item.title, item.link, item.content, item.author, item.commentsURL, item.guid, item.datePublished, item.thumbnail,
                                          item.enclosures, item.categories);

            if (scriptsRanOnNewPost.size() > 0)
            {
                for (const auto& script : scriptsRanOnNewPost)
                {
                    ZapFR::Engine::ScriptLua::getInstance()->runPostScript(script, mParentSource, this, post.get());
                }
            }
        }
    }
}

void ZapFR::Engine::FeedLocal::markAsRead(uint64_t maxPostID)
{
    if (maxPostID == std::numeric_limits<uint64_t>::max())
    {
        PostLocal::updateIsRead(true, {"posts.feedID=?"}, {use(mID, "feedID")});
    }
    else
    {
        PostLocal::updateIsRead(true, {"posts.feedID=?", "posts.id <= ?"}, {use(mID, "feedID"), use(maxPostID, "maxPostID")});
    }
}

void ZapFR::Engine::FeedLocal::refreshIcon()
{
    fetchData();

    // only check for new icons every week
    Poco::DateTime lastFetched;
    int32_t tzd;
    if (Poco::DateTimeParser::tryParse(mIconLastFetched, lastFetched, tzd))
    {
        lastFetched.makeUTC(tzd);
        Poco::DateTime now;
        auto difference = now - lastFetched;
        if (difference.totalHours() < (24 * 7))
        {
            return;
        }
    }

    std::string iconURLToQuery;
    if (mIconURL.empty())
    {
        auto link = mLink;
        // in case no link is provided in the feed details, try a favicon located on the index page of the domain that hosts the feed itself
        if (link.empty())
        {
            auto indexPage = Poco::URI(mURL);
            indexPage.setPath("/");
            link = indexPage.toString();
        }
        auto p = FavIconParser();
        p.parseURL(link, mID);
        iconURLToQuery = p.favIcon();
    }
    else
    {
        iconURLToQuery = mIconURL;
    }

    if (!iconURLToQuery.empty())
    {
        try
        {
            Poco::Net::HTTPCredentials creds; // TODO
            auto uri = Poco::URI(iconURLToQuery);
            std::string cgi;
            std::tie(mIconData, cgi) = Helpers::performHTTPRequest(uri, "GET", creds, {}, mID);
        }
        catch (...) // we ignore errors here because a missing favicon shouldn't put the feed in error state
        {
            Log::log(LogLevel::Debug, fmt::format("Failed to download feed icon: {}", iconURLToQuery), mID);
        }
    }

    if (!mIconData.empty())
    {
        auto i = iconFile(mID);
        auto fos = Poco::FileOutputStream(i.path());
        fos << mIconData;
        fos.close();

        Poco::MD5Engine md5;
        Poco::DigestOutputStream ds(md5);
        ds << mIconData;
        ds.close();
        mIconHash = Poco::DigestEngine::digestToHex(md5.digest());
    }

    // update icon last fetched time and md5 hash
    {
        Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
        updateStmt << "UPDATE feeds SET iconLastFetched=datetime('now'), iconHash=? WHERE id=?", useRef(mIconHash), use(mID), now;
        updateStmt.execute();
    }
}

void ZapFR::Engine::FeedLocal::removeIcon()
{
    auto i = iconFile(mID);
    if (i.exists())
    {
        i.remove();
    }
}

void ZapFR::Engine::FeedLocal::setIconDir(const std::string& iconDir)
{
    msIconDir = iconDir;
    auto d = Poco::File(msIconDir);
    if (!d.exists())
    {
        d.createDirectories();
    }
}

Poco::File ZapFR::Engine::FeedLocal::iconFile(uint64_t feedID)
{
    if (msIconDir.empty())
    {
        return {};
    }

    return Poco::File(msIconDir + Poco::Path::separator() + "feed" + std::to_string(feedID) + ".icon");
}

std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Log>>> ZapFR::Engine::FeedLocal::getLogs(uint64_t perPage, uint64_t page)
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindingsLogs;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindingsLogCount;

    whereClause.emplace_back("logs.feedID=?");
    bindingsLogs.emplace_back(use(mID, "feedID"));
    bindingsLogCount.emplace_back(use(mID, "feedID"));

    auto offset = perPage * (page - 1);
    bindingsLogs.emplace_back(use(perPage, "perPage"));
    bindingsLogs.emplace_back(use(offset, "offset"));

    auto logs = Log::queryMultiple(whereClause, "ORDER BY logs.id DESC", "LIMIT ? OFFSET ?", bindingsLogs);
    auto logCount = Log::queryCount(whereClause, bindingsLogCount);
    return std::make_tuple(logCount, std::move(logs));
}

void ZapFR::Engine::FeedLocal::clearLogs()
{
    Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
    deleteStmt << "DELETE FROM logs WHERE feedID=?", use(mID), now;
}

std::vector<std::unique_ptr<ZapFR::Engine::Category>> ZapFR::Engine::FeedLocal::getCategories()
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;

    whereClause.emplace_back("categories.feedID=?");
    bindings.emplace_back(use(mID, "feedID"));
    return Category::queryMultiple(false, whereClause, "ORDER BY categories.title ASC", "", bindings);
}

void ZapFR::Engine::FeedLocal::fetchStatistics()
{
    mStatistics.clear();

    // total post count
    {
        uint64_t totalPostCount{0};
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << "SELECT COUNT(*) FROM posts WHERE feedID=?", into(totalPostCount), use(mID), now;
        mStatistics[Statistic::PostCount] = std::to_string(totalPostCount);
    }

    // total flagged post count
    {
        uint64_t totalFlaggedPostCount{0};
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << "SELECT COUNT(*)"
                      " FROM flags"
                      " LEFT JOIN posts ON posts.id = flags.postID"
                      " WHERE posts.feedID=?",
            into(totalFlaggedPostCount), use(mID), now;
        mStatistics[Statistic::FlaggedPostCount] = std::to_string(totalFlaggedPostCount);
    }

    // oldest post
    {
        Poco::Nullable<std::string> oldestPost{};
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << "SELECT MIN(datePublished) FROM posts WHERE feedID=?", into(oldestPost), use(mID), now;
        mStatistics[Statistic::OldestPost] = oldestPost.isNull() ? "" : oldestPost.value();
    }

    // newest post
    {
        Poco::Nullable<std::string> newestPost{};
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << "SELECT MAX(datePublished) FROM posts WHERE feedID=?", into(newestPost), use(mID), now;
        mStatistics[Statistic::NewestPost] = newestPost.isNull() ? "" : newestPost.value();
    }
}

void ZapFR::Engine::FeedLocal::fetchThumbnailData()
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;

    whereClause.emplace_back("posts.feedID=?");
    whereClause.emplace_back("posts.isRead=FALSE");
    whereClause.emplace_back("posts.thumbnail NOT NULL");

    bindings.emplace_back(use(mID, "feedID"));

    auto posts = PostLocal::queryMultiple(whereClause, "ORDER BY posts.datePublished DESC", "LIMIT 250", bindings);
    if (posts.size() > 0)
    {
        mThumbnailData.clear();
        fetchData();

        ThumbnailData td;
        td.feedID = mID;
        td.feedTitle = mTitle;
        td.feedLink = mLink;
        for (const auto& post : posts)
        {
            td.posts.emplace_back(post->id(), post->title(), post->thumbnail(), post->link());
        }
        mThumbnailData.emplace_back(td);
    }
}

uint64_t ZapFR::Engine::FeedLocal::nextSortOrder(uint64_t folderID)
{
    uint64_t sortOrder{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT MAX(sortOrder) FROM feeds WHERE folder=?", into(sortOrder), use(folderID), now;
    return sortOrder + 10;
}

std::unique_ptr<ZapFR::Engine::FeedLocal> ZapFR::Engine::FeedLocal::create(Source* parentSource, const std::string& url, const std::string& title, uint64_t parentFolderID)
{
    auto sortOrder = nextSortOrder(parentFolderID);
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
        useRef(url), useRef(url), use(parentFolderID), use(sortOrder), useRef(nowDate);
    const std::lock_guard<std::mutex> lock(msCreateFeedMutex);
    insertStmt.execute();

    uint64_t feedID{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT last_insert_rowid()", into(feedID), now;

    auto f = std::make_unique<FeedLocal>(feedID, parentSource);
    f->setURL(url);
    f->setFolder(parentFolderID);
    f->setTitle(title);
    f->setSortOrder(sortOrder);
    f->setLastChecked(nowDate);
    f->setDataFetched(true);

    return f;
}

void ZapFR::Engine::FeedLocal::fetchUnreadCount()
{
    uint64_t unreadCount{0};
    Poco::Data::Statement selectUnreadStmt(*(Database::getInstance()->session()));
    selectUnreadStmt << "SELECT COUNT(*) FROM posts WHERE feedID=? AND isRead=FALSE", use(mID), into(unreadCount), now;
    setUnreadCount(unreadCount);
}

void ZapFR::Engine::FeedLocal::updateAndLogLastRefreshError(const std::string& error)
{
    Log::log(LogLevel::Error, error, mID);
    Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
    updateStmt << "UPDATE feeds SET lastRefreshError=? WHERE id=?", useRef(error), use(mID), now;
    setLastRefreshError(error);
}

void ZapFR::Engine::FeedLocal::update(const std::string& iconURL, const std::string& guid, const std::string& title, const std::string& subtitle, const std::string& link,
                                      const std::string& description, const std::string& language, const std::string& copyright, const std::string& conditionalGETInfo)
{
    Poco::Nullable<std::string> pocoCGI;
    if (!conditionalGETInfo.empty())
    {
        pocoCGI = conditionalGETInfo;
    }

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
                  ",conditionalGETInfo=?"
                  " WHERE id=?",
        useRef(iconURL), useRef(guid), useRef(title), useRef(subtitle), useRef(link), useRef(description), useRef(language), useRef(copyright), useRef(pocoCGI), use(mID), now;

    setIconURL(iconURL);
    setGuid(guid);
    setTitle(title);
    setSubtitle(subtitle);
    setLink(link);
    setDescription(description);
    setLanguage(language);
    setCopyright(copyright);
    setConditionalGETInfo(conditionalGETInfo);
}

void ZapFR::Engine::FeedLocal::updateProperties(const std::string& feedURL, std::optional<uint64_t> refreshIntervalInSeconds)
{
    Poco::Nullable<uint64_t> ri;
    if (refreshIntervalInSeconds.has_value())
    {
        ri = refreshIntervalInSeconds.value();
    }

    Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
    updateStmt << "UPDATE feeds SET "
                  " url=?"
                  ",refreshInterval=?"
                  " WHERE id=?",
        useRef(feedURL), use(ri), use(mID), now;

    setURL(feedURL);
    setRefreshInterval(refreshIntervalInSeconds);
}

std::unordered_map<uint64_t, uint64_t> ZapFR::Engine::FeedLocal::move(uint64_t feedID, uint64_t newFolder, uint64_t newSortOrder)
{
    uint64_t oldFolder{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT folder FROM feeds WHERE id=?", use(feedID), into(oldFolder), now;

    Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
    updateStmt << "UPDATE feeds SET folder=?, sortOrder=? WHERE id=?", use(newFolder), use(newSortOrder), use(feedID), now;

    std::vector<uint64_t> parentFoldersToQuery{newFolder};
    resort(newFolder);
    if (newFolder != oldFolder) // check in case we are moving within the same folder
    {
        parentFoldersToQuery.emplace_back(oldFolder);
        resort(oldFolder);
    }

    std::unordered_map<uint64_t, uint64_t> affectedFeedIDs;
    auto parentFolderIDs = Helpers::joinIDNumbers(parentFoldersToQuery, ",");
    uint64_t affectedFeedID{0};
    uint64_t affectedSortOrder{0};
    Poco::Data::Statement affectedSelectStmt(*(Database::getInstance()->session()));
    affectedSelectStmt << Poco::format("SELECT id,sortOrder FROM feeds WHERE folder IN (%s)", parentFolderIDs), into(affectedFeedID), into(affectedSortOrder), range(0, 1);
    while (!affectedSelectStmt.done())
    {
        if (affectedSelectStmt.execute() > 0)
        {
            affectedFeedIDs[affectedFeedID] = affectedSortOrder;
        }
    }

    return affectedFeedIDs;
}

void ZapFR::Engine::FeedLocal::resort(uint64_t folder)
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

void ZapFR::Engine::FeedLocal::remove(Source* parentSource, uint64_t feedID)
{
    auto feed = querySingle(parentSource, {"feeds.id=?"}, {use(feedID, "id")}, Source::FetchInfo::Data);
    if (feed.has_value())
    {
        auto localFeed = dynamic_cast<FeedLocal*>(feed.value().get());
        auto folder = feed.value()->folder();
        localFeed->removeIcon();

        {
            Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
            deleteStmt << "DELETE FROM feeds WHERE id=?", use(feedID), now;
        }

        {
            Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
            deleteStmt << "DELETE FROM post_enclosures WHERE postID IN (SELECT id FROM posts WHERE feedID=?)", use(feedID), now;
        }

        {
            Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
            deleteStmt << "DELETE FROM posts WHERE feedID=?", use(feedID), now;
        }
        // TODO: remove all scripts->runOnFeedIDs

        resort(folder);
    }
}

std::vector<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::FeedLocal::queryMultiple(Source* parentSource, const std::vector<std::string>& whereClause,
                                                                                          const std::string& orderClause, const std::string& limitClause,
                                                                                          const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings, uint32_t fetchInfo)
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
    Poco::Nullable<uint64_t> refreshInterval;
    uint64_t sortOrder;
    Poco::Nullable<std::string> conditionalGETInfo;

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT feeds.id"
          ",feeds.url"
          ",feeds.iconURL"
          ",feeds.iconHash"
          ",feeds.iconLastFetched"
          ",feeds.folder"
          ",feeds.guid"
          ",feeds.title"
          ",feeds.subtitle"
          ",feeds.link"
          ",feeds.description"
          ",feeds.language"
          ",feeds.copyright"
          ",feeds.lastChecked"
          ",feeds.lastRefreshError"
          ",feeds.refreshInterval"
          ",feeds.sortOrder"
          ",feeds.conditionalGETInfo"
          " FROM feeds";
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
    selectStmt.addExtract(into(url));
    selectStmt.addExtract(into(iconURL));
    selectStmt.addExtract(into(iconHash));
    selectStmt.addExtract(into(iconLastFetched));
    selectStmt.addExtract(into(folder));
    selectStmt.addExtract(into(guid));
    selectStmt.addExtract(into(title));
    selectStmt.addExtract(into(subtitle));
    selectStmt.addExtract(into(link));
    selectStmt.addExtract(into(description));
    selectStmt.addExtract(into(language));
    selectStmt.addExtract(into(copyright));
    selectStmt.addExtract(into(lastChecked));
    selectStmt.addExtract(into(lastRefreshError));
    selectStmt.addExtract(into(refreshInterval));
    selectStmt.addExtract(into(sortOrder));
    selectStmt.addExtract(into(conditionalGETInfo));

    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            auto f = std::make_unique<FeedLocal>(id, parentSource);
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
                f->setLastRefreshError(lastRefreshError.value());
            }
            if (!refreshInterval.isNull())
            {
                f->setRefreshInterval(refreshInterval.value());
            }
            f->setSortOrder(sortOrder);
            if (!conditionalGETInfo.isNull())
            {
                f->setConditionalGETInfo(conditionalGETInfo.value());
            }
            f->fetchUnreadCount();
            f->setDataFetched(true);

            if ((fetchInfo & Source::FetchInfo::Icon) == Source::FetchInfo::Icon && !msIconDir.empty())
            {
                // TODO: maybe also cache the icons here locally
                auto i = iconFile(id);
                if (i.exists())
                {
                    auto fis = Poco::FileInputStream(i.path());
                    std::string iconData;
                    Poco::StreamCopier::copyToString(fis, iconData);
                    fis.close();
                    f->setIconData(iconData);
                }
            }

            feeds.emplace_back(std::move(f));
        }
    }
    return feeds;
}

std::optional<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::FeedLocal::querySingle(Source* parentSource, const std::vector<std::string>& whereClause,
                                                                                          const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings, uint32_t fetchInfo)
{
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
    Poco::Nullable<uint64_t> refreshInterval;
    uint64_t sortOrder;
    Poco::Nullable<std::string> conditionalGETInfo;

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT feeds.id"
          ",feeds.url"
          ",feeds.iconURL"
          ",feeds.iconHash"
          ",feeds.iconLastFetched"
          ",feeds.folder"
          ",feeds.guid"
          ",feeds.title"
          ",feeds.subtitle"
          ",feeds.link"
          ",feeds.description"
          ",feeds.language"
          ",feeds.copyright"
          ",feeds.lastChecked"
          ",feeds.lastRefreshError"
          ",feeds.refreshInterval"
          ",feeds.sortOrder"
          ",feeds.conditionalGETInfo"
          " FROM feeds";
    if (!whereClause.empty())
    {
        ss << " WHERE ";
        ss << Helpers::joinString(whereClause, " AND ");
    }

    auto sql = ss.str();
    selectStmt << sql;

    for (const auto& binding : bindings)
    {
        selectStmt.addBind(binding);
    }

    selectStmt.addExtract(into(id));
    selectStmt.addExtract(into(url));
    selectStmt.addExtract(into(iconURL));
    selectStmt.addExtract(into(iconHash));
    selectStmt.addExtract(into(iconLastFetched));
    selectStmt.addExtract(into(folder));
    selectStmt.addExtract(into(guid));
    selectStmt.addExtract(into(title));
    selectStmt.addExtract(into(subtitle));
    selectStmt.addExtract(into(link));
    selectStmt.addExtract(into(description));
    selectStmt.addExtract(into(language));
    selectStmt.addExtract(into(copyright));
    selectStmt.addExtract(into(lastChecked));
    selectStmt.addExtract(into(lastRefreshError));
    selectStmt.addExtract(into(refreshInterval));
    selectStmt.addExtract(into(sortOrder));
    selectStmt.addExtract(into(conditionalGETInfo));

    selectStmt.execute();

    auto rs = Poco::Data::RecordSet(selectStmt);
    if (rs.rowCount() == 1)
    {
        auto f = std::make_unique<FeedLocal>(id, parentSource);
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
            f->setLastRefreshError(lastRefreshError.value());
        }
        if (!refreshInterval.isNull())
        {
            f->setRefreshInterval(refreshInterval.value());
        }
        f->setSortOrder(sortOrder);
        if (!conditionalGETInfo.isNull())
        {
            f->setConditionalGETInfo(conditionalGETInfo.value());
        }
        if ((fetchInfo & Source::FetchInfo::FeedUnreadCount) == Source::FetchInfo::FeedUnreadCount)
        {
            f->fetchUnreadCount();
        }
        if ((fetchInfo & Source::FetchInfo::Statistics) == Source::FetchInfo::Statistics)
        {
            f->fetchStatistics();
        }
        f->setDataFetched(true);
        return f;
    }

    return {};
}
