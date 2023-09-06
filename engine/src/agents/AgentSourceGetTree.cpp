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

#include "ZapFR/agents/AgentSourceGetTree.h"
#include "ZapFR/Feed.h"
#include "ZapFR/Folder.h"
#include "ZapFR/Source.h"

ZapFR::Engine::AgentSourceGetTree::AgentSourceGetTree(uint64_t sourceID,
                                                      std::function<void(Source*, const std::vector<Folder*>&, const std::vector<Feed*>& feeds)> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentSourceGetTree::run()
{
    std::function<void(Folder*)> loadFolder;
    loadFolder = [&](Folder* folder)
    {
        for (const auto& subfolder : folder->subfolders())
        {
            loadFolder(subfolder.get());
        }
    };

    auto source = Source::getSource(mSourceID);
    if (source.has_value())
    {
        std::vector<Feed*> feedPointers{};
        auto feeds = source.value()->getFeeds(Source::FetchInfo::Icon);
        for (const auto& feed : feeds)
        {
            feedPointers.emplace_back(feed.get());
        }

        std::vector<Folder*> folderPointers{};
        auto folders = source.value()->getFolders(0);
        // ensure all subfolders are fetched within this thread
        for (const auto& folder : folders)
        {
            loadFolder(folder.get());
            folderPointers.emplace_back(folder.get());
        }

        mFinishedCallback(source.value().get(), folderPointers, feedPointers);
    }

    mIsDone = true;
}
