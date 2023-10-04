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

#define FMT_HEADER_ONLY
#include <fmt/core.h>

#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPRequest.h>

#include "ZapFR/Helpers.h"
#include "ZapFR/Log.h"
#include "ZapFR/base/Post.h"
#include "ZapFR/remote/FolderRemote.h"
#include "ZapFR/remote/PostRemote.h"
#include "ZapFR/remote/SourceRemote.h"

ZapFR::Engine::FolderRemote::FolderRemote(uint64_t id, uint64_t parentFolderID, Source* parentSource) : Folder(id, parentFolderID, parentSource)
{
}

std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Post>>> ZapFR::Engine::FolderRemote::getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                                                                                              const std::string& searchFilter, FlagColor flagColor)
{
    std::vector<std::unique_ptr<ZapFR::Engine::Post>> posts;
    uint64_t postCount{0};

    auto remoteSource = dynamic_cast<SourceRemote*>(mParentSource);
    auto uri = remoteSource->remoteURL();
    if (remoteSource->remoteURLIsValid())
    {
        uri.setPath("/posts");
        auto creds = Poco::Net::HTTPCredentials(remoteSource->remoteLogin(), remoteSource->remotePassword());

        std::map<std::string, std::string> params;
        params["parentType"] = "folder";
        params["parentID"] = std::to_string(mID);
        params["perPage"] = std::to_string(perPage);
        params["page"] = std::to_string(page);
        params["showOnlyUnread"] = showOnlyUnread ? "true" : "false";
        params["searchFilter"] = searchFilter;
        params["flagColor"] = Flag::nameForFlagColor(flagColor);

        try
        {
            const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, params);
            auto parser = Poco::JSON::Parser();
            auto root = parser.parse(json);
            auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
            if (!rootObj.isNull())
            {
                postCount = rootObj->getValue<uint64_t>("count");
                auto postArr = rootObj->getArray("posts");
                if (!postArr.isNull())
                {
                    for (size_t i = 0; i < postArr->size(); ++i)
                    {
                        auto postObj = postArr->getObject(static_cast<uint32_t>(i));
                        posts.emplace_back(PostRemote::createFromJSON(postObj));
                    }
                }
                SourceRemote::unserializeThumbnailData(mThumbnailData, rootObj->getArray("thumbnaildata"));
            }
        }
        catch (...)
        {
            return std::make_tuple(postCount, std::move(posts));
        }
    }

    return std::make_tuple(postCount, std::move(posts));
}

std::unordered_set<uint64_t> ZapFR::Engine::FolderRemote::markAsRead()
{
    std::unordered_set<uint64_t> affectedFeedIDs;
    auto remoteSource = dynamic_cast<SourceRemote*>(mParentSource);
    auto uri = remoteSource->remoteURL();
    if (remoteSource->remoteURLIsValid())
    {
        uri.setPath(fmt::format("/folder/{}/mark-as-read", mID));
        auto creds = Poco::Net::HTTPCredentials(remoteSource->remoteLogin(), remoteSource->remotePassword());

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, {});
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto rootArr = root.extract<Poco::JSON::Array::Ptr>();
        if (!rootArr.isNull())
        {
            for (size_t i = 0; i < rootArr->size(); ++i)
            {
                affectedFeedIDs.insert(rootArr->getElement<uint64_t>(static_cast<uint32_t>(i)));
            }
        }
    }
    return affectedFeedIDs;
}

std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Log>>> ZapFR::Engine::FolderRemote::getLogs(uint64_t perPage, uint64_t page)
{
    std::vector<std::unique_ptr<ZapFR::Engine::Log>> logs;
    uint64_t logCount{0};

    auto remoteSource = dynamic_cast<SourceRemote*>(mParentSource);
    auto uri = remoteSource->remoteURL();
    if (remoteSource->remoteURLIsValid())
    {
        uri.setPath("/logs");
        auto creds = Poco::Net::HTTPCredentials(remoteSource->remoteLogin(), remoteSource->remotePassword());

        std::map<std::string, std::string> params;
        params["parentType"] = "folder";
        params["parentID"] = std::to_string(mID);
        params["perPage"] = std::to_string(perPage);
        params["page"] = std::to_string(page);

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
        if (!rootObj.isNull())
        {
            logCount = rootObj->getValue<uint64_t>("count");
            auto logArr = rootObj->getArray("logs");
            if (!logArr.isNull())
            {
                for (size_t i = 0; i < logArr->size(); ++i)
                {
                    auto logObj = logArr->getObject(static_cast<uint32_t>(i));
                    logs.emplace_back(Log::fromJSON(logObj));
                }
            }
        }
    }

    return std::make_tuple(logCount, std::move(logs));
}

void ZapFR::Engine::FolderRemote::clearLogs()
{
    auto remoteSource = dynamic_cast<SourceRemote*>(mParentSource);
    auto uri = remoteSource->remoteURL();
    if (remoteSource->remoteURLIsValid())
    {
        uri.setPath(fmt::format("/folder/{}/logs", mID));
        auto creds = Poco::Net::HTTPCredentials(remoteSource->remoteLogin(), remoteSource->remotePassword());
        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_DELETE, creds, {});
    }
}

