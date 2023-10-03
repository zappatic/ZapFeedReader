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

#include "ZapFR/agents/feed/AgentFeedAdd.h"
#include "ZapFR/Agent.h"
#include "ZapFR/base/Source.h"

ZapFR::Engine::AgentFeedAdd::AgentFeedAdd(uint64_t sourceID, const std::string& url, uint64_t folder, std::function<void(uint64_t, ZapFR::Engine::Feed*)> finishedCallback)
    : AgentRunnable(sourceID), mURL(url), mFolderID(folder), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentFeedAdd::payload(Source* source)
{
    const auto& feed = source->addFeed(mURL, mFolderID);
    if (feed.has_value())
    {
        mFinishedCallback(mSourceID, feed.value().get());
    }
}
