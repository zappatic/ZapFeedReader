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
//	Adds a folder
//	/folder (POST)
//
//	Parameters:
//		title (REQD) - The title of the folder to add - apiRequest->parameter("title")
//		parentFolderID (REQD) - The ID of the folder under which to add the new subfolder - apiRequest->parameter("parentFolderID")
//
//	Content-Type: application/json
//	JSON output: Object
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_folder_add([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto title = apiRequest->parameter(ZapFR::Engine::HTTPParam::Folder::Title);
    const auto parentFolderIDStr = apiRequest->parameter(ZapFR::Engine::HTTPParam::Folder::ParentFolderID);

    uint64_t parentFolderID{0};
    Poco::NumberParser::tryParseUnsigned64(parentFolderIDStr, parentFolderID);

    Poco::JSON::Object o;

    auto source = ZapFR::Engine::Source::getSource(1);
    if (source.has_value())
    {
        const auto& [newFolderID, newSortOrder] = source.value()->addFolder(title, parentFolderID);
        o.set(ZapFR::Engine::JSON::Folder::ID, newFolderID);
        o.set(ZapFR::Engine::JSON::Folder::SortOrder, newSortOrder);
    }

    Poco::JSON::Stringifier::stringify(o, response.send());

    return Poco::Net::HTTPResponse::HTTP_OK;
}