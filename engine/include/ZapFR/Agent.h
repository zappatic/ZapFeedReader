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

#include <deque>
#include <optional>

#include <Poco/ThreadPool.h>
#include <Poco/Timer.h>

#include "AgentRunnable.h"
#include "ZapFR/Flag.h"
#include "ZapFR/Global.h"
#include "ZapFR/base/Folder.h"
#include "ZapFR/base/Script.h"

namespace ZapFR
{
    namespace Engine
    {
        class Feed;
        class Source;
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
            void joinAll() const;
            void registerErrorCallback(std::function<void(uint64_t, const std::string&)> ecb) { mErrorCallback = ecb; }
            void broadcastError(uint64_t sourceID, const std::string& errorMessage) const;

            // querying posts
            void queueGetFeedPosts(uint64_t sourceID, uint64_t feedID, uint64_t perPage, uint64_t page, bool showOnlyUnread, bool showUnreadPostsAtTop,
                                   const std::string& searchFilter, uint64_t categoryFilterID, FlagColor flagColor,
                                   std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t, const std::vector<ThumbnailData>&)> finishedCallback);
            void queueGetFolderPosts(uint64_t sourceID, uint64_t folderID, uint64_t perPage, uint64_t page, bool showOnlyUnread, bool showUnreadPostsAtTop,
                                     const std::string& searchFilter, uint64_t categoryFilterID, FlagColor flagColor,
                                     std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t, const std::vector<ThumbnailData>&)> finishedCallback);
            void queueGetSourcePosts(uint64_t sourceID, uint64_t perPage, uint64_t page, bool showOnlyUnread, bool showUnreadPostsAtTop, const std::string& searchFilter,
                                     uint64_t categoryFilterID, FlagColor flagColor,
                                     std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t, const std::vector<ThumbnailData>&)> finishedCallback);
            void queueGetScriptFolderPosts(uint64_t sourceID, uint64_t scriptFolderID, uint64_t perPage, uint64_t page, bool showOnlyUnread, bool showUnreadPostsAtTop,
                                           const std::string& searchFilter, uint64_t categoryFilterID, FlagColor flagColor,
                                           std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t, const std::vector<ThumbnailData>&)> finishedCallback);
            void queueGetPost(uint64_t sourceID, uint64_t feedID, uint64_t postID, std::function<void(std::unique_ptr<Post>)> finishedCallback);

            // querying categories
            void queueGetFeedCategories(uint64_t sourceID, uint64_t feedID, std::function<void(uint64_t, uint64_t, const std::vector<Category*>&)> finishedCallback);
            void queueGetFolderCategories(uint64_t sourceID, uint64_t folderID, std::function<void(uint64_t, uint64_t, const std::vector<Category*>&)> finishedCallback);
            void queueGetSourceCategories(uint64_t sourceID, std::function<void(uint64_t, const std::vector<Category*>&)> finishedCallback);
            void queueGetScriptFolderCategories(uint64_t sourceID, uint64_t scriptFolderID,
                                                std::function<void(uint64_t, uint64_t, const std::vector<Category*>&)> finishedCallback);

            // querying logs
            void queueGetSourceLogs(uint64_t sourceID, uint64_t perPage, uint64_t page,
                                    std::function<void(uint64_t, const std::vector<Log*>&, uint64_t, uint64_t)> finishedCallback);
            void queueGetFolderLogs(uint64_t sourceID, uint64_t folderID, uint64_t perPage, uint64_t page,
                                    std::function<void(uint64_t, const std::vector<Log*>&, uint64_t, uint64_t)> finishedCallback);
            void queueGetFeedLogs(uint64_t sourceID, uint64_t feedID, uint64_t perPage, uint64_t page,
                                  std::function<void(uint64_t, const std::vector<Log*>&, uint64_t, uint64_t)> finishedCallback);

            // querying feeds
            void queueGetFeed(uint64_t sourceID, uint64_t feedID, std::function<void(uint64_t, Feed*)> finishedCallback);

            // querying folders
            void queueGetFolder(uint64_t sourceID, uint64_t folderID, std::function<void(Folder*)> finishedCallback);
            void queueGetFolders(uint64_t sourceID, uint64_t folderID, std::function<void(const std::vector<Folder*>&)> finishedCallback);

            // querying sources
            void queueGetSourceTree(uint64_t sourceID, std::function<void(Source*, const std::vector<Folder*>&, const std::vector<Feed*>&)> finishedCallback);
            void queueGetSourceStatus(uint64_t sourceID, std::function<void(uint64_t, const Poco::JSON::Object&)> finishedCallback);
            void queueGetSource(uint64_t sourceID, std::function<void(Source*)> finishedCallback);

            // post manipulation
            void queueMarkPostsRead(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs,
                                    std::function<void(uint64_t, const std::vector<std::tuple<uint64_t, uint64_t>>&)> finishedCallback);
            void queueMarkPostsUnread(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs,
                                      std::function<void(uint64_t, const std::vector<std::tuple<uint64_t, uint64_t>>&)> finishedCallback);
            void
            queueMarkPostsFlagged(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs, const std::unordered_set<FlagColor>& flagColors,
                                  std::function<void(uint64_t, const std::vector<std::tuple<uint64_t, uint64_t>>&, const std::unordered_set<FlagColor>&)> finishedCallback);
            void
            queueMarkPostsUnflagged(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs, const std::unordered_set<FlagColor>& flagColors,
                                    std::function<void(uint64_t, const std::vector<std::tuple<uint64_t, uint64_t>>&, const std::unordered_set<FlagColor>&)> finishedCallback);
            void queueAssignPostsToScriptFolder(uint64_t sourceID, uint64_t scriptFolderID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs,
                                                std::function<void(uint64_t, uint64_t)> finishedCallback);
            void queueRemovePostsFromScriptFolder(uint64_t sourceID, uint64_t scriptFolderID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs,
                                                  std::function<void(uint64_t, uint64_t)> finishedCallback);
            void queueGetUsedFlagColors(uint64_t sourceID, std::function<void(uint64_t, const std::unordered_set<FlagColor>&)> finishedCallback);

            // feed manipulation
            void queueMarkFeedRead(uint64_t sourceID, uint64_t feedID, uint64_t maxPostID, std::function<void(uint64_t, uint64_t)> finishedCallback);
            void queueMoveFeed(uint64_t sourceID, uint64_t feedID, uint64_t newFolder, uint64_t newSortOrder,
                               std::function<void(uint64_t, const std::unordered_map<uint64_t, uint64_t>&)> finishedCallback);
            void queueGetFeedUnreadCount(uint64_t sourceID, uint64_t feedID, std::function<void(uint64_t, uint64_t, uint64_t)> finishedCallback);
            void queueRemoveFeed(uint64_t sourceID, uint64_t feedID, std::function<void(uint64_t, uint64_t)> finishedCallback);
            void queueRefreshFeed(uint64_t sourceID, uint64_t feedID, std::function<void(uint64_t, Feed* refreshedFeed)> finishedCallback);
            void queueAddFeed(uint64_t sourceID, const std::string& url, uint64_t folder, std::function<void(uint64_t, Feed*)> finishedCallback);
            void queueImportOPML(uint64_t sourceID, const std::string& opml, uint64_t parentFolderID, std::function<void()> opmlParsedCallback,
                                 std::function<void(uint64_t, Feed*)> feedRefreshedCallback);
            void queueUpdateFeed(uint64_t sourceID, uint64_t feedID, const std::string& feedURL, std::optional<uint64_t> refreshIntervalInSeconds,
                                 std::function<void()> finishedCallback);

            // folder manipulation
            void queueMarkFolderRead(uint64_t sourceID, uint64_t folderID, uint64_t maxPostID, std::function<void(uint64_t, std::vector<uint64_t>)> finishedCallback);
            void queueMoveFolder(uint64_t sourceID, uint64_t folderID, uint64_t newFolder, uint64_t newSortOrder,
                                 std::function<void(uint64_t, const std::unordered_map<uint64_t, uint64_t>&)> finishedCallback);
            void queueAddFolder(uint64_t sourceID, uint64_t parentFolderID, const std::string& title,
                                std::function<void(uint64_t, uint64_t, uint64_t, uint64_t, const std::string&)> finishedCallback);
            void queueRemoveFolder(uint64_t sourceID, uint64_t folder, std::function<void(uint64_t, uint64_t)> finishedCallback);
            void queueUpdateFolder(uint64_t sourceID, uint64_t folder, const std::string& newTitle,
                                   std::function<void(uint64_t, uint64_t, const std::string&)> finishedCallback);
            void queueRefreshFolder(uint64_t sourceID, uint64_t folderID, std::function<void(uint64_t, Feed*)> finishedCallback);
            void queueSortFolder(
                uint64_t sourceID, uint64_t folderID, SortMethod sortMethod,
                std::function<void(uint64_t, uint64_t, const std::unordered_map<uint64_t, uint64_t>&, const std::unordered_map<uint64_t, uint64_t>&)> finishedCallback);

            // source manipulation
            void queueMarkSourceRead(uint64_t sourceID, uint64_t maxPostID, std::function<void(uint64_t)> finishedCallback);
            void queueRefreshSource(uint64_t sourceID, std::function<void(uint64_t, Feed*)> finishedCallback);

            // log manipulation
            void queueClearSourceLogs(uint64_t sourceID, std::function<void()> finishedCallback);
            void queueClearFeedLogs(uint64_t sourceID, uint64_t feedID, std::function<void()> finishedCallback);
            void queueClearFolderLogs(uint64_t sourceID, uint64_t folderID, std::function<void()> finishedCallback);

            // script folders
            void queueGetScriptFolders(uint64_t sourceID, std::function<void(uint64_t, const std::vector<ScriptFolder*>&)> finishedCallback);
            void queueAddScriptFolder(uint64_t sourceID, const std::string& title, bool showTotal, bool showUnread, std::function<void(uint64_t)> finishedCallback);
            void queueUpdateScriptFolder(uint64_t sourceID, uint64_t scriptFolderID, const std::string& title, bool showTotal, bool showUnread,
                                         std::function<void(uint64_t, uint64_t)> finishedCallback);
            void queueRemoveScriptFolder(uint64_t sourceID, uint64_t scriptFolderID, std::function<void(uint64_t, uint64_t)> finishedCallback);
            void queueMarkScriptFolderRead(uint64_t sourceID, uint64_t scriptFolderID, uint64_t maxPostID,
                                           std::function<void(uint64_t, std::vector<uint64_t>)> finishedCallback);

            // scripts
            void queueGetScripts(uint64_t sourceID, std::function<void(uint64_t, const std::vector<Script*>&)> finishedCallback);
            void queueUpdateScript(uint64_t sourceID, uint64_t scriptID, Script::Type type, const std::string& title, bool enabled,
                                   const std::unordered_set<Script::Event>& events, const std::optional<std::unordered_set<uint64_t>>& feedIDs, const std::string& script,
                                   std::function<void(uint64_t, uint64_t)> finishedCallback);
            void queueRemoveScript(uint64_t sourceID, uint64_t scriptID, std::function<void(uint64_t, uint64_t)> finishedCallback);
            void queueAddScript(uint64_t sourceID, Script::Type type, const std::string& title, bool enabled, const std::unordered_set<Script::Event>& events,
                                const std::optional<std::unordered_set<uint64_t>>& feedIDs, const std::string& script, std::function<void(uint64_t)> finishedCallback);

            // monitoring
            void queueMonitorFeedRefreshCompletion(std::function<void()> finishedCallback);
            void queueMonitorSourceReloadCompletion(std::function<void()> finishedCallback);
            uint64_t totalCountOfType(AgentRunnable::Type t) const;

          private:
            explicit Agent();
            static std::mutex msMutex;

            std::deque<std::unique_ptr<AgentRunnable>> mQueue{};
            std::unique_ptr<Poco::Timer> mQueueTimer{nullptr};
            std::unique_ptr<Poco::ThreadPool> mThreadPool{nullptr};
            std::vector<std::unique_ptr<AgentRunnable>> mRunningAgents{};

            std::optional<std::function<void(uint64_t, const std::string&)>> mErrorCallback{};

            void onQueueTimer(Poco::Timer& timer);
            void enqueue(std::unique_ptr<AgentRunnable> agent);
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_AGENT_H