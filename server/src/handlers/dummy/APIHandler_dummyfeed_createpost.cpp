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
//	Adds a dummy post to the dummy feed
//	/dummy-feed/add-post (GET)
//
//	Content-Type: text/plain
//
// API::

// This is intentionally a GET request, instead of a POST as it should be, so that new posts can easily be made in the browser

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_dummyfeed_createpost([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
#ifdef DEBUG
    auto now = Poco::DateTime();
    auto nowFormatted = Poco::DateTimeFormatter::format(now, Poco::DateTimeFormat::ISO8601_FORMAT);

    DummyFeed::getInstance()->addEntry(fmt::format("New post on {}", nowFormatted), fmt::format("Content of new post on <b>{}</b>", nowFormatted),
                                       Poco::UUIDGenerator::defaultGenerator().createRandom().toString(), Poco::Timestamp());

    response.send() << "Post created at " << nowFormatted;
#else
    response.send();
#endif
    return Poco::Net::HTTPResponse::HTTP_OK;
}