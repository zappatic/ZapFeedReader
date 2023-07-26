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

#ifndef ZAPFR_ENGINE_AGENT_H
#define ZAPFR_ENGINE_AGENT_H

#include "AgentRunnable.h"
#include "Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class Feed;
        class Post;

        class Agent
        {
          public:
            Agent(const Agent&) = delete;
            Agent& operator=(const Agent&) = delete;
            virtual ~Agent() = default;

            static Agent* getInstance();

            void queueRefreshFeed(uint64_t sourceID, uint64_t feedID, std::function<void()> finishedCallback);
            void queueRefreshAllFeeds(std::function<void()> finishedCallback);
            void queueSubscribeFeed(uint64_t sourceID, const std::string& url, uint64_t folder, std::function<void()> finishedCallback);
            void queueRemoveFeed(uint64_t sourceID, uint64_t feedID, std::function<void()> finishedCallback);
            void queueRemoveFolder(uint64_t sourceID, uint64_t folder, std::function<void()> finishedCallback);
            void queueGetPosts(uint64_t sourceID, uint64_t feedID, uint64_t perPage, uint64_t page,
                               std::function<void(uint64_t, uint64_t, std::vector<std::unique_ptr<Post>>)> finishedCallback);
            void queueMarkPostRead(uint64_t sourceID, uint64_t feedID, uint64_t postID, std::function<void()> finishedCallback);
            void queueMarkFeedRead(uint64_t sourceID, uint64_t feedID, std::function<void()> finishedCallback);
            void queueGetPost(uint64_t sourceID, uint64_t feedID, uint64_t postID, std::function<void(std::unique_ptr<Post>)> finishedCallback);
            void queueAddFolder(uint64_t sourceID, uint64_t parentFolderID, const std::string& title, std::function<void()> finishedCallback);

          private:
            explicit Agent();
            std::mutex mMutex{};

            std::deque<std::unique_ptr<AgentRunnable>> mQueue{};
            std::unique_ptr<Poco::Timer> mQueueTimer{nullptr};
            std::unique_ptr<Poco::ThreadPool> mThreadPool{nullptr};
            std::vector<std::unique_ptr<AgentRunnable>> mRunningAgents{};

            void onQueueTimer(Poco::Timer& timer);
            void enqueue(std::unique_ptr<AgentRunnable> agent);
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_AGENT_H