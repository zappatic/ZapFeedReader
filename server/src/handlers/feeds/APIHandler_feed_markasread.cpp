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
#include "ZapFR/base/Feed.h"
#include "ZapFR/base/Source.h"

// ::API
//
//	Marks all posts in the feed as read
//	/feed/<feedID>/mark-as-read (POST)
//
//	URI parameters:
//		feedID - The id of the feed to mark as read - apiRequest->pathComponentAt(1)
//
//	Parameters:
//		maxPostID (REQD) - The highest post ID to mark as read - apiRequest->parameter("maxPostID")
//
//	Content-Type: application/json
//	JSON output: Object
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_feed_markasread([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto feedIDStr = apiRequest->pathComponentAt(1);
    const auto maxPostIDStr = apiRequest->parameter(ZapFR::Engine::HTTPParam::Post::MaxPostID);

    uint64_t feedID{0};
    Poco::NumberParser::tryParseUnsigned64(feedIDStr, feedID);

    uint64_t maxPostID{0};
    Poco::NumberParser::tryParseUnsigned64(maxPostIDStr, maxPostID);

    if (feedID != 0)
    {
        auto source = ZapFR::Engine::Source::getSource(1);
        if (source.has_value())
        {
            auto feed = source.value()->getFeed(feedID, ZapFR::Engine::Source::FetchInfo::None);
            if (feed.has_value())
            {
                feed.value()->markAsRead(maxPostID);
            }
        }
    }

    Poco::JSON::Object o;
    Poco::JSON::Stringifier::stringify(o, response.send());
    return Poco::Net::HTTPResponse::HTTP_OK;
}