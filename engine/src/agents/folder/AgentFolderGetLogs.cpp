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

#include "ZapFR/agents/folder/AgentFolderGetLogs.h"
#include "ZapFR/Agent.h"
#include "ZapFR/Log.h"
#include "ZapFR/base/Folder.h"
#include "ZapFR/base/Source.h"

ZapFR::Engine::AgentFolderGetLogs::AgentFolderGetLogs(uint64_t sourceID, uint64_t folderID, uint64_t perPage, uint64_t page,
                                                      std::function<void(uint64_t, const std::vector<Log*>&, uint64_t, uint64_t)> finishedCallback)
    : AgentRunnable(sourceID), mFolderID(folderID), mPerPage(perPage), mPage(page), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentFolderGetLogs::payload(Source* source)
{
    auto folder = source->getFolder(mFolderID, ZapFR::Engine::Source::FetchInfo::None);
    if (folder.has_value())
    {
        auto [logCount, logs] = folder.value()->getLogs(mPerPage, mPage);
        std::vector<Log*> logPointers;
        for (const auto& log : logs)
        {
            logPointers.emplace_back(log.get());
        }
        mFinishedCallback(mSourceID, logPointers, mPage, logCount);
    }
}
