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

#include <thread>

#include "ZapFR/Agent.h"
#include "ZapFR/agents/AgentMonitorSourceReloadCompletion.h"
#include "ZapFR/base/Source.h"

using namespace std::chrono_literals;

ZapFR::Engine::AgentMonitorSourceReloadCompletion::AgentMonitorSourceReloadCompletion(Agent* agentManager, std::function<void()> finishedCallback)
    : AgentRunnable(0), mAgentManager(agentManager), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentMonitorSourceReloadCompletion::run()
{
    while (true && !mShouldAbort)
    {
        auto getSourceTreeThreadCount = mAgentManager->totalCountOfType(Type::SourceGetTree);
        if (getSourceTreeThreadCount == 0)
        {
            break;
        }
        std::this_thread::sleep_for(500ms);
    }

    mFinishedCallback();
}

void ZapFR::Engine::AgentMonitorSourceReloadCompletion::payload([[maybe_unused]] Source* source)
{
}
