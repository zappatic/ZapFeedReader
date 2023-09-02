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

#include "ZapFR/Feed.h"

const std::unordered_map<std::string, ZapFR::Engine::Feed::Statistic> ZapFR::Engine::Feed::JSONIdentifierFeedStatisticMap{
    {"postCount", Statistic::PostCount},
    {"flaggedPostCount", Statistic::FlaggedPostCount},
    {"oldestPost", Statistic::OldestPost},
    {"newestPost", Statistic::NewestPost},
};

const std::unordered_map<ZapFR::Engine::Feed::Statistic, std::string> ZapFR::Engine::Feed::FeedStatisticJSONIdentifierMap{
    {Statistic::PostCount, "postCount"},
    {Statistic::FlaggedPostCount, "flaggedPostCount"},
    {Statistic::OldestPost, "oldestPost"},
    {Statistic::NewestPost, "newestPost"},
};

ZapFR::Engine::Feed::Feed(uint64_t feedID) : mID(feedID)
{
}
