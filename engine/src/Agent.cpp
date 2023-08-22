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
#include "ZapFR/agents/AgentAddFolder.h"
#include "ZapFR/agents/AgentAddScript.h"
#include "ZapFR/agents/AgentAddScriptFolder.h"
#include "ZapFR/agents/AgentGetFeedFlaggedPosts.h"
#include "ZapFR/agents/AgentGetFeedLogs.h"
#include "ZapFR/agents/AgentGetFeedPosts.h"
#include "ZapFR/agents/AgentGetFeedUnreadCount.h"
#include "ZapFR/agents/AgentGetFolderFlaggedPosts.h"
#include "ZapFR/agents/AgentGetFolderLogs.h"
#include "ZapFR/agents/AgentGetFolderPosts.h"
#include "ZapFR/agents/AgentGetPost.h"
#include "ZapFR/agents/AgentGetScriptFolderFlaggedPosts.h"
#include "ZapFR/agents/AgentGetScriptFolderPosts.h"
#include "ZapFR/agents/AgentGetScriptFolders.h"
#include "ZapFR/agents/AgentGetScripts.h"
#include "ZapFR/agents/AgentGetSourceFlaggedPosts.h"
#include "ZapFR/agents/AgentGetSourceLogs.h"
#include "ZapFR/agents/AgentGetSourcePosts.h"
#include "ZapFR/agents/AgentGetSourceTree.h"
#include "ZapFR/agents/AgentGetUsedFlagColors.h"
#include "ZapFR/agents/AgentMarkFeedRead.h"
#include "ZapFR/agents/AgentMarkFolderRead.h"
#include "ZapFR/agents/AgentMarkPostsFlagged.h"
#include "ZapFR/agents/AgentMarkPostsRead.h"
#include "ZapFR/agents/AgentMarkPostsUnflagged.h"
#include "ZapFR/agents/AgentMarkPostsUnread.h"
#include "ZapFR/agents/AgentMarkSourceRead.h"
#include "ZapFR/agents/AgentMoveFeed.h"
#include "ZapFR/agents/AgentMoveFolder.h"
#include "ZapFR/agents/AgentRefreshFeed.h"
#include "ZapFR/agents/AgentRefreshFolder.h"
#include "ZapFR/agents/AgentRefreshSource.h"
#include "ZapFR/agents/AgentRemoveFeed.h"
#include "ZapFR/agents/AgentRemoveFolder.h"
#include "ZapFR/agents/AgentRemoveScript.h"
#include "ZapFR/agents/AgentRemoveScriptFolder.h"
#include "ZapFR/agents/AgentSubscribeFeed.h"
#include "ZapFR/agents/AgentUpdateScript.h"
#include "ZapFR/agents/AgentUpdateScriptFolder.h"

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

void ZapFR::Engine::Agent::queueRefreshFeed(uint64_t sourceID, uint64_t feedID, std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentRefreshFeed>(sourceID, feedID, finishedCallback));
}

void ZapFR::Engine::Agent::queueRefreshFolder(uint64_t sourceID, uint64_t folderID, std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentRefreshFolder>(sourceID, folderID, finishedCallback));
}

void ZapFR::Engine::Agent::queueRefreshSource(uint64_t sourceID, std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentRefreshSource>(sourceID, finishedCallback));
}

void ZapFR::Engine::Agent::queueSubscribeFeed(uint64_t sourceID, const std::string& url, uint64_t folder, const std::vector<std::string>& newFolderHierarchy,
                                              std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentSubscribeFeed>(sourceID, url, folder, newFolderHierarchy, finishedCallback));
}

void ZapFR::Engine::Agent::queueRemoveFeed(uint64_t sourceID, uint64_t feedID, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentRemoveFeed>(sourceID, feedID, finishedCallback));
}

void ZapFR::Engine::Agent::queueMoveFeed(uint64_t sourceID, uint64_t feedID, uint64_t newFolder, uint64_t newSortOrder, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentMoveFeed>(sourceID, feedID, newFolder, newSortOrder, finishedCallback));
}

