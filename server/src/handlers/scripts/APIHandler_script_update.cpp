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
//	Updates the properties of a script
//	/script/<scriptID> (PATCH)
//
//	URI parameters:
//		scriptID - The id of the script to update - apiRequest->pathComponentAt(1)
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

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_script_update([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto scriptIDStr = apiRequest->pathComponentAt(1);
    const auto type = apiRequest->parameter(ZapFR::Engine::HTTPParam::Script::Type);
    const auto title = apiRequest->parameter(ZapFR::Engine::HTTPParam::Script::Title);
    const auto isEnabled = (apiRequest->parameter(ZapFR::Engine::HTTPParam::Script::IsEnabled) == ZapFR::Engine::HTTPParam::True);
    const auto runOnEventsStr = apiRequest->parameter(ZapFR::Engine::HTTPParam::Script::RunOnEvents);
    const auto runOnFeedIDsStr = apiRequest->parameter(ZapFR::Engine::HTTPParam::Script::RunOnFeedIDs);
    const auto scriptContents = apiRequest->parameter(ZapFR::Engine::HTTPParam::Script::Script);

    uint64_t scriptID{0};
    Poco::NumberParser::tryParseUnsigned64(scriptIDStr, scriptID);

    if (scriptID != 0)
    {
        auto events = ZapFR::Engine::Script::parseRunOnEvents(runOnEventsStr);
        std::optional<std::unordered_set<uint64_t>> feedIDs;
        if (!runOnFeedIDsStr.empty())
        {
            feedIDs = ZapFR::Engine::Script::parseRunOnFeedIDs(runOnFeedIDsStr);
        }

        auto source = ZapFR::Engine::Source::getSource(1);
        if (source.has_value())
        {
            auto script = source.value()->getScript(scriptID, ZapFR::Engine::Source::FetchInfo::None);
            if (script.has_value())
            {
                // force lua
                script.value()->update(ZapFR::Engine::Script::Type::Lua, title, isEnabled, events, feedIDs, scriptContents);
            }
        }
    }
    Poco::JSON::Object o;
    Poco::JSON::Stringifier::stringify(o, response.send());
    return Poco::Net::HTTPResponse::HTTP_OK;
}