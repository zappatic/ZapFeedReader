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
#include "ZapFR/Feed.h"
#include "ZapFR/Source.h"

// ::API
//
//	Adds a feed
//	/feed (POST)
//
//	Parameters:
//		url (REQD) - The url of the feed to add - apiRequest->parameter("url")
//		folder (REQD) - The ID of the folder in which to add the new feed - apiRequest->parameter("folder")
//
//	Content-Type: application/json
//	JSON output: Object
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_feed_add([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto url = apiRequest->parameter("url");
    const auto folderStr = apiRequest->parameter("folder");

    uint64_t folderID{0};
    Poco::NumberParser::tryParseUnsigned64(folderStr, folderID);

    Poco::JSON::Object o;

    auto source = ZapFR::Engine::Source::getSource(1);
    if (source.has_value())
    {
        o.set(ZapFR::Engine::Feed::JSONIdentifierFeedID, source.value()->addFeed(url, folderID));
    }

    Poco::JSON::Stringifier::stringify(o, response.send());

    return Poco::Net::HTTPResponse::HTTP_OK;
}