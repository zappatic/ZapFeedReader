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

#include "ZapFR/agents/AgentGetFeedPosts.h"
#include "ZapFR/Feed.h"
#include "ZapFR/Source.h"

ZapFR::Engine::AgentGetFeedPosts::AgentGetFeedPosts(uint64_t sourceID, uint64_t feedID, uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter,
                                                    FlagColor flagColor,
                                                    std::function<void(uint64_t, const std::vector<ZapFR::Engine::Post*>&, uint64_t, uint64_t)> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mFeedID(feedID), mPerPage(perPage), mPage(page), mShowOnlyUnread(showOnlyUnread), mSearchFilter(searchFilter),
      mFlagColorFilter(flagColor), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentGetFeedPosts::run()
{
    auto source = Source::getSource(mSourceID);
    if (source.has_value())
    {
        auto feed = source.value()->getFeed(mFeedID);
        if (feed.has_value())
        {
            auto posts = feed.value()->getPosts(mPerPage, mPage, mShowOnlyUnread, mSearchFilter, mFlagColorFilter);
            std::vector<Post*> postPointers;
            for (const auto& post : posts)
            {
                postPointers.emplace_back(post.get());
            }

            mFinishedCallback(source.value()->id(), postPointers, mPage, feed.value()->getTotalPostCount(mShowOnlyUnread, mSearchFilter, mFlagColorFilter));
        }
    }

    mIsDone = true;
}
