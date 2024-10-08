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

#include <Poco/JSON/Parser.h>

#include "API.h"
#include "APIHandlers.h"
#include "APIRequest.h"
#include "ZapFR/base/Source.h"

// ::API
//
//	Sets the read status of posts in bulk
//	/set-posts-read-status (POST)
//
//	Parameters:
//		markAsRead (REQD) - Whether to mark the posts as read or unread ('true' or 'false') - apiRequest->parameter("markAsRead")
//		feedsAndPostIDs (REQD) - Stringified json array: [ {feedID: x, postID: x}, {...}, ...] - apiRequest->parameter("feedsAndPostIDs")
//
//	Content-Type: application/json
//	JSON output: Object
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_source_setpostsreadstatus([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto feedsAndPostIDsStr = apiRequest->parameter(ZapFR::Engine::HTTPParam::Source::FeedsAndPostIDs);
    const auto markAsRead = (apiRequest->parameter(ZapFR::Engine::HTTPParam::Source::MarkAsRead) == ZapFR::Engine::HTTPParam::True);

    auto source = ZapFR::Engine::Source::getSource(1);
    if (source.has_value())
    {
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(feedsAndPostIDsStr);
        auto rootArr = root.extract<Poco::JSON::Array::Ptr>();
        if (!rootArr.isNull())
        {
            std::vector<std::tuple<uint64_t, uint64_t>> tuples;

            for (size_t i = 0; i < rootArr->size(); ++i)
            {
                auto obj = rootArr->getObject(static_cast<uint32_t>(i));
                if (obj->has(ZapFR::Engine::HTTPParam::Source::FeedID) && obj->has(ZapFR::Engine::HTTPParam::Source::PostID))
                {
                    auto feedID = obj->getValue<uint64_t>(ZapFR::Engine::HTTPParam::Source::FeedID);
                    auto postID = obj->getValue<uint64_t>(ZapFR::Engine::HTTPParam::Source::PostID);
                    tuples.emplace_back(std::make_tuple(feedID, postID));
                }
            }

            if (tuples.size() > 0)
            {
                source.value()->setPostsReadStatus(markAsRead, tuples);
            }
        }
    }

    Poco::JSON::Object o;
    Poco::JSON::Stringifier::stringify(o, response.send());
    return Poco::Net::HTTPResponse::HTTP_OK;
}