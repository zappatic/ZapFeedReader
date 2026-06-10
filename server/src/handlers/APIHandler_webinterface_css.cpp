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
//	Returns the /web/css/ folder of the webinterface
//	/web/css/* (GET)
//
//	Content-Type: text/css
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_webinterface_css(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    auto cssFileName = apiRequest->pathComponentAt(1);
    Poco::trimInPlace(cssFileName);
    if (cssFileName.empty())
    {
        throw std::runtime_error("No css file specified");
    }

    auto daemon = apiRequest->api()->daemon();
    auto cssFile = daemon->webinterfaceRoot().pushDirectory("css").setFileName(cssFileName);
    if (!Poco::File(cssFile.toString()).exists())
    {
        throw std::runtime_error(fmt::format("Requested css file '{}' does not exist", cssFileName));
    }

    Poco::FileInputStream fileStream(cssFile.toString());
    Poco::StreamCopier::copyStream(fileStream, response.send());

    return Poco::Net::HTTPResponse::HTTP_OK;
}