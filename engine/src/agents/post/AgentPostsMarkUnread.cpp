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

#include "ZapFR/agents/post/AgentPostsMarkUnread.h"
#include "ZapFR/Agent.h"
#include "ZapFR/base/Feed.h"
#include "ZapFR/base/Source.h"

ZapFR::Engine::AgentPostsMarkUnread::AgentPostsMarkUnread(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs,
                                                          std::function<void(uint64_t, const std::vector<std::tuple<uint64_t, uint64_t>>&)> finishedCallback)
    : AgentRunnable(sourceID), mFeedAndPostIDs(feedAndPostIDs), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentPostsMarkUnread::payload(Source* source)
{
    source->setPostsReadStatus(false, mFeedAndPostIDs);
    mFinishedCallback(mSourceID, mFeedAndPostIDs);
}
