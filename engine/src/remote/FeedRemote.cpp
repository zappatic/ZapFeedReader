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

#include "ZapFR/remote/FeedRemote.h"
#include "ZapFR/Helpers.h"
#include "ZapFR/Log.h"
#include "ZapFR/base/Post.h"
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
        params["parentType"] = "feed";
        params["parentID"] = std::to_string(mID);
        params["perPage"] = std::to_string(perPage);
        params["page"] = std::to_string(page);
        params["showOnlyUnread"] = showOnlyUnread ? "true" : "false";
        params["searchFilter"] = searchFilter;
        params["flagColor"] = Flag::nameForFlagColor(flagColor);

        auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, params);
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
                    posts.emplace_back(std::move(PostRemote::fromJSON(postObj)));
                }
            }
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
        params["feedID"] = std::to_string(mID);

        auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
        if (!rootObj.isNull())
        {
            return PostRemote::fromJSON(rootObj);
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

        auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, {});
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
        if (!rootObj.isNull())
        {
            fromJSON(rootObj);
        }
    }
}

void ZapFR::Engine::FeedRemote::markAllAsRead()
{
    auto remoteSource = dynamic_cast<SourceRemote*>(mParentSource);
    auto uri = remoteSource->remoteURL();
    if (remoteSource->remoteURLIsValid())
    {
        uri.setPath(fmt::format("/feed/{}/mark-as-read", mID));
        auto creds = Poco::Net::HTTPCredentials(remoteSource->remoteLogin(), remoteSource->remotePassword());

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, {});
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
        params["parentType"] = "feed";
        params["parentID"] = std::to_string(mID);
        params["perPage"] = std::to_string(perPage);
        params["page"] = std::to_string(page);

        auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, params);
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
                    logs.emplace_back(std::move(Log::fromJSON(logObj)));
                }
            }
        }
    }

    return std::make_tuple(logCount, std::move(logs));
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
        params["url"] = feedURL;
        params["refreshInterval"] = "";
        if (refreshIntervalInSeconds.has_value())
        {
            params["refreshInterval"] = std::to_string(refreshIntervalInSeconds.value());
        }

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_PATCH, creds, params);
    }
}

void ZapFR::Engine::FeedRemote::fromJSON(const Poco::JSON::Object::Ptr o)
{
    setURL(o->getValue<std::string>(Feed::JSONIdentifierFeedURL));
    setFolder(o->getValue<uint64_t>(Feed::JSONIdentifierFeedFolder));
    setGuid(o->getValue<std::string>(Feed::JSONIdentifierFeedGUID));
    setTitle(o->getValue<std::string>(Feed::JSONIdentifierFeedTitle));
    setSubtitle(o->getValue<std::string>(Feed::JSONIdentifierFeedSubtitle));
    setLink(o->getValue<std::string>(Feed::JSONIdentifierFeedLink));
    setDescription(o->getValue<std::string>(Feed::JSONIdentifierFeedDescription));
    setLanguage(o->getValue<std::string>(Feed::JSONIdentifierFeedLanguage));
    setCopyright(o->getValue<std::string>(Feed::JSONIdentifierFeedCopyright));
    setIconHash(o->getValue<std::string>(Feed::JSONIdentifierFeedIconHash));

    auto lre = o->getValue<std::string>(Feed::JSONIdentifierFeedLastRefreshError);
    if (!lre.empty())
    {
        setLastRefreshError(lre);
    }

    auto ri = o->getValue<uint64_t>(Feed::JSONIdentifierFeedRefreshInterval);
    if (ri > 0)
    {
        setRefreshInterval(ri);
    }

    setSortOrder(o->getValue<uint64_t>(Feed::JSONIdentifierFeedSortOrder));
    setLastChecked(o->getValue<std::string>(Feed::JSONIdentifierFeedLastChecked));
    setUnreadCount(o->getValue<uint64_t>(Feed::JSONIdentifierFeedUnreadCount));
    setDataFetched(true);

    if (o->has(Feed::JSONIdentifierFeedStatistics))
    {
        std::unordered_map<Feed::Statistic, std::string> stats;
        auto statsObj = o->getObject(Feed::JSONIdentifierFeedStatistics);
        auto statsObjNames = statsObj->getNames();
        for (size_t i = 0; i < statsObjNames.size(); ++i)
        {
            auto key = statsObjNames.at(i);
            if (Feed::JSONIdentifierFeedStatisticMap.contains(key))
            {
                stats[Feed::JSONIdentifierFeedStatisticMap.at(key)] = statsObj->getValue<std::string>(key);
            }
        }
        setStatistics(stats);
    }

    if (o->has(Feed::JSONIdentifierFeedIcon))
    {
        std::istringstream base64stream(o->getValue<std::string>(Feed::JSONIdentifierFeedIcon));
        Poco::Base64Decoder b64decoderstream(base64stream);
        std::string decoded(std::istreambuf_iterator<char>(b64decoderstream), {});
        setIconData(decoded);
    }
}

std::unique_ptr<ZapFR::Engine::Feed> ZapFR::Engine::FeedRemote::fromJSON(Source* parentSource, const Poco::JSON::Object::Ptr o)
{
    auto feedID = o->getValue<uint64_t>(Feed::JSONIdentifierFeedID);

    auto feed = std::make_unique<FeedRemote>(feedID, parentSource);
    feed->fromJSON(o);

    return feed;
}
