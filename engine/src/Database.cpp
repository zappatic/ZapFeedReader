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

ZapFR::Engine::Database* ZapFR::Engine::Database::getInstance()
{
    static Database instance{};
    return &instance;
}

void ZapFR::Engine::Database::setDatabasePath(const std::string& dbPath)
{
    Database::getInstance()->initialize(dbPath);
}

void ZapFR::Engine::Database::initialize(const std::string& dbPath)
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
                       ",iconURL TEXT"
                       ",iconHash TEXT"
                       ",iconLastFetched INTEGER NOT NULL DEFAULT 0"
                       ",folder INTEGER NOT NULL DEFAULT 0"
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

    // FOLDERS TABLE
    {
        (*mSession) << "CREATE TABLE IF NOT EXISTS folders ("
                       " id INTEGER PRIMARY KEY"
                       ",parent INTEGER NOT NULL"
                       ",sortOrder INTEGER NOT NULL"
                       ",title TEXT NOT NULL"
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

    // LOGS TABLE
    {
        (*mSession) << "CREATE TABLE IF NOT EXISTS logs ("
                       " id INTEGER PRIMARY KEY"
                       ",timestamp TEXT"
                       ",level INTEGER NOT NULL"
                       ",message TEXT NOT NULL"
                       ",feedID INTEGER"
                       ")",
            now;
    }
}
