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
//	Assigns posts to a script folder
//	/scriptfolder/<scriptFolderID>/assign-posts (POST)
//
//	URI parameters:
//		scriptFolderID - The id of the script folder to assign the posts to - apiRequest->pathComponentAt(1)
//
//	Parameters:
//		feedsAndPostIDs (REQD) - Stringified json array: [ {feedID: x, postID: x}, {...}, ...] - apiRequest->parameter("feedsAndPostIDs")
//		assign (REQD) - Whether to assign or unassign the posts ('true' or 'false') - apiRequest->parameter("assign")
//
//	Content-Type: application/json
//	JSON output: Object
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_scriptfolder_assignposts([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto scriptFolderIDStr = apiRequest->pathComponentAt(1);
    const auto feedsAndPostIDsStr = apiRequest->parameter("feedsAndPostIDs");
    const auto assign = (apiRequest->parameter("assign") == "true");

    uint64_t scriptFolderID{0};
    Poco::NumberParser::tryParseUnsigned64(scriptFolderIDStr, scriptFolderID);

    if (scriptFolderID != 0)
    {
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
                    if (obj->has("feedID") && obj->has("postID"))
                    {
                        auto feedID = obj->getValue<uint64_t>("feedID");
                        auto postID = obj->getValue<uint64_t>("postID");
                        tuples.emplace_back(std::make_tuple(feedID, postID));
                    }
                }

                if (tuples.size() > 0)
                {
                    source.value()->assignPostsToScriptFolder(scriptFolderID, assign, tuples);
                }
            }
        }
    }

    Poco::JSON::Object o;
    o.set("success", true);

    Poco::JSON::Stringifier::stringify(o, response.send());

    return Poco::Net::HTTPResponse::HTTP_OK;
}