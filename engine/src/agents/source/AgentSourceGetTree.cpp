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
    : AgentRunnable(sourceID), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentSourceGetTree::payload(Source* source)
{
    std::vector<Feed*> feedPointers{};
    auto feeds = source->getFeeds(Source::FetchInfo::Icon);
    for (const auto& feed : feeds)
    {
        feedPointers.emplace_back(feed.get());
    }

    std::vector<Folder*> folderPointers{};
    auto folders = source->getFolders(0, Source::FetchInfo::Subfolders);
    for (const auto& folder : folders)
    {
        folderPointers.emplace_back(folder.get());
    }

    mFinishedCallback(source, folderPointers, feedPointers);
}

void ZapFR::Engine::AgentSourceGetTree::onPayloadException(Source* source)
{
    mFinishedCallback(source, {}, {});
}
