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

#include "ZapFR/agents/folder/AgentFolderMove.h"
#include "ZapFR/Feed.h"
#include "ZapFR/Source.h"

ZapFR::Engine::AgentFolderMove::AgentFolderMove(uint64_t sourceID, uint64_t folderID, uint64_t newFolder, uint64_t newSortOrder, std::function<void()> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mFolderID(folderID), mNewFolderID(newFolder), mNewSortOrder(newSortOrder), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentFolderMove::run()
{
    auto source = Source::getSource(mSourceID);
    if (source.has_value())
    {
        source.value()->moveFolder(mFolderID, mNewFolderID, mNewSortOrder);
        mFinishedCallback();
    }

    mIsDone = true;
}
