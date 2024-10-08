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

#include "ZapFR/agents/script/AgentScriptsGet.h"
#include "ZapFR/Agent.h"
#include "ZapFR/base/Script.h"
#include "ZapFR/base/Source.h"

ZapFR::Engine::AgentScriptsGet::AgentScriptsGet(uint64_t sourceID, std::function<void(uint64_t, const std::vector<Script*>&)> finishedCallback)
    : AgentRunnable(sourceID), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentScriptsGet::payload(Source* source)
{
    std::vector<Script*> scriptPointers;
    auto scripts = source->getScripts();
    for (const auto& script : scripts)
    {
        scriptPointers.emplace_back(script.get());
    }
    mFinishedCallback(mSourceID, scriptPointers);
}
