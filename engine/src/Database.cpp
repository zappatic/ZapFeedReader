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

#include <Poco/Data/SQLite/Connector.h>

#include "ZapFR/Database.h"
#include "ZapFR/base/Feed.h"
#include "ZapFR/base/Source.h"

using namespace Poco::Data::Keywords;

ZapFR::Engine::Database* ZapFR::Engine::Database::getInstance()
{
    static Database instance{};
    return &instance;
}

void ZapFR::Engine::Database::initialize(const std::string& dbPath, ApplicationType appType)
{
    Poco::Data::SQLite::Connector::registerConnector();
    mSession = std::make_unique<Poco::Data::Session>("SQLite", dbPath);
    mAppType = appType;

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

    std::string currentDBVersionStr{""};
    Poco::Data::Statement selectVersionStmt(*mSession);
    selectVersionStmt << "SELECT value FROM config WHERE key='db_schema_version'", into(currentDBVersionStr), now;

    uint64_t currentDBVersion{0};
    Poco::NumberParser::tryParseUnsigned64(currentDBVersionStr, currentDBVersion);
    if (currentDBVersion > 0)
    {
        if (currentDBVersion < ZapFR::Engine::DBVersion)
        {
            static std::vector<std::function<void()>> upgradeFunctions{
                []() { /* nop, there is no db version 0 */ },    []() { /* nop, version 1 should have been installed with installDBSchemaV1 */ },
                std::bind(&Database::upgradeToDBSchemaV2, this), std::bind(&Database::upgradeToDBSchemaV3, this),
                std::bind(&Database::upgradeToDBSchemaV4, this), std::bind(&Database::upgradeToDBSchemaV5, this),
                std::bind(&Database::upgradeToDBSchemaV6, this)};

            for (auto i = currentDBVersion + 1; i <= ZapFR::Engine::DBVersion; ++i)
            {
                const auto& upgradeFunc = upgradeFunctions.at(i);
                upgradeFunc();
            }
        }
        else if (currentDBVersion > ZapFR::Engine::DBVersion)
        {
            std::cerr << "ERROR: Existing database is for a newer version of ZapFeedReader";
            throw std::runtime_error("ERROR: Existing database is for a newer version of ZapFeedReader");
        }
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
                       ",lastRefreshError TEXT"
                       ",refreshInterval INTEGER"
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
                       ",content TEXT"
                       ",author TEXT"
                       ",commentsURL TEXT"
                       ",guid TEXT"
                       ",datePublished INTEGER"
                       ")",
            now;
    }

    // POST_ENCLOSURESS TABLE
    {
        (*mSession) << "CREATE TABLE IF NOT EXISTS post_enclosures ("
                       " id INTEGER PRIMARY KEY"
                       ",postID INTEGER NOT NULL"
                       ",url TEXT NOT NULL"
                       ",size INTEGER NOT NULL DEFAULT 0"
                       ",mimetype TEXT"
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
                       ",lastError TEXT"
                       ")",
            now;

        std::string localSourceName;
        switch (mAppType)
        {
            case ApplicationType::Client:
            {
                localSourceName = "On this computer";
                break;
            }
            case ApplicationType::Server:
            {
                localSourceName = "ZapFeedReader server";
                break;
            }
        }
        std::string localType = ZapFR::Engine::ServerIdentifier::Local;
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

    // FLAGS TABLE
    {
        (*mSession) << "CREATE TABLE IF NOT EXISTS flags ("
                       " id INTEGER PRIMARY KEY"
                       ",postID INTEGER NOT NULL"
                       ",flagID INTEGER NOT NULL"
                       ")",
            now;
    }

    // SCRIPTFOLDERS TABLE
    {
        (*mSession) << "CREATE TABLE IF NOT EXISTS scriptfolders ("
                       " id INTEGER PRIMARY KEY"
                       ",title TEXT NOT NULL"
                       ",showTotal BOOLEAN NOT NULL DEFAULT TRUE"
                       ",showUnread BOOLEAN NOT NULL DEFAULT TRUE"
                       ")",
            now;
    }

    // SCRIPTFOLDER_POSTS TABLE
    {
        (*mSession) << "CREATE TABLE IF NOT EXISTS scriptfolder_posts ("
                       " id INTEGER PRIMARY KEY"
                       ",scriptfolderID INTEGER NOT NULL"
                       ",postID INTEGER NOT NULL"
                       ")",
            now;
    }

    // SCRIPTS TABLE
    {
        (*mSession) << "CREATE TABLE IF NOT EXISTS scripts ("
                       " id INTEGER PRIMARY KEY"
                       ",title TEXT NOT NULL"
                       ",type TEXT NOT NULL DEFAULT 'lua'"
                       ",isEnabled BOOLEAN DEFAULT TRUE"
                       ",runOnEvents TEXT NOT NULL DEFAULT 'newpost'"
                       ",runOnFeedIDs TEXT"
                       ",script TEXT NOT NULL DEFAULT ''"
                       ")",
            now;
    }
}

void ZapFR::Engine::Database::upgradeToDBSchemaV2()
{
    (*mSession) << "ALTER TABLE posts ADD thumbnail TEXT", now;
    (*mSession) << "UPDATE config SET VALUE='2' WHERE key='db_schema_version'", now;
}

void ZapFR::Engine::Database::upgradeToDBSchemaV3()
{
    (*mSession) << "ALTER TABLE feeds ADD conditionalGETInfo TEXT", now;
    (*mSession) << "UPDATE config SET VALUE='3' WHERE key='db_schema_version'", now;
}

void ZapFR::Engine::Database::upgradeToDBSchemaV4()
{
    (*mSession) << R"(CREATE INDEX scriptfolder_posts_IX_scriptFolderID ON scriptfolder_posts (scriptFolderID))", now;
    (*mSession) << R"(CREATE INDEX flags_IX_flagID ON flags (flagID))", now;
    (*mSession) << R"(CREATE INDEX feeds_IX_folder ON feeds (folder))", now;
    (*mSession) << R"(CREATE INDEX folders_IX_parent ON folders (parent))", now;
    (*mSession) << R"(CREATE INDEX posts_IX_feedID ON posts (feedID))", now;
    (*mSession) << R"(CREATE INDEX posts_IX_isRead ON posts (isRead))", now;
    (*mSession) << R"(CREATE INDEX post_enclosures_IX_postID ON post_enclosures (postID))", now;
    (*mSession) << R"(CREATE INDEX logs_IX_feedID ON logs (feedID))", now;
    (*mSession) << "UPDATE config SET VALUE='4' WHERE key='db_schema_version'", now;
}

void ZapFR::Engine::Database::upgradeToDBSchemaV5()
{
    (*mSession) << "CREATE TABLE IF NOT EXISTS categories ("
                   " id INTEGER PRIMARY KEY"
                   ",feedID INTEGER NOT NULL"
                   ",title TEXT NOT NULL"
                   ")",
        now;

    (*mSession) << "CREATE TABLE IF NOT EXISTS post_categories ("
                   " postID INTEGER NOT NULL"
                   ",categoryID INTEGER NOT NULL"
                   ",PRIMARY KEY (postID, categoryID)"
                   ")",
        now;

    (*mSession) << "UPDATE config SET VALUE='5' WHERE key='db_schema_version'", now;
}

void ZapFR::Engine::Database::upgradeToDBSchemaV6()
{
    (*mSession) << R"(CREATE INDEX post_categories_IX_categoryID ON post_categories (categoryID))", now;
    (*mSession) << "UPDATE config SET VALUE='6' WHERE key='db_schema_version'", now;
}