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
#include "ZapFR/base/Source.h"

// ::API
//
//	Adds a script folder
//	/scriptfolder (POST)
//
//	Parameters:
//		title (REQD) - The title of the script folder to add - apiRequest->parameter("title")
//		showTotal (REQD) - Whether to show the total number of posts - apiRequest->parameter("showTotal")
//		showUnread (REQD) - Whether to show the unread number of posts - apiRequest->parameter("showUnread")
//
//	Content-Type: application/json
//	JSON output: Object
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_scriptfolder_add([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto title = apiRequest->parameter(ZapFR::Engine::HTTPParam::ScriptFolder::Title);
    const auto showTotal = apiRequest->parameter(ZapFR::Engine::HTTPParam::ScriptFolder::ShowTotal) == ZapFR::Engine::HTTPParam::True;
    const auto showUnread = apiRequest->parameter(ZapFR::Engine::HTTPParam::ScriptFolder::ShowUnread) == ZapFR::Engine::HTTPParam::True;

    auto source = ZapFR::Engine::Source::getSource(1);
    if (source.has_value())
    {
        source.value()->addScriptFolder(title, showTotal, showUnread);
    }

    Poco::JSON::Object o;
    Poco::JSON::Stringifier::stringify(o, response.send());
    return Poco::Net::HTTPResponse::HTTP_OK;
}