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

#define FMT_HEADER_ONLY
#include <fmt/core.h>

#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/StreamCopier.h>

#include "API.h"
#include "APIHandlers.h"
#include "APIRequest.h"
#include "Daemon.h"

// ::API
//
//	Returns the /js/ folder of the webinterface
//	/web/js/* (GET)
//
//	Content-Type: text/javascript
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_webinterface_js([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    auto jsFileName = apiRequest->pathComponentAt(1);
    Poco::trimInPlace(jsFileName);
    if (jsFileName.empty())
    {
        throw std::runtime_error("No css file specified");
    }

    auto daemon = apiRequest->api()->daemon();
    auto jsFile = daemon->webinterfaceRoot().pushDirectory("js").setFileName(jsFileName);
    if (!Poco::File(jsFile.toString()).exists())
    {
        throw std::runtime_error(fmt::format("Requested js file '{}' does not exist", jsFileName));
    }

    Poco::FileInputStream fileStream(jsFile.toString());
    Poco::StreamCopier::copyStream(fileStream, response.send());

    return Poco::Net::HTTPResponse::HTTP_OK;
}