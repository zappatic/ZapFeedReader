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
#include "ZapFR/agents/AgentMonitorFeedRefreshCompletion.h"
#include "ZapFR/agents/feed/AgentFeedAdd.h"
#include "ZapFR/agents/feed/AgentFeedGet.h"
#include "ZapFR/agents/feed/AgentFeedGetLogs.h"
#include "ZapFR/agents/feed/AgentFeedGetPosts.h"
#include "ZapFR/agents/feed/AgentFeedGetUnreadCount.h"
#include "ZapFR/agents/feed/AgentFeedMarkRead.h"
#include "ZapFR/agents/feed/AgentFeedMove.h"
#include "ZapFR/agents/feed/AgentFeedRefresh.h"
#include "ZapFR/agents/feed/AgentFeedRemove.h"
#include "ZapFR/agents/feed/AgentFeedUpdate.h"
#include "ZapFR/agents/folder/AgentFolderAdd.h"
#include "ZapFR/agents/folder/AgentFolderGet.h"
#include "ZapFR/agents/folder/AgentFolderGetLogs.h"
#include "ZapFR/agents/folder/AgentFolderGetPosts.h"
#include "ZapFR/agents/folder/AgentFolderMarkRead.h"
#include "ZapFR/agents/folder/AgentFolderMove.h"
#include "ZapFR/agents/folder/AgentFolderRefresh.h"
#include "ZapFR/agents/folder/AgentFolderRemove.h"
#include "ZapFR/agents/folder/AgentFolderUpdate.h"
#include "ZapFR/agents/folder/AgentFoldersGet.h"
#include "ZapFR/agents/post/AgentPostGet.h"
#include "ZapFR/agents/post/AgentPostsMarkFlagged.h"
#include "ZapFR/agents/post/AgentPostsMarkRead.h"
#include "ZapFR/agents/post/AgentPostsMarkUnflagged.h"
#include "ZapFR/agents/post/AgentPostsMarkUnread.h"
#include "ZapFR/agents/script/AgentScriptAdd.h"
#include "ZapFR/agents/script/AgentScriptRemove.h"
#include "ZapFR/agents/script/AgentScriptUpdate.h"
#include "ZapFR/agents/script/AgentScriptsGet.h"
#include "ZapFR/agents/scriptfolder/AgentScriptFolderAdd.h"
#include "ZapFR/agents/scriptfolder/AgentScriptFolderAssignPosts.h"
#include "ZapFR/agents/scriptfolder/AgentScriptFolderGetPosts.h"
#include "ZapFR/agents/scriptfolder/AgentScriptFolderRemove.h"
#include "ZapFR/agents/scriptfolder/AgentScriptFolderRemovePosts.h"
#include "ZapFR/agents/scriptfolder/AgentScriptFolderUpdate.h"
#include "ZapFR/agents/scriptfolder/AgentScriptFoldersGet.h"
#include "ZapFR/agents/source/AgentSourceGet.h"
#include "ZapFR/agents/source/AgentSourceGetLogs.h"
#include "ZapFR/agents/source/AgentSourceGetPosts.h"
#include "ZapFR/agents/source/AgentSourceGetTree.h"
#include "ZapFR/agents/source/AgentSourceGetUnreadCount.h"
#include "ZapFR/agents/source/AgentSourceGetUsedFlagColors.h"
#include "ZapFR/agents/source/AgentSourceImportOPML.h"
#include "ZapFR/agents/source/AgentSourceMarkRead.h"
#include "ZapFR/agents/source/AgentSourceRefresh.h"
#include "ZapFR/base/Feed.h"
#include "ZapFR/base/Post.h"
#include "ZapFR/base/Source.h"

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
    {
        std::lock_guard<std::mutex> lock(msMutex);
        for (const auto& runningAgent : mRunningAgents)
        {
            runningAgent->setShouldAbort(true);
        }
    }

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

uint64_t ZapFR::Engine::Agent::totalCountOfType(AgentRunnable::Type t) const
{
    uint64_t amount{0};

    std::lock_guard<std::mutex> lock(msMutex);
    for (const auto& runningAgent : mRunningAgents)
    {
        if (runningAgent->type() == t && !runningAgent->isDone())
        {
            amount++;
        }
    }

    for (const auto& queuedAgent : mQueue)
    {
        if (queuedAgent->type() == t && !queuedAgent->isDone())
        {
            amount++;
        }
    }

    return amount;
}

void ZapFR::Engine::Agent::broadcastError(uint64_t sourceID, const std::string& errorMessage) const
{
    if (mErrorCallback.has_value())
    {
        mErrorCallback.value()(sourceID, errorMessage);
    }
}

void ZapFR::Engine::Agent::queueRefreshFeed(uint64_t sourceID, uint64_t feedID, std::function<void(uint64_t, Feed*)> finishedCallback)
{
    enqueue(std::make_unique<AgentFeedRefresh>(sourceID, feedID, finishedCallback));
}

void ZapFR::Engine::Agent::queueRefreshFolder(uint64_t sourceID, uint64_t folderID, std::function<void(uint64_t, Feed*)> finishedCallback)
{
    enqueue(std::make_unique<AgentFolderRefresh>(sourceID, folderID, finishedCallback));
}

void ZapFR::Engine::Agent::queueRefreshSource(uint64_t sourceID, std::function<void(uint64_t, Feed*)> finishedCallback)
{
    enqueue(std::make_unique<AgentSourceRefresh>(sourceID, finishedCallback));
}

