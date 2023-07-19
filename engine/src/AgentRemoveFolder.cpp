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

#include "AgentRemoveFolder.h"
#include "Source.h"

ZapFR::Engine::AgentRemoveFolder::AgentRemoveFolder(uint64_t sourceID, const std::string& folderHierarchy, std::function<void()> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mFolderHierarchy(folderHierarchy), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentRemoveFolder::run()
{
    auto source = ZapFR::Engine::Source::getSource(mSourceID);
    if (source.has_value())
    {
        // we remove both the exact (no wildcards) folder hierarchy, and everything that falls below, in subfolders
        source.value()->removeFolder(mFolderHierarchy);
        source.value()->removeFolder(mFolderHierarchy + "/%");
        mFinishedCallback();
    }

    mIsDone = true;
}
