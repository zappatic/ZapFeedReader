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
//	Sorts all the subfolders and feeds in a given folder
//	/folder/<folderID>/sort (POST)
//
//	URI parameters:
//		folderID - The id of the folder to sort - apiRequest->pathComponentAt(1)
//
//	Parameters:
//		sortMethod - The method of sorting. Currently supported: 'alphaAsc' (default) - apiRequest->parameter("sortMethod")
//
//	Content-Type: application/json
//	JSON output: Object
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_folder_sort([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto folderIDStr = apiRequest->pathComponentAt(1);
    const auto sortMethodStr = apiRequest->parameter(ZapFR::Engine::HTTPParam::Folder::SortMethod);

    uint64_t folderID{0};
    Poco::NumberParser::tryParseUnsigned64(folderIDStr, folderID);

    auto sortMethod = ZapFR::Engine::SortMethod::AlphabeticallyAscending;
    if (!sortMethodStr.empty())
    {
        if (sortMethodStr == ZapFR::Engine::HTTPParam::Folder::SortMethodAlphabeticallyAscending)
        {
            sortMethod = ZapFR::Engine::SortMethod::AlphabeticallyAscending;
        }
        else
        {
            throw std::runtime_error("Invalid sort method provided");
        }
    }

    Poco::JSON::Array folderSortOrderArr;
    Poco::JSON::Array feedSortOrderArr;

    if (folderID != 0)
    {
        auto source = ZapFR::Engine::Source::getSource(1);
        if (source.has_value())
        {
            auto folder = source.value()->getFolder(folderID, ZapFR::Engine::Source::FetchInfo::None);
            if (folder.has_value())
            {
                const auto& [folderSortOrders, feedSortOrders] = folder.value()->sort(sortMethod);

                for (const auto& [sortedFolderID, folderSortOrder] : folderSortOrders)
                {
                    Poco::JSON::Object folderObj;
                    folderObj.set(ZapFR::Engine::JSON::Folder::ID, sortedFolderID);
                    folderObj.set(ZapFR::Engine::JSON::Folder::SortOrder, folderSortOrder);
                    folderSortOrderArr.add(folderObj);
                }

                for (const auto& [feedID, feedSortOrder] : feedSortOrders)
                {
                    Poco::JSON::Object feedObj;
                    feedObj.set(ZapFR::Engine::JSON::Feed::ID, feedID);
                    feedObj.set(ZapFR::Engine::JSON::Feed::SortOrder, feedSortOrder);
                    feedSortOrderArr.add(feedObj);
                }
            }
        }
    }

    Poco::JSON::Object o;
    o.set(ZapFR::Engine::JSON::Folder::FolderSortOrders, folderSortOrderArr);
    o.set(ZapFR::Engine::JSON::Folder::FeedSortOrders, feedSortOrderArr);

    Poco::JSON::Stringifier::stringify(o, response.send());

    return Poco::Net::HTTPResponse::HTTP_OK;
}