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
#include "ZapFR/local/FolderLocal.h"

// ::API
//
//	Retrieves a folder (subfolders are not populated)
//	/folder/<folderID> (GET)
//
//	URI parameters:
//		folderID - The id of the folder to retrieve - apiRequest->pathComponentAt(1)
//
//	Parameters:
//		getStatistics - Whether to fetch the statistics of the folder ('true' or 'false'; default false) - apiRequest->parameter("getStatistics")
//		getFeedIDs - Whether to fetch the feed IDs of the folder ('true' or 'false'; default false) - apiRequest->parameter("getFeedIDs")
//
//	Content-Type: application/json
//	JSON output: Object
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_folder_get([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto folderIDStr = apiRequest->pathComponentAt(1);
    const auto getStatistics = (apiRequest->parameter("getStatistics") == "true");
    const auto getFeedIDs = (apiRequest->parameter("getFeedIDs") == "true");

    uint64_t folderID{0};
    Poco::NumberParser::tryParseUnsigned64(folderIDStr, folderID);

    Poco::JSON::Object o;
    if (folderID != 0)
    {
        auto source = ZapFR::Engine::Source::getSource(1);
        if (source.has_value())
        {
            uint32_t fetchInfo{0};
            if (getStatistics)
            {
                fetchInfo |= ZapFR::Engine::Source::FetchInfo::Statistics;
            }
            if (getFeedIDs)
            {
                fetchInfo |= ZapFR::Engine::Source::FetchInfo::FolderFeedIDs;
            }

            auto folder = source.value()->getFolder(folderID, fetchInfo);
            if (folder.has_value())
            {
                o = folder.value()->toJSON();
            }
        }
    }

    Poco::JSON::Stringifier::stringify(o, response.send());

    return Poco::Net::HTTPResponse::HTTP_OK;
}