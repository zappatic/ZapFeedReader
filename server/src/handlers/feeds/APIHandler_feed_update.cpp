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
//	Updates the properties of a feed
//	/feed/<feedID> (PATCH)
//
//	URI parameters:
//		feedID - The id of the feed to update - apiRequest->pathComponentAt(1)
//
//	Parameters:
//		url (REQD) - The new url of the feed - apiRequest->parameter("url")
//		refreshInterval - The new refresh interval of the feed in seconds (optional, uses global default if not specified) - apiRequest->parameter("refreshInterval")
//
//	Content-Type: application/json
//	JSON output: Object
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_feed_update([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto feedIDStr = apiRequest->pathComponentAt(1);
    const auto url = apiRequest->parameter("url");
    const auto refreshIntervalStr = apiRequest->parameter("refreshInterval");

    uint64_t feedID{0};
    Poco::NumberParser::tryParseUnsigned64(feedIDStr, feedID);

    if (feedID != 0)
    {
        auto source = ZapFR::Engine::Source::getSource(1);
        if (source.has_value())
        {
            auto feed = source.value()->getFeed(feedID, ZapFR::Engine::Source::FetchInfo::None);
            if (feed.has_value())
            {

                std::optional<uint64_t> ri;
                uint64_t refreshInterval{0};
                if (!refreshIntervalStr.empty())
                {
                    Poco::NumberParser::tryParseUnsigned64(refreshIntervalStr, refreshInterval);
                    if (refreshInterval > 0)
                    {
                        ri = refreshInterval;
                    }
                }

                feed.value()->updateProperties(url, ri);
            }
        }
    }

    Poco::JSON::Object o;
    o.set("success", true);

    Poco::JSON::Stringifier::stringify(o, response.send());

    return Poco::Net::HTTPResponse::HTTP_OK;
}