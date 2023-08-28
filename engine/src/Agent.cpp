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

#include "ZapFR/Agent.h"
#include "ZapFR/Feed.h"
#include "ZapFR/Post.h"
#include "ZapFR/Source.h"
#include "ZapFR/agents/AgentFeedGet.h"
#include "ZapFR/agents/AgentFeedGetLogs.h"
#include "ZapFR/agents/AgentFeedGetPosts.h"
#include "ZapFR/agents/AgentFeedGetUnreadCount.h"
#include "ZapFR/agents/AgentFeedMarkRead.h"
#include "ZapFR/agents/AgentFeedMove.h"
#include "ZapFR/agents/AgentFeedRefresh.h"
#include "ZapFR/agents/AgentFeedRemove.h"
#include "ZapFR/agents/AgentFeedSetProperties.h"
#include "ZapFR/agents/AgentFeedSubscribe.h"
#include "ZapFR/agents/AgentFolderAdd.h"
#include "ZapFR/agents/AgentFolderGet.h"
#include "ZapFR/agents/AgentFolderGetLogs.h"
#include "ZapFR/agents/AgentFolderGetPosts.h"
#include "ZapFR/agents/AgentFolderMarkRead.h"
#include "ZapFR/agents/AgentFolderMove.h"
#include "ZapFR/agents/AgentFolderRefresh.h"
#include "ZapFR/agents/AgentFolderRemove.h"
#include "ZapFR/agents/AgentPostGet.h"
#include "ZapFR/agents/AgentPostsMarkFlagged.h"
#include "ZapFR/agents/AgentPostsMarkRead.h"
#include "ZapFR/agents/AgentPostsMarkUnflagged.h"
#include "ZapFR/agents/AgentPostsMarkUnread.h"
#include "ZapFR/agents/AgentScriptAdd.h"
#include "ZapFR/agents/AgentScriptFolderAdd.h"
#include "ZapFR/agents/AgentScriptFolderAssignPosts.h"
#include "ZapFR/agents/AgentScriptFolderGetPosts.h"
#include "ZapFR/agents/AgentScriptFolderRemove.h"
#include "ZapFR/agents/AgentScriptFolderRemovePosts.h"
#include "ZapFR/agents/AgentScriptFolderUpdate.h"
#include "ZapFR/agents/AgentScriptFoldersGet.h"
#include "ZapFR/agents/AgentScriptRemove.h"
#include "ZapFR/agents/AgentScriptUpdate.h"
#include "ZapFR/agents/AgentScriptsGet.h"
#include "ZapFR/agents/AgentSourceGetLogs.h"
#include "ZapFR/agents/AgentSourceGetPosts.h"
#include "ZapFR/agents/AgentSourceGetTree.h"
#include "ZapFR/agents/AgentSourceGetUsedFlagColors.h"
#include "ZapFR/agents/AgentSourceMarkRead.h"
#include "ZapFR/agents/AgentSourceRefresh.h"

std::mutex ZapFR::Engine::Agent::msMutex{};

ZapFR::Engine::Agent::Agent()
{
    mThreadPool = std::make_unique<Poco::ThreadPool>();
    mQueueTimer = std::make_unique<Poco::Timer>(0, 50);
    auto callback = Poco::TimerCallback<Agent>(*this, &Agent::onQueueTimer);
    mQueueTimer->start(callback);
}

ZapFR::Engine::Agent* ZapFR::Engine::Agent::getInstance()
{
    static Agent instance{};
    return &instance;
}

void ZapFR::Engine::Agent::joinAll() const
{
    mThreadPool->joinAll();
}

