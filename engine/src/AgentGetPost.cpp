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

#include "AgentGetPost.h"
#include "Feed.h"
#include "Post.h"
#include "Source.h"

ZapFR::Engine::AgentGetPost::AgentGetPost(uint64_t sourceID, uint64_t feedID, uint64_t postID, std::function<void(std::unique_ptr<ZapFR::Engine::Post>)> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mFeedID(feedID), mPostID(postID), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentGetPost::run()
{
    auto source = ZapFR::Engine::Source::getSource(mSourceID);
    if (source.has_value())
    {
        auto feed = source.value()->getFeed(mFeedID);
        if (feed.has_value())
        {
            auto post = feed.value()->getPost(mPostID);
            if (post.has_value())
            {
                mFinishedCallback(std::move(post.value()));
            }
        }
    }

    mIsDone = true;
}
