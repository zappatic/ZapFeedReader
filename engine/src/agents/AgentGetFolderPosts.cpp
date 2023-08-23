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

#include "ZapFR/agents/AgentGetFolderPosts.h"
#include "ZapFR/Folder.h"
#include "ZapFR/Source.h"

ZapFR::Engine::AgentGetFolderPosts::AgentGetFolderPosts(uint64_t sourceID, uint64_t folderID, uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                                        const std::string& searchFilter, FlagColor flagColor,
                                                        std::function<void(uint64_t, const std::vector<ZapFR::Engine::Post*>&, uint64_t, uint64_t)> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mFolderID(folderID), mPerPage(perPage), mPage(page), mShowOnlyUnread(showOnlyUnread), mSearchFilter(searchFilter),
      mFlagColor(flagColor), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentGetFolderPosts::run()
{
    auto source = Source::getSource(mSourceID);
    if (source.has_value())
    {
        auto folder = source.value()->getFolder(mFolderID);
        if (folder.has_value())
        {
            auto posts = folder.value()->getPosts(mPerPage, mPage, mShowOnlyUnread, mSearchFilter, mFlagColor);
            std::vector<Post*> postPointers;
            for (const auto& post : posts)
            {
                postPointers.emplace_back(post.get());
            }

            mFinishedCallback(source.value()->id(), postPointers, mPage, folder.value()->getTotalPostCount(mShowOnlyUnread, mSearchFilter, mFlagColor));
        }
    }

    mIsDone = true;
}
