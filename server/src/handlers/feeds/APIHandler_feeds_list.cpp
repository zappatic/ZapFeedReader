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
#include "ZapFR/local/FeedLocal.h"
#include "ZapFR/local/SourceLocal.h"

// ::API
//
//	Returns all the feeds within the source
//	/feeds (GET)
//
//	Content-Type: application/json
//	JSON output: Array
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_feeds_list([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    Poco::JSON::Array a;

    auto source = ZapFR::Engine::Source::getSource(1);
    if (source.has_value())
    {
        auto feeds = source.value()->getFeeds();
        for (const auto& feed : feeds)
        {
            auto localFeed = dynamic_cast<ZapFR::Engine::FeedLocal*>(feed.get());
            a.add(localFeed->toJSON());
        }
    }

    Poco::JSON::Stringifier::stringify(a, response.send());

    return Poco::Net::HTTPResponse::HTTP_OK;
}