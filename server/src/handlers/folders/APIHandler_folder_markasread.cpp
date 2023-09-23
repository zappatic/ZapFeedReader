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
#include "ZapFR/base/Folder.h"
#include "ZapFR/base/Source.h"

// ::API
//
//	Marks all posts in the folder as read, returns the affected feedIDs
//	/folder/<folderID>/mark-as-read (POST)
//
//	URI parameters:
//		folderID - The id of the folder to mark as read - apiRequest->pathComponentAt(1)
//
//	Content-Type: application/json
//	JSON output: Array
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_folder_markasread([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto folderIDStr = apiRequest->pathComponentAt(1);

    uint64_t folderID{0};
    Poco::NumberParser::tryParseUnsigned64(folderIDStr, folderID);

    Poco::JSON::Array arr;

    if (folderID != 0)
    {
        auto source = ZapFR::Engine::Source::getSource(1);
        if (source.has_value())
        {
            auto folder = source.value()->getFolder(folderID, ZapFR::Engine::Source::FetchInfo::None);
            if (folder.has_value())
            {
                auto feedIDs = folder.value()->markAsRead();
                for (const auto& feedID : feedIDs)
                {
                    arr.add(feedID);
                }
            }
        }
    }

    Poco::JSON::Stringifier::stringify(arr, response.send());

    return Poco::Net::HTTPResponse::HTTP_OK;
}