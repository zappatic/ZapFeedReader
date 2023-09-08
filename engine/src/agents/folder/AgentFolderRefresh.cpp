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

#include "ZapFR/agents/folder/AgentFolderRefresh.h"
#include "ZapFR/Agent.h"
#include "ZapFR/Feed.h"
#include "ZapFR/Folder.h"
#include "ZapFR/Source.h"

ZapFR::Engine::AgentFolderRefresh::AgentFolderRefresh(uint64_t sourceID, uint64_t folderID,
                                                      std::function<void(uint64_t, uint64_t, uint64_t, const std::optional<std::string>&)> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mFolderID(folderID), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentFolderRefresh::run()
{
    auto source = Source::getSource(mSourceID);
    if (source.has_value())
    {
        auto folder = source.value()->getFolder(mFolderID, ZapFR::Engine::Source::FetchInfo::FolderFeedIDs);
        if (folder.has_value())
        {
            auto feedIDs = folder.value()->feedIDsInFoldersAndSubfolders();
            for (const auto& feedID : feedIDs)
            {
                // We just create agent threads here instead of refreshing the folder manually
                // so the refreshing can be done concurrently
                // The callback will be called for each feed that is refreshed (with the feed ID as the parameter)
                Agent::getInstance()->queueRefreshFeed(mSourceID, feedID, mFinishedCallback);
            }
        }
    }

    mIsDone = true;
}
