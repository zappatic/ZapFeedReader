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

#include "ZapFR/agents/AgentFeedGetLogs.h"
#include "ZapFR/Feed.h"
#include "ZapFR/Log.h"
#include "ZapFR/Source.h"

ZapFR::Engine::AgentFeedGetLogs::AgentFeedGetLogs(uint64_t sourceID, uint64_t feedID, uint64_t perPage, uint64_t page,
                                                  std::function<void(uint64_t, const std::vector<Log*>&, uint64_t, uint64_t)> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mFeedID(feedID), mPerPage(perPage), mPage(page), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentFeedGetLogs::run()
{
    auto source = Source::getSource(mSourceID);
    if (source.has_value())
    {
        auto feed = source.value()->getFeed(mFeedID, ZapFR::Engine::Source::FetchInfo::None);
        if (feed.has_value())
        {
            auto logs = feed.value()->getLogs(mPerPage, mPage);
            std::vector<Log*> logPointers;
            for (const auto& log : logs)
            {
                logPointers.emplace_back(log.get());
            }

            mFinishedCallback(mSourceID, logPointers, mPage, feed.value()->getTotalLogCount());
        }
    }

    mIsDone = true;
}
