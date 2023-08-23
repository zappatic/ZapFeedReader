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

#include "ZapFR/FeedLocal.h"
#include "ZapFR/Database.h"
#include "ZapFR/FavIconParser.h"
#include "ZapFR/FeedFetcher.h"
#include "ZapFR/Helpers.h"
#include "ZapFR/Log.h"
#include "ZapFR/PostLocal.h"
#include "ZapFR/Script.h"
#include "ZapFR/ScriptLua.h"
#include "ZapFR/SourceLocal.h"

using namespace Poco::Data::Keywords;

std::string ZapFR::Engine::FeedLocal::msIconDir{""};
std::mutex ZapFR::Engine::FeedLocal::msInsertPostMutex{};

ZapFR::Engine::FeedLocal::FeedLocal(uint64_t id) : Feed(id)
{
}

std::vector<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::FeedLocal::getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter,
                                                                                     FlagColor flagColor)
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;
    std::string wildcardSearchFilter = "%" + searchFilter + "%";
    auto fc = Flag::idForFlagColor(flagColor);

    whereClause.emplace_back("posts.feedID = ?");
    bindings.emplace_back(use(mID, "feedID"));

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

uint64_t ZapFR::Engine::FeedLocal::getTotalPostCount(bool showOnlyUnread, const std::string& searchFilter, FlagColor flagColor)
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;
    std::string wildcardSearchFilter = "%" + searchFilter + "%";
    auto fc = Flag::idForFlagColor(flagColor);

    whereClause.emplace_back("posts.feedID = ?");
    bindings.emplace_back(use(mID, "feedID"));

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

bool ZapFR::Engine::FeedLocal::fetchData()
{
    if (!mDataFetched)
    {
        Poco::Nullable<std::string> lastRefreshError;
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
                      " FROM feeds"
                      " WHERE id=?",
            use(mID), into(mURL), into(mFolderID), into(mGuid), into(mTitle), into(mSubtitle), into(mLink), into(mDescription), into(mLanguage), into(mCopyright),
            into(mLastChecked), into(lastRefreshError), into(mSortOrder), now;

        mDataFetched = true;
        if (!lastRefreshError.isNull())
        {
            mLastRefreshError = lastRefreshError.value();
        }
        auto rs = Poco::Data::RecordSet(selectStmt);
        return (rs.rowCount() == 1);
    }
    return true;
}

