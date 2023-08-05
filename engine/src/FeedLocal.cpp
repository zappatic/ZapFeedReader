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

#include "FeedLocal.h"
#include "Database.h"
#include "FavIconParser.h"
#include "FeedFetcher.h"
#include "Helpers.h"
#include "Log.h"
#include "Post.h"

using namespace Poco::Data::Keywords;

std::string ZapFR::Engine::FeedLocal::msIconDir{""};

ZapFR::Engine::FeedLocal::FeedLocal(uint64_t id) : Feed(id)
{
}

std::vector<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::FeedLocal::getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread)
{
    std::vector<std::unique_ptr<Post>> posts;

    auto offset = perPage * (page - 1);

    uint64_t id{0};
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

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << Poco::format("SELECT id"
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
                               " WHERE feedID=?"
                               "       %s"
                               " ORDER BY datePublished DESC"
                               " LIMIT ? OFFSET ?",
                               whereClause),
        use(mID), use(perPage), use(offset), into(id), into(isRead), into(title), into(link), into(description), into(author), into(commentsURL), into(enclosureURL),
        into(enclosureLength), into(enclosureMimeType), into(guid), into(guidIsPermalink), into(datePublished), into(sourceURL), into(sourceTitle), range(0, 1);

    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            auto p = std::make_unique<Post>(id);
            p->setIsRead(isRead);
            p->setFeedID(mID);
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

std::optional<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::FeedLocal::getPost(uint64_t postID)
{
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

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT "
                  " isRead"
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
                  " WHERE feedID=?"
                  "   AND id=?",
        use(mID), use(postID), into(isRead), into(title), into(link), into(description), into(author), into(commentsURL), into(enclosureURL), into(enclosureLength),
        into(enclosureMimeType), into(guid), into(guidIsPermalink), into(datePublished), into(sourceURL), into(sourceTitle), now;

    auto rs = Poco::Data::RecordSet(selectStmt);
    if (rs.rowCount() == 1)
    {
        auto p = std::make_unique<Post>(postID);
        p->setFeedID(mID);
        p->setIsRead(isRead);
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
        return p;
    }

    return {};
}

bool ZapFR::Engine::FeedLocal::fetchData()
{
    if (!mDataFetched)
    {
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
                      ",sortOrder"
                      " FROM feeds"
                      " WHERE id=?",
            use(mID), into(mURL), into(mFolderID), into(mGuid), into(mTitle), into(mSubtitle), into(mLink), into(mDescription), into(mLanguage), into(mCopyright),
            into(mLastChecked), into(mSortOrder), now;

        mDataFetched = true;
        auto rs = Poco::Data::RecordSet(selectStmt);
        return (rs.rowCount() == 1);
    }
    return true;
}

void ZapFR::Engine::FeedLocal::refresh(const std::optional<std::string>& feedXML)
{
    Log::log(LogLevel::Info, "Refreshing feed", mID);
    fetchData();
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
        Log::log(LogLevel::Error, e.displayText(), mID);
    }
    catch (const std::runtime_error& e)
    {
        Log::log(LogLevel::Error, e.what(), mID);
    }
    catch (...)
    {
        Log::log(LogLevel::Error, "Unknown exception", mID);
    }
}

void ZapFR::Engine::FeedLocal::processItems(FeedParser* parsedFeed)
{
    for (const auto& item : parsedFeed->items())
    {
        auto isPermaLink = item.guidIsPermalink ? 1 : 0;
        auto guid = item.guid;

        // see if it already exists
        Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
        selectStmt << "SELECT id FROM posts WHERE feedID=? AND guid=?", use(mID), useRef(guid), now;
        selectStmt.execute();
        auto rs = Poco::Data::RecordSet(selectStmt);
        if (rs.rowCount() == 1)
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
        }
        else
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
            insertStmt.execute();
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

uint64_t ZapFR::Engine::FeedLocal::getTotalPostCount(bool showOnlyUnread)
{
    std::string whereClause = showOnlyUnread ? " AND isRead=FALSE" : "";

    uint64_t postCount;
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << Poco::format("SELECT COUNT(*) FROM posts WHERE feedID=? %s", whereClause), use(mID), into(postCount), now;
    return postCount;
}

std::vector<std::unique_ptr<ZapFR::Engine::Log>> ZapFR::Engine::FeedLocal::getLogs(uint64_t perPage, uint64_t page)
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
                  " WHERE logs.feedID=?"
                  " ORDER BY logs.id DESC"
                  " LIMIT ? OFFSET ?",
        use(mID), use(perPage), use(offset), into(id), into(timestamp), into(level), into(message), into(feedID), into(feedTitle), range(0, 1);

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

uint64_t ZapFR::Engine::FeedLocal::getTotalLogCount()
{
    uint64_t logCount;
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT COUNT(*) FROM logs WHERE feedID=?", use(mID), into(logCount), now;
    return logCount;
}