void ZapFR::Engine::Agent::queueMoveFolder(uint64_t sourceID, uint64_t folderID, uint64_t newFolder, uint64_t newSortOrder, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentMoveFolder>(sourceID, folderID, newFolder, newSortOrder, finishedCallback));
}

void ZapFR::Engine::Agent::queueRemoveFolder(uint64_t sourceID, uint64_t folder, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentRemoveFolder>(sourceID, folder, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetFeedPosts(uint64_t sourceID, uint64_t feedID, uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                             std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentGetFeedPosts>(sourceID, feedID, perPage, page, showOnlyUnread, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetFolderPosts(uint64_t sourceID, uint64_t folderID, uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                               std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentGetFolderPosts>(sourceID, folderID, perPage, page, showOnlyUnread, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetSourcePosts(uint64_t sourceID, uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                               std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentGetSourcePosts>(sourceID, perPage, page, showOnlyUnread, finishedCallback));
}

void ZapFR::Engine::Agent::queueMarkPostsRead(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs,
                                              std::function<void(uint64_t, const std::vector<std::tuple<uint64_t, uint64_t>>&)> finishedCallback)
{
    enqueue(std::make_unique<AgentMarkPostsRead>(sourceID, feedAndPostIDs, finishedCallback));
}

void ZapFR::Engine::Agent::queueMarkPostsUnread(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs,
                                                std::function<void(uint64_t, const std::vector<std::tuple<uint64_t, uint64_t>>&)> finishedCallback)
{
    enqueue(std::make_unique<AgentMarkPostsUnread>(sourceID, feedAndPostIDs, finishedCallback));
}

void ZapFR::Engine::Agent::queueMarkFeedRead(uint64_t sourceID, uint64_t feedID, std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentMarkFeedRead>(sourceID, feedID, finishedCallback));
}

void ZapFR::Engine::Agent::queueMarkFolderRead(uint64_t sourceID, uint64_t folderID, std::function<void(uint64_t, std::unordered_set<uint64_t>)> finishedCallback)
{
    enqueue(std::make_unique<AgentMarkFolderRead>(sourceID, folderID, finishedCallback));
}

void ZapFR::Engine::Agent::queueMarkSourceRead(uint64_t sourceID, std::function<void(uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentMarkSourceRead>(sourceID, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetPost(uint64_t sourceID, uint64_t feedID, uint64_t postID, std::function<void(std::unique_ptr<Post>)> finishedCallback)
{
    enqueue(std::make_unique<AgentGetPost>(sourceID, feedID, postID, finishedCallback));
}

void ZapFR::Engine::Agent::queueAddFolder(uint64_t sourceID, uint64_t parentFolderID, const std::string& title, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentAddFolder>(sourceID, parentFolderID, title, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetSourceLogs(uint64_t sourceID, uint64_t perPage, uint64_t page,
                                              std::function<void(uint64_t, const std::vector<Log*>&, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentGetSourceLogs>(sourceID, perPage, page, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetFolderLogs(uint64_t sourceID, uint64_t folderID, uint64_t perPage, uint64_t page,
                                              std::function<void(uint64_t, const std::vector<Log*>&, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentGetFolderLogs>(sourceID, folderID, perPage, page, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetFeedLogs(uint64_t sourceID, uint64_t feedID, uint64_t perPage, uint64_t page,
                                            std::function<void(uint64_t, const std::vector<Log*>&, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentGetFeedLogs>(sourceID, feedID, perPage, page, finishedCallback));
}

void ZapFR::Engine::Agent::queueMarkPostsFlagged(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs,
                                                 const std::unordered_set<FlagColor>& flagColors, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentMarkPostsFlagged>(sourceID, feedAndPostIDs, flagColors, finishedCallback));
}

void ZapFR::Engine::Agent::queueMarkPostsUnflagged(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs,
                                                   const std::unordered_set<FlagColor>& flagColors, std::function<void()> finishedCallback)
{
    enqueue(std::make_unique<AgentMarkPostsUnflagged>(sourceID, feedAndPostIDs, flagColors, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetSourceTree(uint64_t sourceID,
                                              std::function<void(uint64_t, const std::string&, const std::vector<Folder*>&, const std::vector<Feed*>& feeds)> finishedCallback)
{
    enqueue(std::make_unique<AgentGetSourceTree>(sourceID, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetFeedUnreadCount(uint64_t sourceID, uint64_t feedID, std::function<void(uint64_t, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentGetFeedUnreadCount>(sourceID, feedID, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetUsedFlagColors(uint64_t sourceID, std::function<void(uint64_t, const std::unordered_set<FlagColor>&)> finishedCallback)
{
    enqueue(std::make_unique<AgentGetUsedFlagColors>(sourceID, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetSourceFlaggedPosts(FlagColor flagColor, uint64_t sourceID, uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                                      std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentGetSourceFlaggedPosts>(flagColor, sourceID, perPage, page, showOnlyUnread, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetFeedFlaggedPosts(FlagColor flagColor, uint64_t sourceID, uint64_t feedID, uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                                    std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentGetFeedFlaggedPosts>(flagColor, sourceID, feedID, perPage, page, showOnlyUnread, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetFolderFlaggedPosts(FlagColor flagColor, uint64_t sourceID, uint64_t folderID, uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                                      std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentGetFolderFlaggedPosts>(flagColor, sourceID, folderID, perPage, page, showOnlyUnread, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetScriptFolders(uint64_t sourceID, std::function<void(uint64_t, const std::vector<ScriptFolder*>&)> finishedCallback)
{
    enqueue(std::make_unique<AgentGetScriptFolders>(sourceID, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetScriptFolderPosts(uint64_t sourceID, uint64_t scriptFolderID, uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                                     std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentGetScriptFolderPosts>(sourceID, scriptFolderID, perPage, page, showOnlyUnread, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetScriptFolderFlaggedPosts(FlagColor flagColor, uint64_t sourceID, uint64_t scriptFolderID, uint64_t perPage, uint64_t page,
                                                            bool showOnlyUnread, std::function<void(uint64_t, const std::vector<Post*>&, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentGetScriptFolderFlaggedPosts>(flagColor, sourceID, scriptFolderID, perPage, page, showOnlyUnread, finishedCallback));
}

void ZapFR::Engine::Agent::queueAddScriptFolder(uint64_t sourceID, const std::string& title, std::function<void(uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentAddScriptFolder>(sourceID, title, finishedCallback));
}

void ZapFR::Engine::Agent::queueUpdateScriptFolder(uint64_t sourceID, uint64_t scriptFolderID, const std::string& title,
                                                   std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentUpdateScriptFolder>(sourceID, scriptFolderID, title, finishedCallback));
}

void ZapFR::Engine::Agent::queueRemoveScriptFolder(uint64_t sourceID, uint64_t scriptFolderID, std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentRemoveScriptFolder>(sourceID, scriptFolderID, finishedCallback));
}

void ZapFR::Engine::Agent::queueGetScripts(uint64_t sourceID, std::function<void(uint64_t, const std::vector<Script*>&)> finishedCallback)
{
    enqueue(std::make_unique<AgentGetScripts>(sourceID, finishedCallback));
}

void ZapFR::Engine::Agent::queueUpdateScript(uint64_t sourceID, uint64_t scriptID, Script::Type type, const std::string& filename, bool enabled,
                                             const std::unordered_set<Script::Event>& events, const std::optional<std::unordered_set<uint64_t>>& feedIDs,
                                             std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentUpdateScript>(sourceID, scriptID, type, filename, enabled, events, feedIDs, finishedCallback));
}

void ZapFR::Engine::Agent::queueRemoveScript(uint64_t sourceID, uint64_t scriptID, std::function<void(uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentRemoveScript>(sourceID, scriptID, finishedCallback));
}

void ZapFR::Engine::Agent::queueAddScript(uint64_t sourceID, Script::Type type, const std::string& filename, bool enabled, const std::unordered_set<Script::Event>& events,
                                          const std::optional<std::unordered_set<uint64_t>>& feedIDs, std::function<void(uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentAddScript>(sourceID, type, filename, enabled, events, feedIDs, finishedCallback));
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
