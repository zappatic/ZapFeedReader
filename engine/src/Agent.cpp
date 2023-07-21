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

#include "Agent.h"
#include "AgentGetPost.h"
#include "AgentGetPosts.h"
#include "AgentMarkFeedRead.h"
#include "AgentMarkPostRead.h"
#include "AgentRefreshFeed.h"
#include "AgentRemoveFeed.h"
#include "AgentRemoveFolder.h"
#include "AgentSubscribeFeed.h"
#include "Feed.h"
#include "Post.h"

ZapFR::Engine::Agent::Agent()
{
    mThreadPool = std::make_unique<Poco::ThreadPool>();
    mQueueTimer = std::make_unique<Poco::Timer>(0, 500);
    auto callback = Poco::TimerCallback<Agent>(*this, &Agent::onQueueTimer);
    mQueueTimer->start(callback);
}

ZapFR::Engine::Agent* ZapFR::Engine::Agent::getInstance()
{
    static Agent instance{};
    return &instance;
}

void ZapFR::Engine::Agent::queueRefreshFeed(uint64_t sourceID, uint64_t feedID, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentRefreshFeed>(sourceID, feedID, finishedCallback));
}

void ZapFR::Engine::Agent::queueSubscribeFeed(uint64_t sourceID, const std::string& url, const std::string& folderHierarchy, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentSubscribeFeed>(sourceID, url, folderHierarchy, finishedCallback));
}

void ZapFR::Engine::Agent::queueRemoveFeed(uint64_t sourceID, uint64_t feedID, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentRemoveFeed>(sourceID, feedID, finishedCallback));
}

void ZapFR::Engine::Agent::queueRemoveFolder(uint64_t sourceID, const std::string& folderHierarchy, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentRemoveFolder>(sourceID, folderHierarchy, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetPosts(uint64_t sourceID, uint64_t feedID, uint64_t perPage, uint64_t page,
                                         std::function<void(uint64_t, uint64_t, std::vector<std::unique_ptr<Post>>)> finishedCallback)
{
    enqueue(std::make_unique<AgentGetPosts>(sourceID, feedID, perPage, page, finishedCallback));
}

void ZapFR::Engine::Agent::queueMarkPostRead(uint64_t sourceID, uint64_t feedID, uint64_t postID, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentMarkPostRead>(sourceID, feedID, postID, finishedCallback));
}

void ZapFR::Engine::Agent::queueMarkFeedRead(uint64_t sourceID, uint64_t feedID, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentMarkFeedRead>(sourceID, feedID, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetPost(uint64_t sourceID, uint64_t feedID, uint64_t postID, std::function<void(std::unique_ptr<Post>)> finishedCallback)
{
    enqueue(std::make_unique<AgentGetPost>(sourceID, feedID, postID, finishedCallback));
}

void ZapFR::Engine::Agent::onQueueTimer(Poco::Timer& /*timer*/)
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (mThreadPool->available() > 0 && mQueue.size() > 0)
    {
        auto task = std::move(mQueue.front());
        mQueue.pop_front();
        mThreadPool->start(*task);
        mRunningAgents.push_back(std::move(task));
    }

    // clear out the finished agents from the running agents vector
    std::erase_if(mRunningAgents, [](const std::unique_ptr<AgentRunnable>& agent) { return agent->isDone(); });
}

void ZapFR::Engine::Agent::enqueue(std::unique_ptr<AgentRunnable> agent)
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (mThreadPool->available() > 0)
    {
        mThreadPool->start(*agent);
        mRunningAgents.push_back(std::move(agent));
    }
    else
    {
        mQueue.push_back(std::move(agent));
    }
}
