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
#include "ZapFR/Post.h"
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

        try
        {
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
        catch (...)
        {
            return std::make_tuple(postCount, std::move(posts));
        }
    }

    return std::make_tuple(postCount, std::move(posts));
}

std::optional<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::FeedRemote::getPost(uint64_t /*postID*/)
{
    return {};
}

bool ZapFR::Engine::FeedRemote::refresh(const std::optional<std::string>& /*feedXML*/)
{
    return true;
}

void ZapFR::Engine::FeedRemote::markAllAsRead()
{
}

void ZapFR::Engine::FeedRemote::markAsRead(uint64_t /*postID*/)
{
}

void ZapFR::Engine::FeedRemote::markAsUnread(uint64_t /*postID*/)
{
}

void ZapFR::Engine::FeedRemote::refreshIcon()
{
}

void ZapFR::Engine::FeedRemote::removeIcon()
{
}

std::string ZapFR::Engine::FeedRemote::icon() const
{
    return "";
}

std::vector<std::unique_ptr<ZapFR::Engine::Log>> ZapFR::Engine::FeedRemote::getLogs(uint64_t /*perPage*/, uint64_t /*page*/)
{
    return {};
}

uint64_t ZapFR::Engine::FeedRemote::getTotalLogCount()
{
    return 0;
}

void ZapFR::Engine::FeedRemote::updateProperties(const std::string& /*feedURL*/, std::optional<uint64_t> /*refreshIntervalInSeconds*/)
{
}

std::unique_ptr<ZapFR::Engine::Feed> ZapFR::Engine::FeedRemote::fromJSON(Source* parentSource, const Poco::JSON::Object::Ptr o)
{
    auto feedID = o->getValue<uint64_t>(Feed::JSONIdentifierFeedID);

    auto feed = std::make_unique<FeedRemote>(feedID, parentSource);
    feed->setURL(o->getValue<std::string>(Feed::JSONIdentifierFeedURL));
    feed->setFolder(o->getValue<uint64_t>(Feed::JSONIdentifierFeedFolder));
    feed->setGuid(o->getValue<std::string>(Feed::JSONIdentifierFeedGUID));
    feed->setTitle(o->getValue<std::string>(Feed::JSONIdentifierFeedTitle));
    feed->setSubtitle(o->getValue<std::string>(Feed::JSONIdentifierFeedSubtitle));
    feed->setLink(o->getValue<std::string>(Feed::JSONIdentifierFeedLink));
    feed->setDescription(o->getValue<std::string>(Feed::JSONIdentifierFeedDescription));
    feed->setLanguage(o->getValue<std::string>(Feed::JSONIdentifierFeedLanguage));
    feed->setCopyright(o->getValue<std::string>(Feed::JSONIdentifierFeedCopyright));

    auto lre = o->getValue<std::string>(Feed::JSONIdentifierFeedLastRefreshError);
    if (!lre.empty())
    {
        feed->setLastRefreshError(lre);
    }

    auto ri = o->getValue<uint64_t>(Feed::JSONIdentifierFeedRefreshInterval);
    if (ri > 0)
    {
        feed->setRefreshInterval(ri);
    }

    feed->setSortOrder(o->getValue<uint64_t>(Feed::JSONIdentifierFeedSortOrder));
    feed->setLastChecked(o->getValue<std::string>(Feed::JSONIdentifierFeedLastChecked));
    feed->setUnreadCount(o->getValue<uint64_t>(Feed::JSONIdentifierFeedUnreadCount));
    feed->setDataFetched(true);

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
        feed->setStatistics(stats);
    }

    return feed;
}