void ZapFR::Engine::FolderRemote::update(const std::string& newTitle)
{
    auto remoteSource = dynamic_cast<SourceRemote*>(mParentSource);
    auto uri = remoteSource->remoteURL();
    if (remoteSource->remoteURLIsValid())
    {
        uri.setPath(fmt::format("/folder/{}", mID));
        auto creds = Poco::Net::HTTPCredentials(remoteSource->remoteLogin(), remoteSource->remotePassword());

        std::map<std::string, std::string> params;
        params["newTitle"] = newTitle;

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_PATCH, creds, params);
    }
}

std::tuple<const std::unordered_map<uint64_t, uint64_t>, const std::unordered_map<uint64_t, uint64_t>> ZapFR::Engine::FolderRemote::sort(SortMethod sortMethod)
{
    std::unordered_map<uint64_t, uint64_t> folderSortOrders{};
    std::unordered_map<uint64_t, uint64_t> feedSortOrders{};

    auto remoteSource = dynamic_cast<SourceRemote*>(mParentSource);
    auto uri = remoteSource->remoteURL();
    if (remoteSource->remoteURLIsValid())
    {
        uri.setPath(fmt::format("/folder/{}/sort", mID));
        auto creds = Poco::Net::HTTPCredentials(remoteSource->remoteLogin(), remoteSource->remotePassword());

        std::map<std::string, std::string> params;
        switch (sortMethod)
        {
            case SortMethod::AlphabeticallyAscending:
            {
                params["sortMethod"] = "alphaAsc";
                break;
            }
        }

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
        if (!rootObj.isNull())
        {
            auto folderSortOrderArr = rootObj->getArray(JSON::Folder::FolderSortOrders);
            for (size_t i = 0; i < folderSortOrderArr->size(); ++i)
            {
                auto folderSortObj = folderSortOrderArr->getObject(static_cast<uint32_t>(i));
                auto folderID = folderSortObj->getValue<uint64_t>(JSON::Folder::ID);
                auto folderSortOrder = folderSortObj->getValue<uint64_t>(JSON::Folder::SortOrder);
                folderSortOrders[folderID] = folderSortOrder;
            }

            auto feedSortOrderArr = rootObj->getArray(JSON::Folder::FeedSortOrders);
            for (size_t i = 0; i < feedSortOrderArr->size(); ++i)
            {
                auto feedSortObj = feedSortOrderArr->getObject(static_cast<uint32_t>(i));
                auto feedID = feedSortObj->getValue<uint64_t>(JSON::Feed::ID);
                auto feedSortOrder = feedSortObj->getValue<uint64_t>(JSON::Feed::SortOrder);
                feedSortOrders[feedID] = feedSortOrder;
            }
        }
    }
    return std::make_tuple(folderSortOrders, feedSortOrders);
}

std::unique_ptr<ZapFR::Engine::Folder> ZapFR::Engine::FolderRemote::fromJSON(Source* parentSource, const Poco::JSON::Object::Ptr o)
{
    std::function<std::unique_ptr<Folder>(const Poco::JSON::Object::Ptr)> constructFolder;
    constructFolder = [&](const Poco::JSON::Object::Ptr folderObj) -> std::unique_ptr<Folder>
    {
        auto folderID = folderObj->getValue<uint64_t>(JSON::Folder::ID);
        auto folderParentID = folderObj->getValue<uint64_t>(JSON::Folder::Parent);

        auto folder = std::make_unique<FolderRemote>(folderID, folderParentID, parentSource);
        folder->setTitle(folderObj->getValue<std::string>(JSON::Folder::Title));
        folder->setSortOrder(folderObj->getValue<uint64_t>(JSON::Folder::SortOrder));
        folder->setDataFetched(true);

        if (folderObj->has(JSON::Folder::Statistics))
        {
            std::unordered_map<Folder::Statistic, std::string> stats;
            auto statsObj = o->getObject(JSON::Folder::Statistics);
            auto statsObjNames = statsObj->getNames();
            for (size_t i = 0; i < statsObjNames.size(); ++i)
            {
                auto key = statsObjNames.at(i);
                if (JSONIdentifierFolderStatisticMap.contains(key))
                {
                    stats[JSONIdentifierFolderStatisticMap.at(key)] = statsObj->getValue<std::string>(key);
                }
            }
            folder->setStatistics(stats);
        }

        if (folderObj->has(JSON::Folder::Subfolders))
        {
            auto subfolders = folderObj->getArray(JSON::Folder::Subfolders);
            for (size_t i = 0; i < subfolders->size(); ++i)
            {
                auto subfolderObj = subfolders->getObject(static_cast<uint32_t>(i));
                folder->appendSubfolder(constructFolder(subfolderObj));
            }
        }

        std::vector<uint64_t> feedIDs;
        auto feedIDArr = folderObj->getArray(JSON::Folder::FeedIDs);
        for (size_t i = 0; i < feedIDArr->size(); ++i)
        {
            auto feedID = feedIDArr->getElement<uint64_t>(static_cast<uint32_t>(i));
            feedIDs.emplace_back(feedID);
        }
        folder->setFeedIDsInFoldersAndSubfolders(feedIDs);

        return folder;
    };

    return constructFolder(o);
}
