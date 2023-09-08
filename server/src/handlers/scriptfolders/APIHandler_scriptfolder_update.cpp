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
#include "ZapFR/base/ScriptFolder.h"
#include "ZapFR/base/Source.h"

// ::API
//
//	Updates the properties of a script folder
//	/scriptfolder/<scriptFolderID> (PATCH)
//
//	URI parameters:
//		scriptFolderID - The id of the script folder to update - apiRequest->pathComponentAt(1)
//
//	Parameters:
//		title (REQD) - The new title of the script folder - apiRequest->parameter("title")
//
//	Content-Type: application/json
//	JSON output: Object
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_scriptfolder_update([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto scriptFolderIDStr = apiRequest->pathComponentAt(1);
    const auto newTitle = apiRequest->parameter("title");

    uint64_t scriptFolderID{0};
    Poco::NumberParser::tryParseUnsigned64(scriptFolderIDStr, scriptFolderID);

    if (scriptFolderID != 0)
    {
        auto source = ZapFR::Engine::Source::getSource(1);
        if (source.has_value())
        {
            auto scriptFolder = source.value()->getScriptFolder(scriptFolderID, ZapFR::Engine::Source::FetchInfo::None);
            if (scriptFolder.has_value())
            {
                scriptFolder.value()->update(newTitle);
            }
        }
    }
    Poco::JSON::Object o;
    o.set("success", true);

    Poco::JSON::Stringifier::stringify(o, response.send());

    return Poco::Net::HTTPResponse::HTTP_OK;
}