void ZapFR::Engine::Agent::queueAddFeed(uint64_t sourceID, const std::string& url, uint64_t folder, std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentFeedAdd>(sourceID, url, folder, finishedCallback));
}

void ZapFR::Engine::Agent::queueImportOPML(uint64_t sourceID, const std::string& opml, uint64_t parentFolderID, std::function<void()> opmlParsedCallback,
                                           std::function<void(uint64_t, Feed*)> feedRefreshedCallback)
{
    enqueue(std::make_unique<AgentSourceImportOPML>(sourceID, opml, parentFolderID, opmlParsedCallback, feedRefreshedCallback));
}

void ZapFR::Engine::Agent::queueGetFeed(uint64_t sourceID, uint64_t feedID, std::function<void(uint64_t, Feed*)> finishedCallback)
{
    enqueue(std::make_unique<AgentFeedGet>(sourceID, feedID, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetFolder(uint64_t sourceID, uint64_t folderID, std::function<void(Folder*)> finishedCallback)
{
    enqueue(std::make_unique<AgentFolderGet>(sourceID, folderID, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetFolders(uint64_t sourceID, uint64_t folderID, std::function<void(const std::vector<Folder*>&)> finishedCallback)
{
    enqueue(std::make_unique<AgentFoldersGet>(sourceID, folderID, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetSource(uint64_t sourceID, std::function<void(Source*)> finishedCallback)
{
    enqueue(std::make_unique<AgentSourceGet>(sourceID, finishedCallback));
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

void ZapFR::Engine::Agent::queueUpdateFolder(uint64_t sourceID, uint64_t folder, const std::string& newTitle,
                                             std::function<void(uint64_t, uint64_t, const std::string&)> finishedCallback)
{
    enqueue(std::make_unique<AgentFolderUpdate>(sourceID, folder, newTitle, finishedCallback));
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

void ZapFR::Engine::Agent::queueGetSourceTree(uint64_t sourceID, std::function<void(Source*, const std::vector<Folder*>&, const std::vector<Feed*>& feeds)> finishedCallback)
{
    enqueue(std::make_unique<AgentSourceGetTree>(sourceID, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetSourceUnreadCount(uint64_t sourceID, std::function<void(uint64_t, const std::unordered_map<uint64_t, uint64_t>&)> finishedCallback)
{
    enqueue(std::make_unique<AgentSourceGetUnreadCount>(sourceID, finishedCallback));
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

void ZapFR::Engine::Agent::queueAddScriptFolder(uint64_t sourceID, const std::string& title, bool showTotal, bool showUnread, std::function<void(uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentScriptFolderAdd>(sourceID, title, showTotal, showUnread, finishedCallback));
}

void ZapFR::Engine::Agent::queueUpdateScriptFolder(uint64_t sourceID, uint64_t scriptFolderID, const std::string& title, bool showTotal, bool showUnread,
                                                   std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentScriptFolderUpdate>(sourceID, scriptFolderID, title, showTotal, showUnread, finishedCallback));
}

void ZapFR::Engine::Agent::queueRemoveScriptFolder(uint64_t sourceID, uint64_t scriptFolderID, std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentScriptFolderRemove>(sourceID, scriptFolderID, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetScripts(uint64_t sourceID, std::function<void(uint64_t, const std::vector<Script*>&)> finishedCallback)
{
    enqueue(std::make_unique<AgentScriptsGet>(sourceID, finishedCallback));
}

void ZapFR::Engine::Agent::queueUpdateScript(uint64_t sourceID, uint64_t scriptID, Script::Type type, const std::string& title, bool enabled,
                                             const std::unordered_set<Script::Event>& events, const std::optional<std::unordered_set<uint64_t>>& feedIDs,
                                             const std::string& script, std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentScriptUpdate>(sourceID, scriptID, type, title, enabled, events, feedIDs, script, finishedCallback));
}

void ZapFR::Engine::Agent::queueRemoveScript(uint64_t sourceID, uint64_t scriptID, std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentScriptRemove>(sourceID, scriptID, finishedCallback));
}

void ZapFR::Engine::Agent::queueAddScript(uint64_t sourceID, Script::Type type, const std::string& title, bool enabled, const std::unordered_set<Script::Event>& events,
                                          const std::optional<std::unordered_set<uint64_t>>& feedIDs, const std::string& script,
                                          std::function<void(uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentScriptAdd>(sourceID, type, title, enabled, events, feedIDs, script, finishedCallback));
}

void ZapFR::Engine::Agent::queueUpdateFeed(uint64_t sourceID, uint64_t feedID, const std::string& feedURL, std::optional<uint64_t> refreshIntervalInSeconds,
                                           std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentFeedUpdate>(sourceID, feedID, feedURL, refreshIntervalInSeconds, finishedCallback));
}

void ZapFR::Engine::Agent::queueMonitorFeedRefreshCompletion(std::function<void()> finishedCallback)
{
    // even though this might end up in the queue (if there's a load of refresh agents before it, filling up the threadpool),
    // that doesn't really matter, as the feed-refreshing would have to finish first anyway, and the monitor thread wouldn't
    // have been able to complete before that.
    enqueue(std::make_unique<AgentMonitorFeedRefreshCompletion>(this, finishedCallback));
}
