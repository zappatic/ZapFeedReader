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
#include "ZapFR/Source.h"

// ::API
//
//	Moves a folder to a new subfolder and/or position within the parent folder
//	/folder/<folderID>/move (POST)
//
//	URI parameters:
//		folderID - The id of the folder to move - apiRequest->pathComponentAt(1)
//
//	Parameters:
//		sortOrder (REQD) - The new sort order of the folder (the position within the new parent folder) - apiRequest->parameter("sortOrder")
//		parentFolderID (REQD) - The (new) folder parent to put the folder in - apiRequest->parameter("parentFolderID")
//
//	Content-Type: application/json
//	JSON output: Object
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_folder_move([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto folderIDStr = apiRequest->pathComponentAt(1);
    const auto sortOrderStr = apiRequest->parameter("sortOrder");
    const auto parentFolderIDStr = apiRequest->parameter("parentFolderID");

    uint64_t folderID{0};
    uint64_t sortOrder{0};
    uint64_t parentFolderID{0};
    Poco::NumberParser::tryParseUnsigned64(folderIDStr, folderID);
    Poco::NumberParser::tryParseUnsigned64(sortOrderStr, sortOrder);
    Poco::NumberParser::tryParseUnsigned64(parentFolderIDStr, parentFolderID);

    if (folderID != 0)
    {
        auto source = ZapFR::Engine::Source::getSource(1);
        if (source.has_value())
        {
            source.value()->moveFolder(folderID, parentFolderID, sortOrder);
        }
    }

    Poco::JSON::Object o;
    o.set("success", true);

    Poco::JSON::Stringifier::stringify(o, response.send());

    return Poco::Net::HTTPResponse::HTTP_OK;
}