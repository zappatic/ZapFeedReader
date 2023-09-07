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

#include "ZapFR/agents/AgentScriptFolderAssignPosts.h"
#include "ZapFR/Feed.h"
#include "ZapFR/Post.h"
#include "ZapFR/Source.h"

ZapFR::Engine::AgentScriptFolderAssignPosts::AgentScriptFolderAssignPosts(uint64_t sourceID, uint64_t scriptFolderID,
                                                                          const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs,
                                                                          std::function<void(uint64_t, uint64_t)> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mScriptFolderID(scriptFolderID), mFeedAndPostIDs(feedAndPostIDs), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentScriptFolderAssignPosts::run()
{
    auto source = Source::getSource(mSourceID);
    if (source.has_value())
    {
        source.value()->assignPostsToScriptFolder(mScriptFolderID, true, mFeedAndPostIDs);
        mFinishedCallback(mSourceID, mScriptFolderID);
    }

    mIsDone = true;
}
