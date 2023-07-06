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

#include "Database.h"
#include "Feed.h"
#include "Global.h"
#include "Source.h"

using namespace Poco::Data::Keywords;

ZapFR::Engine::Database::Database(const std::string& dbPath)
{
    Poco::Data::SQLite::Connector::registerConnector();
    mSession = std::make_unique<Poco::Data::Session>("SQLite", dbPath);

    upgrade();
}

Poco::Data::Session* ZapFR::Engine::Database::session() const noexcept
{
    return mSession.get();
}

void ZapFR::Engine::Database::upgrade()
{
    // check if we have a config table, which contains the current version of the database
    Poco::Data::Statement selectStmt(*mSession);
    uint64_t count{0};
    selectStmt << "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='config'", into(count), now;
    if (count == 0)
    {
        installDBSchemaV1();
    }
    else
    {
    }
}

void ZapFR::Engine::Database::installDBSchemaV1()
{
    // CONFIG TABLE
    {
        (*mSession) << "CREATE TABLE IF NOT EXISTS config ("
                       " key TEXT PRIMARY KEY NOT NULL"
                       ",value TEXT"
                       ")",
            now;

        Poco::Data::Statement insertStmt(*mSession);
        insertStmt << "INSERT INTO config (key, value) VALUES ('db_schema_version', 1)", now;
    }

    // FEEDS TABLE
    {
        (*mSession) << "CREATE TABLE IF NOT EXISTS feeds ("
                       " id INTEGER PRIMARY KEY"
                       ",url TEXT NOT NULL"
                       ",folderHierarchy TEXT"
                       ",guid TEXT"
                       ",title TEXT NOT NULL"
                       ",subtitle TEXT"
                       ",link TEXT"
                       ",description TEXT "
                       ",language TEXT "
                       ",copyright TEXT "
                       ",sortOrder INTEGER NOT NULL"
                       ",lastChecked INTEGER NOT NULL DEFAULT 0"
                       ")",
            now;
    }

    // POSTS TABLE
    {
        (*mSession) << "CREATE TABLE IF NOT EXISTS posts ("
                       " id INTEGER PRIMARY KEY"
                       ",feedID INTEGER NOT NULL"
                       ",isRead BOOLEAN DEFAULT FALSE"
                       ",title TEXT"
                       ",link TEXT"
                       ",description TEXT"
                       ",author TEXT"
                       ",commentsURL TEXT"
                       ",enclosureURL TEXT"
                       ",enclosureLength TEXT"
                       ",enclosureMimeType TEXT"
                       ",guid TEXT"
                       ",guidIsPermalink BOOLEAN"
                       ",datePublished INTEGER"
                       ",sourceURL TEXT"
                       ",sourceTitle TEXT"
                       ")",
            now;
    }

    // SOURCES TABLE
    {
        (*mSession) << "CREATE TABLE IF NOT EXISTS sources ("
                       " id INTEGER PRIMARY KEY"
                       ",type TEXT NOT NULL"
                       ",title TEXT"
                       ",sortOrder INTEGER NOT NULL"
                       ",configData TEXT"
                       ")",
            now;

        std::string localSourceName = "On this computer";
        std::string localType = "local";
        uint64_t localSortOrder = 10;
        Poco::Data::Statement insertStmt(*mSession);
        insertStmt << "INSERT INTO sources ("
                      "type,title,sortOrder"
                      ") VALUES (?,?,?)",
            useRef(localType), useRef(localSourceName), use(localSortOrder), now;
    }
}

void ZapFR::Engine::Database::subscribeToFeed(const FeedParser& feed)
{
    auto url = feed.url();
    auto guid = feed.guid();
    auto title = feed.title();
    auto subtitle = feed.subtitle();
    auto link = feed.link();
    auto description = feed.description();
    auto language = feed.language();
    auto copyright = feed.copyright();
    uint64_t feedID{0};

    // scope for insert mutex lock
    {
        Poco::Data::Statement insertStmt(*mSession);
        insertStmt << "INSERT INTO feeds ("
                      " url"
                      ",guid"
                      ",title"
                      ",subtitle"
                      ",link"
                      ",description"
                      ",language"
                      ",copyright"
                      ",lastChecked"
                      ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP)",
            useRef(url), useRef(guid), useRef(title), useRef(subtitle), useRef(link), useRef(description), useRef(language), useRef(copyright);
        const std::lock_guard<std::mutex> lock(mInsertMutex);
        insertStmt.execute();

        Poco::Data::Statement selectStmt(*mSession);
        selectStmt << "SELECT last_insert_rowid()", into(feedID), range(0, 1);
        selectStmt.execute();
    }

    for (const auto& item : feed.items())
    {
        auto isPermaLink = item.guidIsPermalink ? 1 : 0;
        Poco::Data::Statement insertStmt(*mSession);
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
            use(feedID), useRef(item.title), useRef(item.link), useRef(item.description), useRef(item.author), useRef(item.commentsURL), useRef(item.enclosureURL),
            useRef(item.enclosureLength), useRef(item.enclosureMimeType), useRef(item.guid), use(isPermaLink), useRef(item.datePublished), useRef(item.sourceURL),
            useRef(item.sourceTitle);
        insertStmt.execute();
    }
}

Poco::JSON::Array ZapFR::Engine::Database::getPosts(uint64_t feedID, uint64_t perPage, uint64_t page)
{
    Poco::JSON::Array arr;

    uint64_t id;
    bool isRead;
    std::string title;
    std::string link;
    std::string description;
    std::string author;
    std::string commentsURL;
    std::string enclosureURL;
    std::string enclosureLength;
    std::string enclosureMimeType;
    std::string guid;
    bool guidIsPermalink;
    uint64_t datePublished;
    std::string sourceURL;
    std::string sourceTitle;

    auto offset = perPage * (page - 1);

    Poco::Data::Statement selectStmt(*mSession);
    selectStmt << "SELECT id"
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
                  " ORDER BY datePublished DESC"
                  " LIMIT ? OFFSET ?",
        use(feedID), use(perPage), use(offset), into(id), into(id), into(isRead), into(title), into(link), into(description), into(author), into(commentsURL),
        into(enclosureURL), into(enclosureLength), into(enclosureMimeType), into(guid), into(guidIsPermalink), into(datePublished), into(sourceURL), into(sourceTitle),
        range(0, 1);

    while (!selectStmt.done())
    {
        Poco::JSON::Object o;

        selectStmt.execute();
        o.set("id", id);
        o.set("isRead", isRead);
        o.set("title", title);
        o.set("link", link);
        o.set("description", description);
        o.set("author", author);
        o.set("commentsURL", commentsURL);
        o.set("enclosureURL", enclosureURL);
        o.set("enclosureLength", enclosureLength);
        o.set("enclosureMimeType", enclosureMimeType);
        o.set("guid", guid);
        o.set("guidIsPermalink", guidIsPermalink);
        o.set("datePublished", datePublished);
        o.set("sourceURL", sourceURL);
        o.set("sourceTitle", sourceTitle);
        arr.add(o);
    }

    return arr;
}
