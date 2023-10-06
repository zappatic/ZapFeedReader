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

#include <Poco/Base64Decoder.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPRequest.h>

#include "ZapFR/Helpers.h"
#include "ZapFR/Log.h"
#include "ZapFR/base/Post.h"
#include "ZapFR/remote/FeedRemote.h"
#include "ZapFR/remote/PostRemote.h"
#include "ZapFR/remote/SourceRemote.h"

ZapFR::Engine::FeedRemote::FeedRemote(uint64_t id, Source* parentSource) : Feed(id, parentSource)
{
}

std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Post>>> ZapFR::Engine::FeedRemote::getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread,
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
        params[HTTPParam::Post::ParentType] = HTTPParam::Post::ParentTypeFeed;
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
            }
            SourceRemote::unserializeThumbnailData(mThumbnailData, rootObj->getArray(JSON::Post::ThumbnailData));
        }
    }

    return std::make_tuple(postCount, std::move(posts));
}

std::optional<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::FeedRemote::getPost(uint64_t postID)
{
    auto remoteSource = dynamic_cast<SourceRemote*>(mParentSource);
    auto uri = remoteSource->remoteURL();
    if (remoteSource->remoteURLIsValid())
    {
        uri.setPath(fmt::format("/post/{}", postID));
        auto creds = Poco::Net::HTTPCredentials(remoteSource->remoteLogin(), remoteSource->remotePassword());

        std::map<std::string, std::string> params;
        params[HTTPParam::Post::FeedID] = std::to_string(mID);

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
        if (!rootObj.isNull())
        {
            return PostRemote::createFromJSON(rootObj);
        }
    }
    return {};
}

void ZapFR::Engine::FeedRemote::refresh()
{
    auto remoteSource = dynamic_cast<SourceRemote*>(mParentSource);
    auto uri = remoteSource->remoteURL();
    if (remoteSource->remoteURLIsValid())
    {
        uri.setPath(fmt::format("/feed/{}/refresh", mID));
        auto creds = Poco::Net::HTTPCredentials(remoteSource->remoteLogin(), remoteSource->remotePassword());

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, {});
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
        if (!rootObj.isNull())
        {
            fromJSON(rootObj);
        }
    }
}

void ZapFR::Engine::FeedRemote::markAsRead(uint64_t maxPostID)
{
    auto remoteSource = dynamic_cast<SourceRemote*>(mParentSource);
    auto uri = remoteSource->remoteURL();
    if (remoteSource->remoteURLIsValid())
    {
        uri.setPath(fmt::format("/feed/{}/mark-as-read", mID));
        auto creds = Poco::Net::HTTPCredentials(remoteSource->remoteLogin(), remoteSource->remotePassword());

        std::map<std::string, std::string> params;
        params[HTTPParam::Post::MaxPostID] = std::to_string(maxPostID);

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
    }
}

std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Log>>> ZapFR::Engine::FeedRemote::getLogs(uint64_t perPage, uint64_t page)
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
        params[HTTPParam::Log::ParentType] = HTTPParam::Log::ParentTypeFeed;
        params[HTTPParam::Log::ParentID] = std::to_string(mID);
        params[HTTPParam::Log::PerPage] = std::to_string(perPage);
        params[HTTPParam::Log::Page] = std::to_string(page);

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
        if (!rootObj.isNull())
        {
            logCount = rootObj->getValue<uint64_t>(JSON::Log::Count);
            auto logArr = rootObj->getArray(JSON::Log::Logs);
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

void ZapFR::Engine::FeedRemote::clearLogs()
{
    auto remoteSource = dynamic_cast<SourceRemote*>(mParentSource);
    auto uri = remoteSource->remoteURL();
    if (remoteSource->remoteURLIsValid())
    {
        uri.setPath(fmt::format("/feed/{}/logs", mID));
        auto creds = Poco::Net::HTTPCredentials(remoteSource->remoteLogin(), remoteSource->remotePassword());
        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_DELETE, creds, {});
    }
}

