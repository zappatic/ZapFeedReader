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

#include "ZapFR/agents/source/AgentSourceGetTree.h"
#include "ZapFR/Agent.h"
#include "ZapFR/base/Feed.h"
#include "ZapFR/base/Folder.h"
#include "ZapFR/base/Source.h"

ZapFR::Engine::AgentSourceGetTree::AgentSourceGetTree(uint64_t sourceID,
                                                      std::function<void(Source*, const std::vector<Folder*>&, const std::vector<Feed*>& feeds)> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentSourceGetTree::run()
{
    auto source = Source::getSource(mSourceID);
    if (source.has_value())
    {
        std::vector<Feed*> feedPointers{};
        std::vector<std::unique_ptr<Feed>> feeds;
        std::vector<Folder*> folderPointers{};
        std::vector<std::unique_ptr<Folder>> folders;

        try
        {
            feeds = source.value()->getFeeds(Source::FetchInfo::Icon);
            for (const auto& feed : feeds)
            {
                feedPointers.emplace_back(feed.get());
            }

            folders = source.value()->getFolders(0, Source::FetchInfo::Subfolders);
            for (const auto& folder : folders)
            {
                folderPointers.emplace_back(folder.get());
            }
        }
        CATCH_AND_LOG_EXCEPTION_IN_SOURCE
        mFinishedCallback(source.value().get(), folderPointers, feedPointers);
    }

    mIsDone = true;
}
