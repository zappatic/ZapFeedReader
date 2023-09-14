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

#include "ZapFR/agents/folder/AgentFolderMarkRead.h"
#include "ZapFR/base/Feed.h"
#include "ZapFR/base/Folder.h"
#include "ZapFR/base/Source.h"

ZapFR::Engine::AgentFolderMarkRead::AgentFolderMarkRead(uint64_t sourceID, uint64_t folderID, std::function<void(uint64_t, std::unordered_set<uint64_t>)> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mFolderID(folderID), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentFolderMarkRead::run()
{
    auto source = Source::getSource(mSourceID);
    if (source.has_value())
    {
        std::unordered_set<uint64_t> feedIDs;
        try
        {
            auto folder = source.value()->getFolder(mFolderID, ZapFR::Engine::Source::FetchInfo::None);
            if (folder.has_value())
            {
                feedIDs = folder.value()->markAllAsRead();
            }
        }
        CATCH_AND_LOG_EXCEPTION_IN_SOURCE
        mFinishedCallback(mSourceID, feedIDs);
    }

    mIsDone = true;
}
