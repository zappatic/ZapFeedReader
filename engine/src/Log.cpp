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

#include "ZapFR/Log.h"
#include "ZapFR/Database.h"
#include "ZapFR/Helpers.h"

using namespace Poco::Data::Keywords;

void ZapFR::Engine::Log::log(LogLevel level, const std::string& message, std::optional<uint64_t> feedID)
{
    // TODO: filter out logging in case the level is not high enough
    if (message.empty())
    {
        return;
    }

    Poco::Nullable<uint64_t> pocoFeedID;
    if (feedID.has_value())
    {
        pocoFeedID.assign(feedID.value());
    }

    auto nowDate = Poco::DateTimeFormatter::format(Poco::DateTime(), Poco::DateTimeFormat::ISO8601_FORMAT);

    Poco::Data::Statement insertStmt(*Database::getInstance()->session());
    insertStmt << "INSERT INTO logs ("
                  " timestamp"
                  ",level"
                  ",message"
                  ",feedID"
                  ") VALUES (?, ?, ?, ?)",
        useRef(nowDate), use(level), useRef(message), use(pocoFeedID), now;
}

std::vector<std::unique_ptr<ZapFR::Engine::Log>> ZapFR::Engine::Log::queryMultiple(const std::vector<std::string>& whereClause, const std::string& orderClause,
                                                                                   const std::string& limitClause,
                                                                                   const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings)
{
    std::vector<std::unique_ptr<Log>> logs;

    uint64_t id{0};
    std::string timestamp{""};
    uint64_t level;
    std::string message{""};
    Poco::Nullable<uint64_t> feedID{0};
    Poco::Nullable<std::string> feedTitle{};

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT logs.id"
          ",logs.timestamp"
          ",logs.level"
          ",logs.message"
          ",logs.feedID"
          ",feeds.title"
          " FROM logs"
          " LEFT JOIN feeds ON feeds.id = logs.feedID";

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
    selectStmt.addExtract(into(timestamp));
    selectStmt.addExtract(into(level));
    selectStmt.addExtract(into(message));
    selectStmt.addExtract(into(feedID));
    selectStmt.addExtract(into(feedTitle));

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

uint64_t ZapFR::Engine::Log::queryCount(const std::vector<std::string>& whereClause, const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings)
{
    uint64_t logCount;
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT COUNT(*) FROM logs";
    if (!whereClause.empty())
    {
        ss << " WHERE ";
        ss << Helpers::joinString(whereClause, " AND ");
    }

    auto sql = ss.str();

    selectStmt << sql, into(logCount);

    for (const auto& binding : bindings)
    {
        selectStmt.addBind(binding);
    }

    selectStmt.execute();
    return logCount;
}