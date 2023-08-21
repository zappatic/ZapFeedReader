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
#include "Flag.h"
#include "Global.h"
#include "Script.h"

namespace ZapFR
{
    namespace Engine
    {
        class Feed;
        class Post;
        class Log;
        class Folder;
        class ScriptFolder;

        class Agent
        {
          public:
            Agent(const Agent&) = delete;
            Agent& operator=(const Agent&) = delete;
            virtual ~Agent() = default;

            static Agent* getInstance();

            // querying posts
            void queueGetFeedPosts(uint64_t sourceID, uint64_t feedID, uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                   std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback);
            void queueGetFolderPosts(uint64_t sourceID, uint64_t folderID, uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                     std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback);
            void queueGetSourcePosts(uint64_t sourceID, uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                     std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback);
            void queueGetFeedFlaggedPosts(FlagColor flagColor, uint64_t sourceID, uint64_t feedID, uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                          std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback);
            void queueGetFolderFlaggedPosts(FlagColor flagColor, uint64_t sourceID, uint64_t folderID, uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                            std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback);
            void queueGetSourceFlaggedPosts(FlagColor flagColor, uint64_t sourceID, uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                            std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback);
            void queueGetPost(uint64_t sourceID, uint64_t feedID, uint64_t postID, std::function<void(std::unique_ptr<Post>)> finishedCallback);

            // querying logs
            void queueGetSourceLogs(uint64_t sourceID, uint64_t perPage, uint64_t page,
                                    std::function<void(uint64_t, const std::vector<Log*>&, uint64_t, uint64_t)> finishedCallback);
            void queueGetFolderLogs(uint64_t sourceID, uint64_t folderID, uint64_t perPage, uint64_t page,
                                    std::function<void(uint64_t, const std::vector<Log*>&, uint64_t, uint64_t)> finishedCallback);
            void queueGetFeedLogs(uint64_t sourceID, uint64_t feedID, uint64_t perPage, uint64_t page,
                                  std::function<void(uint64_t, const std::vector<Log*>&, uint64_t, uint64_t)> finishedCallback);

            // querying sources
            void queueGetSourceTree(uint64_t sourceID,
                                    std::function<void(uint64_t, const std::string&, const std::vector<Folder*>&, const std::vector<Feed*>&)> finishedCallback);

            // post manipulation
            void queueMarkPostRead(uint64_t sourceID, uint64_t feedID, uint64_t postID, std::function<void(uint64_t, uint64_t, uint64_t)> finishedCallback);
            void queueMarkPostsUnread(uint64_t sourceID, std::vector<std::tuple<uint64_t, uint64_t>> feedAndPostIDs,
                                      std::function<void(uint64_t, std::vector<std::tuple<uint64_t, uint64_t>>)> finishedCallback);
            void queueMarkPostFlagged(uint64_t sourceID, uint64_t feedID, uint64_t postID, FlagColor flagColor,
                                      std::function<void(uint64_t, uint64_t, uint64_t, ZapFR::Engine::FlagColor)> finishedCallback);
            void queueMarkPostUnflagged(uint64_t sourceID, uint64_t feedID, uint64_t postID, const std::unordered_set<FlagColor>& flagColors,
                                        std::function<void(uint64_t, uint64_t, uint64_t, const std::unordered_set<ZapFR::Engine::FlagColor>&)> finishedCallback);
            void queueGetUsedFlagColors(uint64_t sourceID, std::function<void(uint64_t, const std::unordered_set<FlagColor>&)> finishedCallback);

            // feed manipulation
            void queueMarkFeedRead(uint64_t sourceID, uint64_t feedID, std::function<void(uint64_t, uint64_t)> finishedCallback);
            void queueMoveFeed(uint64_t sourceID, uint64_t feedID, uint64_t newFolder, uint64_t newSortOrder, std::function<void()> finishedCallback);
            void queueGetFeedUnreadCount(uint64_t sourceID, uint64_t feedID, std::function<void(uint64_t, uint64_t, uint64_t)> finishedCallback);
            void queueRemoveFeed(uint64_t sourceID, uint64_t feedID, std::function<void()> finishedCallback);
            void queueRefreshFeed(uint64_t sourceID, uint64_t feedID, std::function<void(uint64_t, uint64_t)> finishedCallback);
            void queueSubscribeFeed(uint64_t sourceID, const std::string& url, uint64_t folder, const std::vector<std::string>& newFolderHierarchy,
                                    std::function<void()> finishedCallback);

            // folder manipulation
            void queueMarkFolderRead(uint64_t sourceID, uint64_t folderID, std::function<void(uint64_t, std::unordered_set<uint64_t>)> finishedCallback);
            void queueMoveFolder(uint64_t sourceID, uint64_t folderID, uint64_t newFolder, uint64_t newSortOrder, std::function<void()> finishedCallback);
            void queueAddFolder(uint64_t sourceID, uint64_t parentFolderID, const std::string& title, std::function<void()> finishedCallback);
            void queueRemoveFolder(uint64_t sourceID, uint64_t folder, std::function<void()> finishedCallback);
            void queueRefreshFolder(uint64_t sourceID, uint64_t folderID, std::function<void(uint64_t, uint64_t)> finishedCallback);

            // source manipulation
            void queueMarkSourceRead(uint64_t sourceID, std::function<void(uint64_t)> finishedCallback);
            void queueRefreshSource(uint64_t sourceID, std::function<void(uint64_t, uint64_t)> finishedCallback);

            // script folders
            void queueGetScriptFolders(uint64_t sourceID, std::function<void(uint64_t, const std::vector<ScriptFolder*>&)> finishedCallback);
            void queueGetScriptFolderPosts(uint64_t sourceID, uint64_t scriptFolderID, uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                           std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback);
            void queueGetScriptFolderFlaggedPosts(FlagColor flagColor, uint64_t sourceID, uint64_t scriptFolderID, uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                                  std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback);
            void queueAddScriptFolder(uint64_t sourceID, const std::string& title, std::function<void(uint64_t)> finishedCallback);
            void queueUpdateScriptFolder(uint64_t sourceID, uint64_t scriptFolderID, const std::string& title, std::function<void(uint64_t, uint64_t)> finishedCallback);
            void queueRemoveScriptFolder(uint64_t sourceID, uint64_t scriptFolderID, std::function<void(uint64_t, uint64_t)> finishedCallback);

            // scripts
            void queueGetScripts(uint64_t sourceID, std::function<void(uint64_t, const std::vector<Script*>&)> finishedCallback);
            void queueUpdateScript(uint64_t sourceID, uint64_t scriptID, Script::Type type, const std::string& filename, bool enabled,
                                   const std::unordered_set<Script::Event>& events, const std::optional<std::unordered_set<uint64_t>>& feedIDs,
                                   std::function<void(uint64_t, uint64_t)> finishedCallback);
            void queueRemoveScript(uint64_t sourceID, uint64_t scriptID, std::function<void(uint64_t, uint64_t)> finishedCallback);
            void queueAddScript(uint64_t sourceID, Script::Type type, const std::string& filename, bool enabled, const std::unordered_set<Script::Event>& events,
                                const std::optional<std::unordered_set<uint64_t>>& feedIDs, std::function<void(uint64_t)> finishedCallback);

          private:
            explicit Agent();
            static std::mutex msMutex;

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