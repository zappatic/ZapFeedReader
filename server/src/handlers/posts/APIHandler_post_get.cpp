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
#include "ZapFR/Post.h"
#include "ZapFR/Source.h"

// ::API
//
//	Retrieves a post from a specific feed
//	/post/<postID> (GET)
//
//	URI parameters:
//		postID - The id of the post to retrieve - apiRequest->pathComponentAt(1)
//
//	Parameters:
//		feedID (REQD) - The ID of the feed the post belongs to - apiRequest->parameter("feedID")
//
//	Content-Type: application/json
//	JSON output: Object
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_post_get([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto postIDStr = apiRequest->pathComponentAt(1);
    const auto feedIDStr = apiRequest->parameter("feedID");

    uint64_t postID{0};
    Poco::NumberParser::tryParseUnsigned64(postIDStr, postID);

    uint64_t feedID{0};
    Poco::NumberParser::tryParseUnsigned64(feedIDStr, feedID);

    Poco::JSON::Object o;

    auto source = ZapFR::Engine::Source::getSource(1);
    if (source.has_value())
    {
        auto feed = source.value()->getFeed(feedID, ZapFR::Engine::Source::FetchInfo::None);
        if (feed.has_value())
        {
            auto post = feed.value()->getPost(postID);
            if (post.has_value())
            {
                o = post.value()->toJSON();
            }
        }
    }

    Poco::JSON::Stringifier::stringify(o, response.send());

    return Poco::Net::HTTPResponse::HTTP_OK;
}