void ZapFR::Engine::FeedRemote::updateProperties(const std::string& feedURL, std::optional<uint64_t> refreshIntervalInSeconds)
{
    auto remoteSource = dynamic_cast<SourceRemote*>(mParentSource);
    auto uri = remoteSource->remoteURL();
    if (remoteSource->remoteURLIsValid())
    {
        uri.setPath(fmt::format("/feed/{}", mID));
        auto creds = Poco::Net::HTTPCredentials(remoteSource->remoteLogin(), remoteSource->remotePassword());

        std::map<std::string, std::string> params;
        params[HTTPParam::Feed::URL] = feedURL;
        params[HTTPParam::Feed::RefreshInterval] = "";
        if (refreshIntervalInSeconds.has_value())
        {
            params[HTTPParam::Feed::RefreshInterval] = std::to_string(refreshIntervalInSeconds.value());
        }

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_PATCH, creds, params);
    }
}

void ZapFR::Engine::FeedRemote::fromJSON(const Poco::JSON::Object::Ptr o)
{
    setURL(o->getValue<std::string>(JSON::Feed::URL));
    setFolder(o->getValue<uint64_t>(JSON::Feed::Folder));
    setGuid(o->getValue<std::string>(JSON::Feed::GUID));
    setTitle(o->getValue<std::string>(JSON::Feed::Title));
    setSubtitle(o->getValue<std::string>(JSON::Feed::Subtitle));
    setLink(o->getValue<std::string>(JSON::Feed::Link));
    setDescription(o->getValue<std::string>(JSON::Feed::Description));
    setLanguage(o->getValue<std::string>(JSON::Feed::Language));
    setCopyright(o->getValue<std::string>(JSON::Feed::Copyright));
    setIconHash(o->getValue<std::string>(JSON::Feed::IconHash));

    auto lre = o->getValue<std::string>(JSON::Feed::LastRefreshError);
    if (!lre.empty())
    {
        setLastRefreshError(lre);
    }

    auto ri = o->getValue<uint64_t>(JSON::Feed::RefreshInterval);
    if (ri > 0)
    {
        setRefreshInterval(ri);
    }

    setSortOrder(o->getValue<uint64_t>(JSON::Feed::SortOrder));
    setLastChecked(o->getValue<std::string>(JSON::Feed::LastChecked));
    setUnreadCount(o->getValue<uint64_t>(JSON::Feed::UnreadCount));
    setDataFetched(true);

    if (o->has(JSON::Feed::Statistics))
    {
        std::unordered_map<Feed::Statistic, std::string> stats;
        auto statsObj = o->getObject(JSON::Feed::Statistics);
        auto statsObjNames = statsObj->getNames();
        for (size_t i = 0; i < statsObjNames.size(); ++i)
        {
            auto key = statsObjNames.at(i);
            if (JSONIdentifierFeedStatisticMap.contains(key))
            {
                stats[JSONIdentifierFeedStatisticMap.at(key)] = statsObj->getValue<std::string>(key);
            }
        }
        setStatistics(stats);
    }

    if (o->has(JSON::Feed::Icon))
    {
        std::istringstream base64stream(o->getValue<std::string>(JSON::Feed::Icon));
        Poco::Base64Decoder b64decoderstream(base64stream);
        std::string decoded(std::istreambuf_iterator<char>(b64decoderstream), {});
        setIconData(decoded);
    }
}

std::unique_ptr<ZapFR::Engine::Feed> ZapFR::Engine::FeedRemote::fromJSON(Source* parentSource, const Poco::JSON::Object::Ptr o)
{
    auto feedID = o->getValue<uint64_t>(JSON::Feed::ID);

    auto feed = std::make_unique<FeedRemote>(feedID, parentSource);
    feed->fromJSON(o);

    return feed;
}
