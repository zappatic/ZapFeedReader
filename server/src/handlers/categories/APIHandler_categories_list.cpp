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
#include "ZapFR/Global.h"
#include "ZapFR/base/Source.h"
#include "ZapFR/local/FeedLocal.h"
#include "ZapFR/local/FolderLocal.h"
#include "ZapFR/local/PostLocal.h"
#include "ZapFR/local/ScriptFolderLocal.h"

// ::API
//
//	Returns all the categories belonging to a feed, folder, source or script folder
//	/categories (GET)
//
//	Parameters:
//		parentType (REQD) - The type (source, folder, feed, scriptfolder) to retrieve categories for - apiRequest->parameter("parentType")
//		parentID - The ID of the parent type (feedID, folderID or scriptFolderID); n/a in case of 'source' - apiRequest->parameter("parentID")
//
//	Content-Type: application/json
//	JSON output: Array
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_categories_list([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto parentType = apiRequest->parameter(ZapFR::Engine::HTTPParam::Category::ParentType);
    const auto parentIDStr = apiRequest->parameter(ZapFR::Engine::HTTPParam::Category::ParentID);

    uint64_t parentID{0};
    Poco::NumberParser::tryParseUnsigned64(parentIDStr, parentID);

    Poco::JSON::Array catArr;
    auto source = ZapFR::Engine::Source::getSource(1);
    if (source.has_value())
    {
        std::vector<std::unique_ptr<ZapFR::Engine::Category>> categories;

        if (parentType == ZapFR::Engine::HTTPParam::Category::ParentTypeFeed)
        {
            auto feed = source.value()->getFeed(parentID, ZapFR::Engine::Source::FetchInfo::None);
            if (feed.has_value())
            {
                categories = feed.value()->getCategories();
            }
        }
        else if (parentType == ZapFR::Engine::HTTPParam::Category::ParentTypeSource)
        {
            categories = source.value()->getCategories();
        }
        else if (parentType == ZapFR::Engine::HTTPParam::Category::ParentTypeFolder)
        {
            auto folder = source.value()->getFolder(parentID, ZapFR::Engine::Source::FetchInfo::None);
            if (folder.has_value())
            {
                categories = folder.value()->getCategories();
            }
        }
        else if (parentType == ZapFR::Engine::HTTPParam::Category::ParentTypeScriptFolder)
        {
            auto scriptFolder = source.value()->getScriptFolder(parentID, ZapFR::Engine::Source::FetchInfo::None);
            if (scriptFolder.has_value())
            {
                categories = scriptFolder.value()->getCategories();
            }
        }
        else
        {
            throw std::runtime_error("Invalid parent type requested");
        }

        for (const auto& category : categories)
        {
            catArr.add(category->toJSON());
        }
    }

    Poco::JSON::Stringifier::stringify(catArr, response.send());
    return Poco::Net::HTTPResponse::HTTP_OK;
}