void ZapFR::Engine::FeedLocal::refresh(const std::optional<std::string>& feedXML)
{
    Log::log(LogLevel::Info, "Refreshing feed", mID);
    fetchData();

    {
        Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
        updateStmt << "UPDATE feeds SET lastRefreshError=NULL WHERE id=?", use(mID), now;
    }

    try
    {
        FeedFetcher ff;
        if (feedXML.has_value())
        {
            auto parsedFeed = ff.parseString(feedXML.value(), mURL);
            processItems(parsedFeed.get());
        }
        else
        {
            auto parsedFeed = ff.parseURL(mURL, mID);
            processItems(parsedFeed.get());
        }
        refreshIcon();
    }
    catch (const Poco::Exception& e)
    {
        auto error = e.displayText();
        Log::log(LogLevel::Error, error, mID);
        Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
        updateStmt << "UPDATE feeds SET lastRefreshError=? WHERE id=?", useRef(error), use(mID), now;
    }
    catch (const std::runtime_error& e)
    {
        auto error = e.what();
        Log::log(LogLevel::Error, error, mID);
        Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
        updateStmt << "UPDATE feeds SET lastRefreshError=? WHERE id=?", useRef(error), use(mID), now;
    }
    catch (...)
    {
        Log::log(LogLevel::Error, "Unknown exception", mID);
        Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
        updateStmt << "UPDATE feeds SET lastRefreshError='Unkown exception' WHERE id=?", use(mID), now;
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
        if (script->isEnabled() && script->existsOnDisk())
        {
            // check if we run this script on this feed ID
            auto runOnFeedIDs = script->runOnFeedIDs();
            if (runOnFeedIDs.has_value() && !runOnFeedIDs.value().contains(mID))
            {
                continue;
            }

            // check if we run this script on NewPost or UpdatePost
            auto events = script->runOnEvents();
            if (events.contains(Script::Event::NewPost))
            {
                scriptsRanOnNewPost.emplace_back(script->scriptContents());
            }
            if (events.contains(Script::Event::UpdatePost))
            {
                scriptsRanOnUpdatePost.emplace_back(script->scriptContents());
            }
        }
    }

    for (const auto& item : parsedFeed->items())
    {
        auto isPermaLink = item.guidIsPermalink ? 1 : 0;
        auto guid = item.guid;

        // see if it already exists
        auto existingPost = getPostByGuid(guid);
        if (existingPost.has_value()) // UPDATE in case it does
        {
            Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
            updateStmt << "UPDATE posts SET"
                          " title=?"
                          ",link=?"
                          ",description=?"
                          ",author=?"
                          ",commentsURL=?"
                          ",enclosureURL=?"
                          ",enclosureLength=?"
                          ",enclosureMimeType=?"
                          ",guid=?"
                          ",guidIsPermalink=?"
                          ",datePublished=?"
                          ",sourceURL=?"
                          ",sourceTitle=?"
                          " WHERE feedID=? AND guid=?",
                useRef(item.title), useRef(item.link), useRef(item.description), useRef(item.author), useRef(item.commentsURL), useRef(item.enclosureURL),
                useRef(item.enclosureLength), useRef(item.enclosureMimeType), useRef(item.guid), use(isPermaLink), useRef(item.datePublished), useRef(item.sourceURL),
                useRef(item.sourceTitle), use(mID), useRef(guid);
            updateStmt.execute();

            if (scriptsRanOnUpdatePost.size() > 0)
            {
                // Only trigger the update script(s) in case one of the fields is different
                auto isDifferent{false};
                // clang-format off
                if (!isDifferent && (existingPost.value()->title() != item.title)) { isDifferent = true; }
                if (!isDifferent && (existingPost.value()->link() != item.link)) { isDifferent = true; }
                if (!isDifferent && (existingPost.value()->description() != item.description)) { isDifferent = true; }
                if (!isDifferent && (existingPost.value()->author() != item.author)) { isDifferent = true; }
                if (!isDifferent && (existingPost.value()->commentsURL() != item.commentsURL)) { isDifferent = true; }
                if (!isDifferent && (existingPost.value()->enclosureURL() != item.enclosureURL)) { isDifferent = true; }
                if (!isDifferent && (existingPost.value()->enclosureLength() != item.enclosureLength)) { isDifferent = true; }
                if (!isDifferent && (existingPost.value()->enclosureMimeType() != item.enclosureMimeType)) { isDifferent = true; }
                if (!isDifferent && (existingPost.value()->datePublished() != item.datePublished)) { isDifferent = true; }
                if (!isDifferent && (existingPost.value()->sourceURL() != item.sourceURL)) { isDifferent = true; }
                if (!isDifferent && (existingPost.value()->sourceTitle() != item.sourceTitle)) { isDifferent = true; }
                // clang-format on

                if (isDifferent)
                {
                    auto updatedPost = getPost(existingPost.value()->id());
                    if (updatedPost.has_value())
                    {
                        for (const auto& script : scriptsRanOnUpdatePost)
                        {
                            ZapFR::Engine::ScriptLua::getInstance()->runPostScript(script, updatedPost.value().get());
                        }
                    }
                }
            }
        }
        else // INSERT in case it doesn't
        {
            Poco::Data::Statement insertStmt(*(Database::getInstance()->session()));
            insertStmt << "INSERT INTO posts ("
                          " feedID"
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
                          ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                use(mID), useRef(item.title), useRef(item.link), useRef(item.description), useRef(item.author), useRef(item.commentsURL), useRef(item.enclosureURL),
                useRef(item.enclosureLength), useRef(item.enclosureMimeType), useRef(item.guid), use(isPermaLink), useRef(item.datePublished), useRef(item.sourceURL),
                useRef(item.sourceTitle);

            if (scriptsRanOnNewPost.size() > 0)
            {
                uint64_t postID{0};
                {
                    const std::lock_guard<std::mutex> lock(msInsertPostMutex);
                    insertStmt.execute();
                    Poco::Data::Statement selectInsertRowIDStmt(*(Database::getInstance()->session()));
                    selectInsertRowIDStmt << "SELECT last_insert_rowid()", into(postID), now;
                }

                auto post = getPost(postID);
                if (post.has_value())
                {
                    for (const auto& script : scriptsRanOnNewPost)
                    {
                        ZapFR::Engine::ScriptLua::getInstance()->runPostScript(script, post.value().get());
                    }
                }
            }
            else
            {
                insertStmt.execute();
            }
        }
    }
}

