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
#include "ZapFR/base/Post.h"
#include "ZapFR/remote/PostRemote.h"
#include "ZapFR/remote/ScriptFolderRemote.h"
#include "ZapFR/remote/SourceRemote.h"

ZapFR::Engine::ScriptFolderRemote::ScriptFolderRemote(uint64_t id, Source* parentSource) : ScriptFolder(id, parentSource)
{
}

std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Post>>> ZapFR::Engine::ScriptFolderRemote::getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread,
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
        params[HTTPParam::Post::ParentType] = HTTPParam::Post::ParentTypeScriptFolder;
        params[HTTPParam::Post::ParentID] = std::to_string(mID);
        params[HTTPParam::Post::PerPage] = std::to_string(perPage);
        params[HTTPParam::Post::Page] = std::to_string(page);
        params[HTTPParam::Post::ShowOnlyUnread] = showOnlyUnread ? HTTPParam::True : HTTPParam::False;
        params[HTTPParam::Post::SearchFilter] = searchFilter;
        params[HTTPParam::Post::FlagColor] = Flag::nameForFlagColor(flagColor);

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
        if (!rootObj.isNull())
        {
            postCount = rootObj->getValue<uint64_t>(JSON::Post::Count);
            auto postArr = rootObj->getArray(JSON::Post::Posts);
            if (!postArr.isNull())
            {
                for (size_t i = 0; i < postArr->size(); ++i)
                {
                    auto postObj = postArr->getObject(static_cast<uint32_t>(i));
                    posts.emplace_back(PostRemote::createFromJSON(postObj));
                }
                SourceRemote::unserializeThumbnailData(mThumbnailData, rootObj->getArray(JSON::Post::ThumbnailData));
            }
        }
    }

    return std::make_tuple(postCount, std::move(posts));
}

void ZapFR::Engine::ScriptFolderRemote::update(const std::string& title, bool showTotal, bool showUnread)
{
    auto remoteSource = dynamic_cast<SourceRemote*>(mParentSource);
    auto uri = remoteSource->remoteURL();
    if (remoteSource->remoteURLIsValid())
    {
        uri.setPath(fmt::format("/scriptfolder/{}", mID));
        auto creds = Poco::Net::HTTPCredentials(remoteSource->remoteLogin(), remoteSource->remotePassword());

        std::map<std::string, std::string> params;
        params[HTTPParam::ScriptFolder::Title] = title;
        params[HTTPParam::ScriptFolder::ShowTotal] = showTotal ? HTTPParam::True : HTTPParam::False;
        params[HTTPParam::ScriptFolder::ShowUnread] = showUnread ? HTTPParam::True : HTTPParam::False;

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_PATCH, creds, params);
    }
}

std::unordered_set<uint64_t> ZapFR::Engine::ScriptFolderRemote::markAsRead(uint64_t maxPostID)
{
    std::unordered_set<uint64_t> feedIDs;
    auto remoteSource = dynamic_cast<SourceRemote*>(mParentSource);
    auto uri = remoteSource->remoteURL();
    if (remoteSource->remoteURLIsValid())
    {
        uri.setPath(fmt::format("/scriptfolder/{}/mark-as-read", mID));
        auto creds = Poco::Net::HTTPCredentials(remoteSource->remoteLogin(), remoteSource->remotePassword());

        std::map<std::string, std::string> params;
        params[HTTPParam::Post::MaxPostID] = std::to_string(maxPostID);

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto rootArr = root.extract<Poco::JSON::Array::Ptr>();
        if (!rootArr.isNull())
        {
            for (size_t i = 0; i < rootArr->size(); ++i)
            {
                auto feedID = rootArr->getElement<uint64_t>(static_cast<uint32_t>(i));
                feedIDs.insert(feedID);
            }
        }
    }
    return feedIDs;
}

std::unique_ptr<ZapFR::Engine::ScriptFolder> ZapFR::Engine::ScriptFolderRemote::fromJSON(Source* parentSource, const Poco::JSON::Object::Ptr o)
{
    auto scriptFolderID = o->getValue<uint64_t>(JSON::ScriptFolder::ID);

    auto scriptFolder = std::make_unique<ScriptFolderRemote>(scriptFolderID, parentSource);
    scriptFolder->setTitle(o->getValue<std::string>(JSON::ScriptFolder::Title));
    scriptFolder->setShowTotal(o->getValue<bool>(JSON::ScriptFolder::ShowTotal));
    scriptFolder->setShowUnread(o->getValue<bool>(JSON::ScriptFolder::ShowUnread));
    scriptFolder->setTotalPostCount(o->getValue<uint64_t>(JSON::ScriptFolder::TotalPostCount));
    scriptFolder->setTotalUnreadCount(o->getValue<uint64_t>(JSON::ScriptFolder::TotalUnreadCount));

    return scriptFolder;
}