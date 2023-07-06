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

#include "Source.h"
#include "Database.h"
#include "SourceLocal.h"

using namespace Poco::Data::Keywords;

ZapFR::Engine::Database* ZapFR::Engine::Source::msDatabase{nullptr};

void ZapFR::Engine::Source::registerDatabaseInstance(Database* db)
{
    msDatabase = db;
}

ZapFR::Engine::Database* ZapFR::Engine::Source::database() noexcept
{
    return msDatabase;
}

std::unique_ptr<ZapFR::Engine::Source> ZapFR::Engine::Source::createSourceInstance(uint64_t id, const std::string& type)
{
    if (type == "local")
    {
        return std::make_unique<SourceLocal>(id);
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

    Poco::Data::Statement selectStmt(*(msDatabase->session()));
    if (typeFilter.has_value())
    {
        selectStmt << "SELECT id"
                      ",type"
                      ",title"
                      ",sortOrder"
                      ",configData"
                      " FROM sources"
                      " WHERE type=?",
            useRef(typeFilter.value()), into(sourceID), into(sourceType), into(sourceTitle), into(sourceSortOrder), into(sourceConfigData), range(0, 1);
    }
    else
    {
        selectStmt << "SELECT id"
                      ",type"
                      ",title"
                      ",sortOrder"
                      ",configData"
                      " FROM sources",
            into(sourceID), into(sourceType), into(sourceTitle), into(sourceSortOrder), into(sourceConfigData), range(0, 1);
    }
    while (!selectStmt.done())
    {
        selectStmt.execute();

        auto s = Source::createSourceInstance(sourceID, sourceType);
        s->setTitle(sourceTitle);
        s->setSortOrder(sourceSortOrder);
        s->setConfigData(sourceConfigData);
        sourceList.emplace_back(std::move(s));
    }

    return sourceList;
}
