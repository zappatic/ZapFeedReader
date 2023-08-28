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

#include "ZapFR/agents/AgentPostsMarkUnread.h"
#include "ZapFR/Feed.h"
#include "ZapFR/Source.h"

ZapFR::Engine::AgentPostsMarkUnread::AgentPostsMarkUnread(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs,
                                                          std::function<void(uint64_t, const std::vector<std::tuple<uint64_t, uint64_t>>&)> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mFeedAndPostIDs(feedAndPostIDs), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentPostsMarkUnread::run()
{
    auto source = Source::getSource(mSourceID);
    if (source.has_value())
    {
        // remap the vector of tuples to feed -> [post, ...] map, so we can handle it one feed at a time
        std::unordered_map<uint64_t, std::vector<uint64_t>> feedsWithPostsMap;
        for (const auto& [feedID, postID] : mFeedAndPostIDs)
        {
            if (feedsWithPostsMap.contains(feedID))
            {
                feedsWithPostsMap.at(feedID).emplace_back(postID);
            }
            else
            {
                std::vector<uint64_t> vec;
                vec.emplace_back(postID);
                feedsWithPostsMap[feedID] = vec;
            }
        }

        // mark the posts as read per feed
        for (const auto& [feedID, posts] : feedsWithPostsMap)
        {
            auto feed = source.value()->getFeed(feedID, false);
            for (const auto& postID : posts)
            {
                feed.value()->markAsUnread(postID);
            }
        }
        mFinishedCallback(mSourceID, mFeedAndPostIDs);
    }

    mIsDone = true;
}
