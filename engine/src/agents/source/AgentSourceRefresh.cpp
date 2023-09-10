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

#include "ZapFR/agents/source/AgentSourceRefresh.h"
#include "ZapFR/Agent.h"
#include "ZapFR/base/Feed.h"
#include "ZapFR/base/Source.h"

ZapFR::Engine::AgentSourceRefresh::AgentSourceRefresh(uint64_t sourceID, std::function<void(uint64_t, Feed*)> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentSourceRefresh::run()
{
    auto source = Source::getSource(mSourceID);
    if (source.has_value())
    {
        auto feeds = source.value()->getFeeds(Source::FetchInfo::None);
        for (const auto& feed : feeds)
        {
            // We just create agent threads here instead of refreshing the source manually
            // so the refreshing can be done concurrently
            // The callback will be called for each feed that is refreshed (with the feed ID as the parameter)
            Agent::getInstance()->queueRefreshFeed(mSourceID, feed->id(), mFinishedCallback);
        }
    }

    mIsDone = true;
}
