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
