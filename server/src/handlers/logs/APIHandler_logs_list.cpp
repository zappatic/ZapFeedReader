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
#include "ZapFR/Log.h"
#include "ZapFR/base/Feed.h"
#include "ZapFR/base/Folder.h"
#include "ZapFR/base/Source.h"

// ::API
//
//	Returns all the logs belonging to a feed, folder or source
//	/logs (GET)
//
//	Parameters:
//		parentType (REQD) - The type (source, folder, feed) to retrieve posts for - apiRequest->parameter("parentType")
//		parentID - The ID of the parent type (feedID or folderID); n/a in case of 'source' - apiRequest->parameter("parentID")
//		perPage (REQD) - The amount of records per page to retrieve - apiRequest->parameter("perPage")
//		page (REQD) - The page number to retrieve - apiRequest->parameter("page")
//
//	Content-Type: application/json
//	JSON output: Object
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_logs_list([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto parentType = apiRequest->parameter(ZapFR::Engine::HTTPParam::Log::ParentType);
    const auto parentIDStr = apiRequest->parameter(ZapFR::Engine::HTTPParam::Log::ParentID);
    const auto perPageStr = apiRequest->parameter(ZapFR::Engine::HTTPParam::Log::PerPage);
    const auto pageStr = apiRequest->parameter(ZapFR::Engine::HTTPParam::Log::Page);

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
        uint64_t logCount{0};

        std::vector<std::unique_ptr<ZapFR::Engine::Log>> logs;
        if (parentType == ZapFR::Engine::HTTPParam::Log::ParentTypeFeed)
        {
            auto feed = source.value()->getFeed(parentID, ZapFR::Engine::Source::FetchInfo::None);
            if (feed.has_value())
            {
                auto t = feed.value()->getLogs(perPage, page);
                logCount = std::get<uint64_t>(t);
                logs = std::move(std::get<std::vector<std::unique_ptr<ZapFR::Engine::Log>>>(t));
            }
        }
        else if (parentType == ZapFR::Engine::HTTPParam::Log::ParentTypeSource)
        {
            auto t = source.value()->getLogs(perPage, page);
            logCount = std::get<uint64_t>(t);
            logs = std::move(std::get<std::vector<std::unique_ptr<ZapFR::Engine::Log>>>(t));
        }
        else if (parentType == ZapFR::Engine::HTTPParam::Log::ParentTypeFolder)
        {
            auto folder = source.value()->getFolder(parentID, ZapFR::Engine::Source::FetchInfo::None);
            if (folder.has_value())
            {
                auto t = folder.value()->getLogs(perPage, page);
                logCount = std::get<uint64_t>(t);
                logs = std::move(std::get<std::vector<std::unique_ptr<ZapFR::Engine::Log>>>(t));
            }
        }
        else
        {
            throw std::runtime_error("Invalid parent type requested");
        }

        for (const auto& log : logs)
        {
            arr.add(log->toJSON());
        }
        o.set(ZapFR::Engine::JSON::Log::Logs, arr);
        o.set(ZapFR::Engine::JSON::Log::Count, logCount);
    }

    Poco::JSON::Stringifier::stringify(o, response.send());

    return Poco::Net::HTTPResponse::HTTP_OK;
}