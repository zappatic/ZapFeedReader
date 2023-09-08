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

#include "ZapFR/base/Source.h"
#include "ZapFR/Database.h"
#include "ZapFR/local/SourceLocal.h"
#include "ZapFR/remote/SourceRemote.h"

const std::unordered_map<std::string, ZapFR::Engine::Source::Statistic> ZapFR::Engine::Source::JSONIdentifierSourceStatisticMap{
    {"feedCount", Statistic::FeedCount},   {"postCount", Statistic::PostCount},   {"flaggedPostCount", Statistic::FlaggedPostCount},
    {"oldestPost", Statistic::OldestPost}, {"newestPost", Statistic::NewestPost},
};

const std::unordered_map<ZapFR::Engine::Source::Statistic, std::string> ZapFR::Engine::Source::SourceStatisticJSONIdentifierMap{
    {Statistic::FeedCount, "feedCount"},   {Statistic::PostCount, "postCount"},   {Statistic::FlaggedPostCount, "flaggedPostCount"},
    {Statistic::OldestPost, "oldestPost"}, {Statistic::NewestPost, "newestPost"},
};

using namespace Poco::Data::Keywords;

std::mutex ZapFR::Engine::Source::msCreateSourceMutex{};

std::unique_ptr<ZapFR::Engine::Source> ZapFR::Engine::Source::createSourceInstance(uint64_t id, const std::string& type)
{
    if (type == ZapFR::Engine::IdentifierLocalServer)
    {
        auto s = std::make_unique<SourceLocal>(id);
        s->setType(type);
        return s;
    }
    else if (type == ZapFR::Engine::IdentifierRemoteServer)
    {
        auto s = std::make_unique<SourceRemote>(id);
        s->setType(type);
        return s;
    }
    else
    {
        throw std::runtime_error("Unknown source type");
    }
}

std::vector<std::unique_ptr<ZapFR::Engine::Source>> ZapFR::Engine::Source::getSources(std::optional<std::string> typeFilter)
{
    std::vector<std::unique_ptr<Source>> sourceList;

    uint64_t sourceID;
    std::string sourceType;
    std::string sourceTitle;
    uint64_t sourceSortOrder;
    std::string sourceConfigData;

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    if (typeFilter.has_value())
    {
        selectStmt << "SELECT id"
                      ",type"
                      ",title"
                      ",sortOrder"
                      ",configData"
                      " FROM sources"
                      " WHERE type=?"
                      " ORDER BY sortOrder ASC",
            useRef(typeFilter.value()), into(sourceID), into(sourceType), into(sourceTitle), into(sourceSortOrder), into(sourceConfigData), range(0, 1);
    }
    else
    {
        selectStmt << "SELECT id"
                      ",type"
                      ",title"
                      ",sortOrder"
                      ",configData"
                      " FROM sources"
                      " ORDER BY sortOrder ASC",
            into(sourceID), into(sourceType), into(sourceTitle), into(sourceSortOrder), into(sourceConfigData), range(0, 1);
    }
    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            auto s = Source::createSourceInstance(sourceID, sourceType);
            s->setTitle(sourceTitle);
            s->setSortOrder(sourceSortOrder);
            s->setConfigData(sourceConfigData);
            sourceList.emplace_back(std::move(s));
        }
    }
    return sourceList;
}

std::optional<std::unique_ptr<ZapFR::Engine::Source>> ZapFR::Engine::Source::getSource(uint64_t sourceID)
{
    std::string sourceType;
    std::string sourceTitle;
    uint64_t sourceSortOrder;
    std::string sourceConfigData;

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT type"
                  ",title"
                  ",sortOrder"
                  ",configData"
                  " FROM sources"
                  " WHERE id=?",
        use(sourceID), into(sourceType), into(sourceTitle), into(sourceSortOrder), into(sourceConfigData), now;

    auto rs = Poco::Data::RecordSet(selectStmt);
    if (rs.rowCount() == 1)
    {
        auto s = Source::createSourceInstance(sourceID, sourceType);
        s->setTitle(sourceTitle);
        s->setSortOrder(sourceSortOrder);
        s->setConfigData(sourceConfigData);
        return s;
    }
    return {};
}

void ZapFR::Engine::Source::updateTitle(const std::string& newTitle)
{
    Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
    updateStmt << "UPDATE sources SET title=? WHERE id=?", useRef(newTitle), use(mID), now;
}

std::optional<std::unique_ptr<ZapFR::Engine::Source>> ZapFR::Engine::Source::create(const std::string& type, const std::string& title, const std::string& configData)
{
    auto sortOrder = nextSortOrder();
    Poco::Data::Statement insertStmt(*(Database::getInstance()->session()));
    insertStmt << "INSERT INTO sources (type,title,sortOrder,configData) VALUES (?,?,?,?)", useRef(type), useRef(title), use(sortOrder), useRef(configData);

    std::lock_guard<std::mutex> lock(msCreateSourceMutex);
    insertStmt.execute();

    uint64_t sourceID{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT last_insert_rowid()", into(sourceID), now;

    return Source::getSource(sourceID);
}

uint64_t ZapFR::Engine::Source::nextSortOrder()
{
    uint64_t sortOrder{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT MAX(sortOrder) FROM sources", into(sortOrder), now;
    return sortOrder + 10;
}

void ZapFR::Engine::Source::removeSource(uint64_t id)
{
    // double check we're not removing a local source
    auto source = getSource(id);
    if (source.has_value())
    {
        if (source.value()->type() == ZapFR::Engine::IdentifierLocalServer)
        {
            return;
        }

        Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
        deleteStmt << "DELETE FROM sources WHERE id=?", use(id), now;
    }
}
