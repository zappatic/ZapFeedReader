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
#include "ZapFR/local/FeedLocal.h"

// ::API
//
//	Retrieves a feed
//	/feed/<feedID> (GET)
//
//	URI parameters:
//		feedID - The id of the feed to retrieve - apiRequest->pathComponentAt(1)
//
//	Parameters:
//		getData - Whether to fetch the full feed data from the database ('true' or 'false'; default false) - apiRequest->parameter("getData")
//		getStatistics - Whether to fetch the statistics of the feed ('true' or 'false'; default false) - apiRequest->parameter("getStatistics")
//		getUnreadCount - Whether to fetch the unread count of the feed ('true' or 'false'; default false) - apiRequest->parameter("getUnreadCount")
//
//	Content-Type: application/json
//	JSON output: Object
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_feed_get([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto feedIDStr = apiRequest->pathComponentAt(1);
    const auto getData = (apiRequest->parameter(ZapFR::Engine::HTTPParam::Feed::GetData) == ZapFR::Engine::HTTPParam::True);
    const auto getStatistics = (apiRequest->parameter(ZapFR::Engine::HTTPParam::Feed::GetStatistics) == ZapFR::Engine::HTTPParam::True);
    const auto getUnreadCount = (apiRequest->parameter(ZapFR::Engine::HTTPParam::Feed::GetUnreadCount) == ZapFR::Engine::HTTPParam::True);

    uint64_t feedID{0};
    Poco::NumberParser::tryParseUnsigned64(feedIDStr, feedID);

    Poco::JSON::Object o;
    if (feedID != 0)
    {
        auto source = ZapFR::Engine::Source::getSource(1);
        if (source.has_value())
        {
            uint32_t fetchInfo{0};
            if (getData)
            {
                fetchInfo |= ZapFR::Engine::Source::FetchInfo::Data;
            }
            if (getStatistics)
            {
                fetchInfo |= ZapFR::Engine::Source::FetchInfo::Statistics;
            }
            if (getUnreadCount)
            {
                fetchInfo |= ZapFR::Engine::Source::FetchInfo::FeedUnreadCount;
            }
            auto feed = source.value()->getFeed(feedID, fetchInfo);
            if (feed.has_value())
            {
                o = feed.value()->toJSON();
            }
        }
    }

    Poco::JSON::Stringifier::stringify(o, response.send());

    return Poco::Net::HTTPResponse::HTTP_OK;
}