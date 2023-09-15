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

#include "ZapFR/agents/source/AgentSourceImportOPML.h"
#include "ZapFR/Agent.h"
#include "ZapFR/base/Feed.h"
#include "ZapFR/base/Folder.h"
#include "ZapFR/base/Source.h"

ZapFR::Engine::AgentSourceImportOPML::AgentSourceImportOPML(uint64_t sourceID, const std::string& opml, uint64_t parentFolderID, std::function<void()> opmlParsedCallback,
                                                            std::function<void(uint64_t, Feed*)> feedRefreshedCallback)
    : AgentRunnable(sourceID), mOPML(opml), mParentFolderID(parentFolderID), mOPMLParsedCallback(opmlParsedCallback), mFeedRefreshedCallback(feedRefreshedCallback)
{
}

void ZapFR::Engine::AgentSourceImportOPML::payload(Source* source)
{
    auto feedIDs = source->importOPML(mOPML, mParentFolderID);
    mOPMLParsedCallback();
    for (const auto& feedID : feedIDs)
    {
        // We just create agent threads here so the refreshing can be done concurrently
        // The callback will be called for each feed that is refreshed (with the feed ID as the parameter)
        Agent::getInstance()->queueRefreshFeed(mSourceID, feedID, mFeedRefreshedCallback);
    }
}
