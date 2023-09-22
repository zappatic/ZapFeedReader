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
#include "DummyFeed.h"

// ::API
//
//	Retrieves the RSS 2.0 dummy feed
//	/dummy-feed/rss20 (GET)
//
//	Content-Type: application/rss+xml
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_dummyfeed_get_rss20([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
#ifdef DEBUG
    response.send() << DummyFeed::getInstance()->getRSS20();
#else
    response.send();
#endif
    return Poco::Net::HTTPResponse::HTTP_OK;
}