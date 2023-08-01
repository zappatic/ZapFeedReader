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

#include "agents/AgentGetSourcePosts.h"
#include "Feed.h"
#include "Source.h"

ZapFR::Engine::AgentGetSourcePosts::AgentGetSourcePosts(uint64_t sourceID, uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                                        std::function<void(uint64_t, const std::vector<ZapFR::Engine::Post*>&, uint64_t, uint64_t)> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mPerPage(perPage), mPage(page), mShowOnlyUnread(showOnlyUnread), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentGetSourcePosts::run()
{
    auto source = ZapFR::Engine::Source::getSource(mSourceID);
    if (source.has_value())
    {
        auto posts = source.value()->getPosts(mPerPage, mPage, mShowOnlyUnread);
        std::vector<Post*> postPointers;
        for (const auto& post : posts)
        {
            postPointers.emplace_back(post.get());
        }

        mFinishedCallback(source.value()->id(), postPointers, mPage, source.value()->getTotalPostCount(mShowOnlyUnread));
    }

    mIsDone = true;
}
