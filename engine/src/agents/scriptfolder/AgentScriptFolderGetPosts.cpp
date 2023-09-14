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

#include "ZapFR/agents/scriptfolder/AgentScriptFolderGetPosts.h"
#include "ZapFR/base/Post.h"
#include "ZapFR/base/ScriptFolder.h"
#include "ZapFR/base/Source.h"

ZapFR::Engine::AgentScriptFolderGetPosts::AgentScriptFolderGetPosts(
    uint64_t sourceID, uint64_t scriptFolderID, uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter, FlagColor flagColor,
    std::function<void(uint64_t, const std::vector<ZapFR::Engine::Post*>&, uint64_t, uint64_t)> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mScriptFolderID(scriptFolderID), mPerPage(perPage), mPage(page), mShowOnlyUnread(showOnlyUnread), mSearchFilter(searchFilter),
      mFlagColor(flagColor), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentScriptFolderGetPosts::run()
{
    auto source = Source::getSource(mSourceID);
    if (source.has_value())
    {
        std::vector<Post*> postPointers;
        std::vector<std::unique_ptr<Post>> posts;
        uint64_t postCount{0};
        try
        {
            auto scriptFolder = source.value()->getScriptFolder(mScriptFolderID, ZapFR::Engine::Source::FetchInfo::None);
            if (scriptFolder.has_value())
            {
                std::tie(postCount, posts) = scriptFolder.value()->getPosts(mPerPage, mPage, mShowOnlyUnread, mSearchFilter, mFlagColor);
                for (const auto& post : posts)
                {
                    postPointers.emplace_back(post.get());
                }
            }
        }
        CATCH_AND_LOG_EXCEPTION_IN_SOURCE
        mFinishedCallback(source.value()->id(), postPointers, mPage, postCount);
    }

    mIsDone = true;
}
