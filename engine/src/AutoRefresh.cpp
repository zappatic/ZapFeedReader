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

#include "ZapFR/AutoRefresh.h"
#include "ZapFR/Agent.h"
#include "ZapFR/base/Feed.h"
#include "ZapFR/base/Source.h"

ZapFR::Engine::AutoRefresh::AutoRefresh()
{
    mTimer = std::make_unique<Poco::Timer>(5000, 60000);
    auto callback = Poco::TimerCallback<AutoRefresh>(*this, &AutoRefresh::onTimer);
    mTimer->start(callback);
}

ZapFR::Engine::AutoRefresh* ZapFR::Engine::AutoRefresh::getInstance()
{
    static AutoRefresh instance{};
    return &instance;
}

void ZapFR::Engine::AutoRefresh::onTimer([[maybe_unused]] Poco::Timer& timer)
{
    if (mEnabled)
    {
        std::vector<uint64_t> feedIDsToRefresh;
        Poco::DateTime now;

        auto sources = Source::getSources(IdentifierLocalServer);
        for (const auto& source : sources)
        {
            auto feeds = source->getFeeds(Source::FetchInfo::Data);
            for (const auto& feed : feeds)
            {
                Poco::DateTime feedLastChecked;

                try
                {
                    int tzDiff;
                    feedLastChecked = Poco::DateTimeParser::parse(Poco::DateTimeFormat::ISO8601_FORMAT, feed->lastChecked(), tzDiff);
                    feedLastChecked.makeUTC(tzDiff);
                }
                catch (...)
                {
                    continue;
                }

                Poco::Timespan interval;
                if (feed->refreshInterval().has_value())
                {
                    interval.assign(static_cast<int64_t>(feed->refreshInterval().value()), 0);
                }
                else
                {
                    interval.assign(static_cast<int64_t>(mFeedRefreshIntervalInSeconds), 0);
                }

                Poco::DateTime nextCheck = feedLastChecked + interval;

                if (nextCheck < now)
                {
                    feedIDsToRefresh.emplace_back(feed->id());
                }
            }

            static auto dummyCallback = [](uint64_t, Feed*) {};
            auto agent = ZapFR::Engine::Agent::getInstance();
            for (const auto& feedID : feedIDsToRefresh)
            {
                agent->queueRefreshFeed(source->id(), feedID, mFeedRefreshedCallback.has_value() ? mFeedRefreshedCallback.value() : dummyCallback);
            }
        }
    }
}
