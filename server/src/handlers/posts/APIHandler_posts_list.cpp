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
#include "ZapFR/Flag.h"
#include "ZapFR/base/Source.h"
#include "ZapFR/local/FeedLocal.h"
#include "ZapFR/local/FolderLocal.h"
#include "ZapFR/local/PostLocal.h"
#include "ZapFR/local/ScriptFolderLocal.h"

// ::API
//
//	Returns all the posts belonging to a feed, folder or source with various filters applied
//	/posts (GET)
//
//	Parameters:
//		parentType (REQD) - The type (source, folder, feed) to retrieve posts for - apiRequest->parameter("parentType")
//		parentID - The ID of the parent type (feedID or folderID); n/a in case of 'source' - apiRequest->parameter("parentID")
//		perPage (REQD) - The amount of records per page to retrieve - apiRequest->parameter("perPage")
//		page (REQD) - The page number to retrieve - apiRequest->parameter("page")
//		showOnlyUnread - Whether to only retrieve unread posts - 'true' or 'false' - optional (default: false) - apiRequest->parameter("showOnlyUnread")
//		searchFilter - An optional search filter to apply - apiRequest->parameter("searchFilter")
//		flagColor - The ID of a flag color to apply as a filter - apiRequest->parameter("flagColor")
//
//	Content-Type: application/json
//	JSON output: Object
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_posts_list([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto parentType = apiRequest->parameter("parentType");
    const auto parentIDStr = apiRequest->parameter("parentID");
    const auto perPageStr = apiRequest->parameter("perPage");
    const auto pageStr = apiRequest->parameter("page");
    const auto showOnlyUnread = (apiRequest->parameter("showOnlyUnread") == "true");
    const auto searchFilter = apiRequest->parameter("searchFilter");
    const auto flagColorStr = apiRequest->parameter("flagColor");

    ZapFR::Engine::FlagColor flagFilter{ZapFR::Engine::FlagColor::Gray};
    if (!flagColorStr.empty())
    {
        flagFilter = ZapFR::Engine::Flag::flagColorForName(flagColorStr);
    }

    uint64_t perPage{1000};
    uint64_t page{1};
    uint64_t parentID{0};
    Poco::NumberParser::tryParseUnsigned64(perPageStr, perPage);
    Poco::NumberParser::tryParseUnsigned64(pageStr, page);
    Poco::NumberParser::tryParseUnsigned64(parentIDStr, parentID);

    Poco::JSON::Object o;
    auto source = ZapFR::Engine::Source::getSource(1);
    if (source.has_value())
    {
        Poco::JSON::Array arr;
        uint64_t postCount{0};

        std::vector<std::unique_ptr<ZapFR::Engine::Post>> posts;
        if (parentType == "feed")
        {
            auto feed = source.value()->getFeed(parentID, ZapFR::Engine::Source::FetchInfo::None);
            if (feed.has_value())
            {
                auto t = feed.value()->getPosts(perPage, page, showOnlyUnread, searchFilter, flagFilter);
                postCount = std::get<uint64_t>(t);
                posts = std::move(std::get<std::vector<std::unique_ptr<ZapFR::Engine::Post>>>(t));
            }
        }
        else if (parentType == "source")
        {
            auto t = source.value()->getPosts(perPage, page, showOnlyUnread, searchFilter, flagFilter);
            postCount = std::get<uint64_t>(t);
            posts = std::move(std::get<std::vector<std::unique_ptr<ZapFR::Engine::Post>>>(t));
        }
        else if (parentType == "folder")
        {
            auto folder = source.value()->getFolder(parentID, ZapFR::Engine::Source::FetchInfo::None);
            if (folder.has_value())
            {
                auto t = folder.value()->getPosts(perPage, page, showOnlyUnread, searchFilter, flagFilter);
                postCount = std::get<uint64_t>(t);
                posts = std::move(std::get<std::vector<std::unique_ptr<ZapFR::Engine::Post>>>(t));
            }
        }
        else if (parentType == "scriptfolder")
        {
            auto scriptFolder = source.value()->getScriptFolder(parentID, ZapFR::Engine::Source::FetchInfo::None);
            if (scriptFolder.has_value())
            {
                auto t = scriptFolder.value()->getPosts(perPage, page, showOnlyUnread, searchFilter, flagFilter);
                postCount = std::get<uint64_t>(t);
                posts = std::move(std::get<std::vector<std::unique_ptr<ZapFR::Engine::Post>>>(t));
            }
        }
        else
        {
            throw std::runtime_error("Invalid parent type requested");
        }

        for (const auto& post : posts)
        {
            arr.add(post->toJSON());
        }
        o.set("posts", arr);
        o.set("count", postCount);
    }

    Poco::JSON::Stringifier::stringify(o, response.send());

    return Poco::Net::HTTPResponse::HTTP_OK;
}