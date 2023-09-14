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

#include "ZapFR/agents/source/AgentSourceGetLogs.h"
#include "ZapFR/Log.h"
#include "ZapFR/base/Source.h"

ZapFR::Engine::AgentSourceGetLogs::AgentSourceGetLogs(uint64_t sourceID, uint64_t perPage, uint64_t page,
                                                      std::function<void(uint64_t, const std::vector<Log*>&, uint64_t, uint64_t)> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mPerPage(perPage), mPage(page), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentSourceGetLogs::run()
{
    auto source = Source::getSource(mSourceID);
    if (source.has_value())
    {
        std::vector<Log*> logPointers;
        std::vector<std::unique_ptr<Log>> logs;
        uint64_t logCount{0};
        try
        {
            std::tie(logCount, logs) = source.value()->getLogs(mPerPage, mPage);
            for (const auto& log : logs)
            {
                logPointers.emplace_back(log.get());
            }
        }
        CATCH_AND_LOG_EXCEPTION_IN_SOURCE
        mFinishedCallback(mSourceID, logPointers, mPage, logCount);
    }

    mIsDone = true;
}
