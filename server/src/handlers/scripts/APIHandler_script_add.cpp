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

#include "API.h"
#include "APIHandlers.h"
#include "APIRequest.h"
#include "ZapFR/base/Script.h"
#include "ZapFR/base/Source.h"

// ::API
//
//	Adds a script
//	/script (POST)
//
//	Parameters:
//		type (REQD) - The type of the script ('lua') - apiRequest->parameter("type")
//		title (REQD) - The title of the script - apiRequest->parameter("title")
//		isEnabled (REQD) - Whether the script is enabled or not ('true' or 'false') - apiRequest->parameter("isEnabled")
//		runOnEvents - A comma separated list of events the script should run on - apiRequest->parameter("runOnEvents")
//		runOnFeedIDs - A comma separated list of feedIDs the script should run for - apiRequest->parameter("runOnFeedIDs")
//		script - The script contents - apiRequest->parameter("script")
//
//	Content-Type: application/json
//	JSON output: Object
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_script_add([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto typeStr = apiRequest->parameter(ZapFR::Engine::HTTPParam::Script::Type);
    const auto title = apiRequest->parameter(ZapFR::Engine::HTTPParam::Script::Title);
    const auto isEnabled = (apiRequest->parameter(ZapFR::Engine::HTTPParam::Script::IsEnabled) == ZapFR::Engine::HTTPParam::True);
    const auto runOnEventsStr = apiRequest->parameter(ZapFR::Engine::HTTPParam::Script::RunOnEvents);
    const auto runOnFeedIDsStr = apiRequest->parameter(ZapFR::Engine::HTTPParam::Script::RunOnFeedIDs);
    const auto script = apiRequest->parameter(ZapFR::Engine::HTTPParam::Script::Script);

    if (typeStr != ZapFR::Engine::Script::msTypeLuaIdentifier)
    {
        throw std::runtime_error("Invalid script type");
    }
    auto type = ZapFR::Engine::Script::Type::Lua;

    auto events = ZapFR::Engine::Script::parseRunOnEvents(runOnEventsStr);
    std::optional<std::unordered_set<uint64_t>> feedIDs;
    if (!runOnFeedIDsStr.empty())
    {
        feedIDs = ZapFR::Engine::Script::parseRunOnFeedIDs(runOnFeedIDsStr);
    }

    auto source = ZapFR::Engine::Source::getSource(1);
    if (source.has_value())
    {
        source.value()->addScript(type, title, isEnabled, events, feedIDs, script);
    }

    Poco::JSON::Object o;
    Poco::JSON::Stringifier::stringify(o, response.send());
    return Poco::Net::HTTPResponse::HTTP_OK;
}