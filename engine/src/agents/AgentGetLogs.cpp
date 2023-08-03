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

#include "agents/AgentGetLogs.h"
#include "Log.h"
#include "Source.h"

ZapFR::Engine::AgentGetLogs::AgentGetLogs(uint64_t sourceID, std::optional<uint64_t> feedID, uint64_t perPage, uint64_t page,
                                          std::function<void(uint64_t, std::optional<uint64_t>, const std::vector<Log*>&, uint64_t, uint64_t)> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mFeedID(feedID), mPerPage(perPage), mPage(page), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentGetLogs::run()
{
    auto source = ZapFR::Engine::Source::getSource(mSourceID);
    if (source.has_value())
    {
        auto logs = source.value()->getLogs(mFeedID, mPerPage, mPage);
        std::vector<Log*> logPointers;
        for (const auto& log : logs)
        {
            logPointers.emplace_back(log.get());
        }

        // TODO: total log record count
        mFinishedCallback(mSourceID, mFeedID, logPointers, mPage, 10000 /*folder.value()->getTotalPostCount(mShowOnlyUnread)*/);
    }

    mIsDone = true;
}
