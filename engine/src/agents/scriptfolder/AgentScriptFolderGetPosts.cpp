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
#include "ZapFR/Agent.h"
#include "ZapFR/base/Post.h"
#include "ZapFR/base/ScriptFolder.h"
#include "ZapFR/base/Source.h"

ZapFR::Engine::AgentScriptFolderGetPosts::AgentScriptFolderGetPosts(
    uint64_t sourceID, uint64_t scriptFolderID, uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter, uint64_t categoryFilterID,
    FlagColor flagColor, std::function<void(uint64_t, const std::vector<ZapFR::Engine::Post*>&, uint64_t, uint64_t, const std::vector<ThumbnailData>&)> finishedCallback)
    : AgentRunnable(sourceID), mScriptFolderID(scriptFolderID), mPerPage(perPage), mPage(page), mShowOnlyUnread(showOnlyUnread), mSearchFilter(searchFilter),
      mCategoryFilterID(categoryFilterID), mFlagColor(flagColor), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentScriptFolderGetPosts::payload(Source* source)
{
    auto scriptFolder = source->getScriptFolder(mScriptFolderID, ZapFR::Engine::Source::FetchInfo::UnreadThumbnailData);
    if (scriptFolder.has_value())
    {
        auto [postCount, posts] = scriptFolder.value()->getPosts(mPerPage, mPage, mShowOnlyUnread, mSearchFilter, mCategoryFilterID, mFlagColor);
        std::vector<Post*> postPointers;
        for (const auto& post : posts)
        {
            postPointers.emplace_back(post.get());
        }
        mFinishedCallback(source->id(), postPointers, mPage, postCount, scriptFolder.value()->thumbnailData());
    }
}