void ZapFR::Engine::Agent::onQueueTimer(Poco::Timer& /*timer*/)
{
    if (mQueue.size() == 0)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(msMutex);
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
    std::lock_guard<std::mutex> lock(msMutex);
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

void ZapFR::Engine::Agent::queueRefreshFeed(uint64_t sourceID, uint64_t feedID,
                                            std::function<void(uint64_t, uint64_t, uint64_t, const std::optional<std::string>&)> finishedCallback)
{
    enqueue(std::make_unique<AgentFeedRefresh>(sourceID, feedID, finishedCallback));
}

void ZapFR::Engine::Agent::queueRefreshFolder(uint64_t sourceID, uint64_t folderID,
                                              std::function<void(uint64_t, uint64_t, uint64_t, const std::optional<std::string>&)> finishedCallback)
{
    enqueue(std::make_unique<AgentFolderRefresh>(sourceID, folderID, finishedCallback));
}

void ZapFR::Engine::Agent::queueRefreshSource(uint64_t sourceID, std::function<void(uint64_t, uint64_t, uint64_t, const std::optional<std::string>&)> finishedCallback)
{
    enqueue(std::make_unique<AgentSourceRefresh>(sourceID, finishedCallback));
}

void ZapFR::Engine::Agent::queueSubscribeFeed(uint64_t sourceID, const std::string& url, uint64_t folder, const std::vector<std::string>& newFolderHierarchy,
                                              std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentFeedSubscribe>(sourceID, url, folder, newFolderHierarchy, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetFeed(uint64_t sourceID, uint64_t feedID, std::function<void(Feed*)> finishedCallback)
{
    enqueue(std::make_unique<AgentFeedGet>(sourceID, feedID, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetFolder(uint64_t sourceID, uint64_t folderID, std::function<void(Folder*)> finishedCallback)
{
    enqueue(std::make_unique<AgentFolderGet>(sourceID, folderID, finishedCallback));
}

void ZapFR::Engine::Agent::queueRemoveFeed(uint64_t sourceID, uint64_t feedID, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentFeedRemove>(sourceID, feedID, finishedCallback));
}

void ZapFR::Engine::Agent::queueMoveFeed(uint64_t sourceID, uint64_t feedID, uint64_t newFolder, uint64_t newSortOrder, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentFeedMove>(sourceID, feedID, newFolder, newSortOrder, finishedCallback));
}

void ZapFR::Engine::Agent::queueMoveFolder(uint64_t sourceID, uint64_t folderID, uint64_t newFolder, uint64_t newSortOrder, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentFolderMove>(sourceID, folderID, newFolder, newSortOrder, finishedCallback));
}

void ZapFR::Engine::Agent::queueRemoveFolder(uint64_t sourceID, uint64_t folder, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentFolderRemove>(sourceID, folder, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetFeedPosts(uint64_t sourceID, uint64_t feedID, uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter,
                                             FlagColor flagColor, std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentFeedGetPosts>(sourceID, feedID, perPage, page, showOnlyUnread, searchFilter, flagColor, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetFolderPosts(uint64_t sourceID, uint64_t folderID, uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter,
                                               FlagColor flagColor, std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentFolderGetPosts>(sourceID, folderID, perPage, page, showOnlyUnread, searchFilter, flagColor, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetSourcePosts(uint64_t sourceID, uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter, FlagColor flagColor,
                                               std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentSourceGetPosts>(sourceID, perPage, page, showOnlyUnread, searchFilter, flagColor, finishedCallback));
}

void ZapFR::Engine::Agent::queueMarkPostsRead(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs,
                                              std::function<void(uint64_t, const std::vector<std::tuple<uint64_t, uint64_t>>&)> finishedCallback)
{
    enqueue(std::make_unique<AgentPostsMarkRead>(sourceID, feedAndPostIDs, finishedCallback));
}

void ZapFR::Engine::Agent::queueMarkPostsUnread(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs,
                                                std::function<void(uint64_t, const std::vector<std::tuple<uint64_t, uint64_t>>&)> finishedCallback)
{
    enqueue(std::make_unique<AgentPostsMarkUnread>(sourceID, feedAndPostIDs, finishedCallback));
}

void ZapFR::Engine::Agent::queueMarkFeedRead(uint64_t sourceID, uint64_t feedID, std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentFeedMarkRead>(sourceID, feedID, finishedCallback));
}

void ZapFR::Engine::Agent::queueMarkFolderRead(uint64_t sourceID, uint64_t folderID, std::function<void(uint64_t, std::unordered_set<uint64_t>)> finishedCallback)
{
    enqueue(std::make_unique<AgentFolderMarkRead>(sourceID, folderID, finishedCallback));
}

void ZapFR::Engine::Agent::queueMarkSourceRead(uint64_t sourceID, std::function<void(uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentSourceMarkRead>(sourceID, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetPost(uint64_t sourceID, uint64_t feedID, uint64_t postID, std::function<void(std::unique_ptr<Post>)> finishedCallback)
{
    enqueue(std::make_unique<AgentPostGet>(sourceID, feedID, postID, finishedCallback));
}

void ZapFR::Engine::Agent::queueAddFolder(uint64_t sourceID, uint64_t parentFolderID, const std::string& title, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentFolderAdd>(sourceID, parentFolderID, title, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetSourceLogs(uint64_t sourceID, uint64_t perPage, uint64_t page,
                                              std::function<void(uint64_t, const std::vector<Log*>&, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentSourceGetLogs>(sourceID, perPage, page, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetFolderLogs(uint64_t sourceID, uint64_t folderID, uint64_t perPage, uint64_t page,
                                              std::function<void(uint64_t, const std::vector<Log*>&, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentFolderGetLogs>(sourceID, folderID, perPage, page, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetFeedLogs(uint64_t sourceID, uint64_t feedID, uint64_t perPage, uint64_t page,
                                            std::function<void(uint64_t, const std::vector<Log*>&, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentFeedGetLogs>(sourceID, feedID, perPage, page, finishedCallback));
}

void ZapFR::Engine::Agent::queueMarkPostsFlagged(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs,
                                                 const std::unordered_set<FlagColor>& flagColors, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentPostsMarkFlagged>(sourceID, feedAndPostIDs, flagColors, finishedCallback));
}

void ZapFR::Engine::Agent::queueMarkPostsUnflagged(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs,
                                                   const std::unordered_set<FlagColor>& flagColors, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentPostsMarkUnflagged>(sourceID, feedAndPostIDs, flagColors, finishedCallback));
}

void ZapFR::Engine::Agent::queueAssignPostsToScriptFolder(uint64_t sourceID, uint64_t scriptFolderID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs,
                                                          std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentScriptFolderAssignPosts>(sourceID, scriptFolderID, feedAndPostIDs, finishedCallback));
}

void ZapFR::Engine::Agent::queueRemovePostsFromScriptFolder(uint64_t sourceID, uint64_t scriptFolderID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs,
                                                            std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentScriptFolderRemovePosts>(sourceID, scriptFolderID, feedAndPostIDs, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetSourceTree(uint64_t sourceID,
                                              std::function<void(uint64_t, const std::string&, const std::vector<Folder*>&, const std::vector<Feed*>& feeds)> finishedCallback)
{
    enqueue(std::make_unique<AgentSourceGetTree>(sourceID, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetFeedUnreadCount(uint64_t sourceID, uint64_t feedID, std::function<void(uint64_t, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentFeedGetUnreadCount>(sourceID, feedID, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetUsedFlagColors(uint64_t sourceID, std::function<void(uint64_t, const std::unordered_set<FlagColor>&)> finishedCallback)
{
    enqueue(std::make_unique<AgentSourceGetUsedFlagColors>(sourceID, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetScriptFolders(uint64_t sourceID, std::function<void(uint64_t, const std::vector<ScriptFolder*>&)> finishedCallback)
{
    enqueue(std::make_unique<AgentScriptFoldersGet>(sourceID, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetScriptFolderPosts(uint64_t sourceID, uint64_t scriptFolderID, uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                                     const std::string& searchFilter, FlagColor flagColor,
                                                     std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentScriptFolderGetPosts>(sourceID, scriptFolderID, perPage, page, showOnlyUnread, searchFilter, flagColor, finishedCallback));
}

void ZapFR::Engine::Agent::queueAddScriptFolder(uint64_t sourceID, const std::string& title, std::function<void(uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentScriptFolderAdd>(sourceID, title, finishedCallback));
}

void ZapFR::Engine::Agent::queueUpdateScriptFolder(uint64_t sourceID, uint64_t scriptFolderID, const std::string& title,
                                                   std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentScriptFolderUpdate>(sourceID, scriptFolderID, title, finishedCallback));
}

void ZapFR::Engine::Agent::queueRemoveScriptFolder(uint64_t sourceID, uint64_t scriptFolderID, std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentScriptFolderRemove>(sourceID, scriptFolderID, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetScripts(uint64_t sourceID, std::function<void(uint64_t, const std::vector<Script*>&)> finishedCallback)
{
    enqueue(std::make_unique<AgentScriptsGet>(sourceID, finishedCallback));
}

void ZapFR::Engine::Agent::queueUpdateScript(uint64_t sourceID, uint64_t scriptID, Script::Type type, const std::string& filename, bool enabled,
                                             const std::unordered_set<Script::Event>& events, const std::optional<std::unordered_set<uint64_t>>& feedIDs,
                                             std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentScriptUpdate>(sourceID, scriptID, type, filename, enabled, events, feedIDs, finishedCallback));
}

void ZapFR::Engine::Agent::queueRemoveScript(uint64_t sourceID, uint64_t scriptID, std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentScriptRemove>(sourceID, scriptID, finishedCallback));
}

void ZapFR::Engine::Agent::queueAddScript(uint64_t sourceID, Script::Type type, const std::string& filename, bool enabled, const std::unordered_set<Script::Event>& events,
                                          const std::optional<std::unordered_set<uint64_t>>& feedIDs, std::function<void(uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentScriptAdd>(sourceID, type, filename, enabled, events, feedIDs, finishedCallback));
}

void ZapFR::Engine::Agent::queueSetFeedProperties(uint64_t sourceID, uint64_t feedID, const std::string& feedURL, std::optional<uint64_t> refreshIntervalInSeconds,
                                                  std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentFeedSetProperties>(sourceID, feedID, feedURL, refreshIntervalInSeconds, finishedCallback));
}
