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

#include <Poco/Base64Encoder.h>

#include "ZapFR/base/Feed.h"

const std::unordered_map<std::string, ZapFR::Engine::Feed::Statistic> ZapFR::Engine::Feed::JSONIdentifierFeedStatisticMap{
    {JSON::Statistic::PostCount, Statistic::PostCount},
    {JSON::Statistic::FlaggedPostCount, Statistic::FlaggedPostCount},
    {JSON::Statistic::OldestPost, Statistic::OldestPost},
    {JSON::Statistic::NewestPost, Statistic::NewestPost},
};

const std::unordered_map<ZapFR::Engine::Feed::Statistic, std::string> ZapFR::Engine::Feed::FeedStatisticJSONIdentifierMap{
    {Statistic::PostCount, JSON::Statistic::PostCount},
    {Statistic::FlaggedPostCount, JSON::Statistic::FlaggedPostCount},
    {Statistic::OldestPost, JSON::Statistic::OldestPost},
    {Statistic::NewestPost, JSON::Statistic::NewestPost},
};

ZapFR::Engine::Feed::Feed(uint64_t feedID, Source* parentSource) : mID(feedID), mParentSource(parentSource)
{
}

Poco::JSON::Object ZapFR::Engine::Feed::toJSON() const
{
    Poco::JSON::Object o;
    o.set(JSON::Feed::ID, mID);
    o.set(JSON::Feed::URL, mURL);
    o.set(JSON::Feed::Folder, mFolderID);
    o.set(JSON::Feed::GUID, mGuid);
    o.set(JSON::Feed::Title, mTitle);
    o.set(JSON::Feed::Subtitle, mSubtitle);
    o.set(JSON::Feed::Link, mLink);
    o.set(JSON::Feed::Description, mDescription);
    o.set(JSON::Feed::Language, mLanguage);
    o.set(JSON::Feed::Copyright, mCopyright);
    o.set(JSON::Feed::LastRefreshError, mLastRefreshError.has_value() ? mLastRefreshError.value() : "");
    o.set(JSON::Feed::RefreshInterval, mRefreshInterval.has_value() ? mRefreshInterval.value() : 0);
    o.set(JSON::Feed::LastChecked, mLastChecked);
    o.set(JSON::Feed::SortOrder, mSortOrder);
    o.set(JSON::Feed::UnreadCount, mUnreadCount);
    o.set(JSON::Feed::IconHash, mIconHash);
    if (mStatistics.size() > 0)
    {
        Poco::JSON::Object statsObj;
        for (const auto& [stat, value] : mStatistics)
        {
            statsObj.set(FeedStatisticJSONIdentifierMap.at(stat), value);
        }
        o.set(JSON::Feed::Statistics, statsObj);
    }
    if (!mIconData.empty())
    {
        std::stringstream b64Stream;
        Poco::Base64Encoder encoder(b64Stream);
        encoder << mIconData;
        encoder.close();
        o.set(JSON::Feed::Icon, b64Stream.str());
    }
    return o;
}
