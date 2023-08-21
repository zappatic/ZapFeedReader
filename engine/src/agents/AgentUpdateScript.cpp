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

#include "ZapFR/agents/AgentUpdateScript.h"
#include "ZapFR/Source.h"

ZapFR::Engine::AgentUpdateScript::AgentUpdateScript(uint64_t sourceID, uint64_t scriptID, Script::Type type, const std::string& filename, bool enabled,
                                                    const std::unordered_set<Script::Event>& events, const std::optional<std::unordered_set<uint64_t>>& feedIDs,
                                                    std::function<void(uint64_t, uint64_t)> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mScriptID(scriptID), mType(type), mFilename(filename), mEnabled(enabled), mEvents(events), mFeedIDs(feedIDs),
      mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentUpdateScript::run()
{
    auto source = Source::getSource(mSourceID);
    if (source.has_value())
    {
        auto script = source.value()->getScript(mScriptID);
        if (script.has_value())
        {
            script.value()->update(mType, mFilename, mEnabled, mEvents, mFeedIDs);
            mFinishedCallback(mSourceID, mScriptID);
        }
    }

    mIsDone = true;
}
