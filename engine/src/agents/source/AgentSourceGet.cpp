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

#include "ZapFR/agents/source/AgentSourceGet.h"
#include "ZapFR/Agent.h"
#include "ZapFR/base/Source.h"

ZapFR::Engine::AgentSourceGet::AgentSourceGet(uint64_t sourceID, std::function<void(Source*)> finishedCallback) : AgentRunnable(sourceID), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentSourceGet::payload(Source* source)
{
    source->fetchStatistics();
    mFinishedCallback(source);
}

void ZapFR::Engine::AgentSourceGet::onPayloadException(Source* source)
{
    mFinishedCallback(source);
}
