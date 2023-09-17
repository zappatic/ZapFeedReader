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

// ::API
//
//	Retrieves a mapping of feed IDs to unread counts
//	/unread-counts (GET)
//
//	Content-Type: application/json
//	JSON output: Object
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_source_getunreadcounts([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{

    Poco::JSON::Object o;

    std::unordered_map<uint64_t, uint64_t> unreadCounts;
    auto source = ZapFR::Engine::Source::getSource(1);
    if (source.has_value())
    {
        unreadCounts = source.value()->getUnreadCounts();
    }

    for (const auto& [feedID, count] : unreadCounts)
    {
        o.set(std::to_string(feedID), count);
    }

    Poco::JSON::Stringifier::stringify(o, response.send());

    return Poco::Net::HTTPResponse::HTTP_OK;
}