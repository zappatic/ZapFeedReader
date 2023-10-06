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
//	Import an OPML file
//	/import-opml (POST)
//
//	Parameters:
//		opml (REQD) - The OPML XML content to parse and extract feeds from - apiRequest->parameter("opml")
//		parentFolderID (REQD) - The folder ID under which to import the feeds in the OPML - apiRequest->parameter("parentFolderID")
//
//	Content-Type: application/json
//	JSON output: Array
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_source_importopml([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto opml = apiRequest->parameter(ZapFR::Engine::HTTPParam::Source::OPML);
    const auto parentFolderIDStr = apiRequest->parameter(ZapFR::Engine::HTTPParam::Source::ParentFolderID);

    uint64_t parentFolderID{0};
    Poco::NumberParser::tryParseUnsigned64(parentFolderIDStr, parentFolderID);

    Poco::JSON::Array arr;

    auto source = ZapFR::Engine::Source::getSource(1);
    if (source.has_value())
    {
        auto feedIDs = source.value()->importOPML(opml, parentFolderID);
        for (const auto& feedID : feedIDs)
        {
            if (feedID != 0)
            {
                arr.add(feedID);
            }
        }
    }

    Poco::JSON::Stringifier::stringify(arr, response.send());
    return Poco::Net::HTTPResponse::HTTP_OK;
}