void ZapFR::Engine::FeedLocal::markAllAsRead()
{
    Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
    updateStmt << "UPDATE posts SET isRead=TRUE WHERE feedID=?", use(mID), now;
    updateStmt.execute();
}

void ZapFR::Engine::FeedLocal::markAsRead(uint64_t postID)
{
    Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
    updateStmt << "UPDATE posts SET isRead=TRUE WHERE feedID=? AND id=?", use(mID), use(postID), now;
    updateStmt.execute();
}

void ZapFR::Engine::FeedLocal::markAsUnread(uint64_t postID)
{
    Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
    updateStmt << "UPDATE posts SET isRead=FALSE WHERE feedID=? AND id=?", use(mID), use(postID), now;
    updateStmt.execute();
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
        auto p = FavIconParser(link, mID);
        iconURLToQuery = p.favIcon();
    }
    else
    {
        iconURLToQuery = mIconURL;
    }

    std::string iconData;
    if (!iconURLToQuery.empty())
    {
        try
        {
            iconData = Helpers::performHTTPRequest(iconURLToQuery, "GET", mID);
        }
        catch (...) // we ignore errors here because a missing favicon shouldn't put the feed in error state
        {
            Log::log(LogLevel::Debug, fmt::format("Failed to download feed icon: {}", iconURLToQuery), mID);
        }
    }

    std::string iconHash;
    if (!iconData.empty())
    {
        auto i = iconFile();
        auto fos = Poco::FileOutputStream(i.path());
        fos << iconData;
        fos.close();

        Poco::MD5Engine md5;
        Poco::DigestOutputStream ds(md5);
        ds << iconData;
        ds.close();
        iconHash = Poco::DigestEngine::digestToHex(md5.digest());
    }

    // update icon last fetched time and md5 hash
    {
        Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
        updateStmt << "UPDATE feeds SET iconLastFetched=datetime('now'), iconHash=? WHERE id=?", useRef(iconHash), use(mID), now;
        updateStmt.execute();
    }
}

void ZapFR::Engine::FeedLocal::removeIcon()
{
    auto i = iconFile();
    if (i.exists())
    {
        i.remove();
    }
}

std::string ZapFR::Engine::FeedLocal::icon() const
{
    if (msIconDir.empty())
    {
        return "";
    }

    auto i = iconFile();
    if (i.exists())
    {
        auto fis = Poco::FileInputStream(i.path());
        std::string iconData;
        Poco::StreamCopier::copyToString(fis, iconData);
        fis.close();
        return iconData;
    }

    return "";
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

Poco::File ZapFR::Engine::FeedLocal::iconFile() const
{
    if (msIconDir.empty())
    {
        return {};
    }

    return Poco::File(msIconDir + Poco::Path::separator() + "feed" + std::to_string(mID) + ".icon");
}

std::vector<std::unique_ptr<ZapFR::Engine::Log>> ZapFR::Engine::FeedLocal::getLogs(uint64_t perPage, uint64_t page)
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;

    whereClause.emplace_back("logs.feedID=?");
    bindings.emplace_back(use(mID, "feedID"));

    auto offset = perPage * (page - 1);
    bindings.emplace_back(use(perPage, "perPage"));
    bindings.emplace_back(use(offset, "offset"));

    return Log::queryMultiple(whereClause, "ORDER BY logs.id DESC", "LIMIT ? OFFSET ?", bindings);
}

uint64_t ZapFR::Engine::FeedLocal::getTotalLogCount()
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;

    whereClause.emplace_back("logs.feedID=?");
    bindings.emplace_back(use(mID, "feedID"));

    return Log::queryCount(whereClause, bindings);
}
