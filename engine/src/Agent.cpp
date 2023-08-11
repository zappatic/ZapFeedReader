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
#include "Feed.h"
#include "Post.h"
#include "Source.h"
#include "agents/AgentAddFolder.h"
#include "agents/AgentGetFeedFlaggedPosts.h"
#include "agents/AgentGetFeedLogs.h"
#include "agents/AgentGetFeedPosts.h"
#include "agents/AgentGetFeedUnreadCount.h"
#include "agents/AgentGetFolderFlaggedPosts.h"
#include "agents/AgentGetFolderLogs.h"
#include "agents/AgentGetFolderPosts.h"
#include "agents/AgentGetPost.h"
#include "agents/AgentGetScriptFolderPosts.h"
#include "agents/AgentGetScriptFolders.h"
#include "agents/AgentGetSourceFlaggedPosts.h"
#include "agents/AgentGetSourceLogs.h"
#include "agents/AgentGetSourcePosts.h"
#include "agents/AgentGetSourceTree.h"
#include "agents/AgentGetUsedFlagColors.h"
#include "agents/AgentMarkFeedRead.h"
#include "agents/AgentMarkFolderRead.h"
#include "agents/AgentMarkPostFlagged.h"
#include "agents/AgentMarkPostRead.h"
#include "agents/AgentMarkPostUnflagged.h"
#include "agents/AgentMarkPostsUnread.h"
#include "agents/AgentMarkSourceRead.h"
#include "agents/AgentMoveFeed.h"
#include "agents/AgentMoveFolder.h"
#include "agents/AgentRefreshFeed.h"
#include "agents/AgentRefreshFolder.h"
#include "agents/AgentRefreshSource.h"
#include "agents/AgentRemoveFeed.h"
#include "agents/AgentRemoveFolder.h"
#include "agents/AgentSubscribeFeed.h"

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

void ZapFR::Engine::Agent::queueMarkPostRead(uint64_t sourceID, uint64_t feedID, uint64_t postID, std::function<void(uint64_t, uint64_t, uint64_t)> finishedCallback)
{
    enqueue(std::make_unique<AgentMarkPostRead>(sourceID, feedID, postID, finishedCallback));
}

void ZapFR::Engine::Agent::queueMarkPostsUnread(uint64_t sourceID, std::vector<std::tuple<uint64_t, uint64_t>> feedAndPostIDs,
                                                std::function<void(uint64_t, std::vector<std::tuple<uint64_t, uint64_t>>)> finishedCallback)
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

void ZapFR::Engine::Agent::queueMarkPostFlagged(uint64_t sourceID, uint64_t feedID, uint64_t postID, FlagColor flagColor,
                                                std::function<void(uint64_t, uint64_t, uint64_t, ZapFR::Engine::FlagColor)> finishedCallback)
{
    enqueue(std::make_unique<AgentMarkPostFlagged>(sourceID, feedID, postID, flagColor, finishedCallback));
}

void ZapFR::Engine::Agent::queueMarkPostUnflagged(uint64_t sourceID, uint64_t feedID, uint64_t postID, FlagColor flagColor,
                                                  std::function<void(uint64_t, uint64_t, uint64_t, ZapFR::Engine::FlagColor)> finishedCallback)
{
    enqueue(std::make_unique<AgentMarkPostUnflagged>(sourceID, feedID, postID, flagColor, finishedCallback));
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
