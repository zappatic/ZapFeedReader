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

#include "ZapFR/agents/script/AgentScriptAdd.h"
#include "ZapFR/base/Source.h"

ZapFR::Engine::AgentScriptAdd::AgentScriptAdd(uint64_t sourceID, Script::Type type, const std::string& title, bool enabled, const std::unordered_set<Script::Event>& events,
                                              const std::optional<std::unordered_set<uint64_t>>& feedIDs, const std::string& script,
                                              std::function<void(uint64_t)> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mType(type), mTitle(title), mEnabled(enabled), mEvents(events), mFeedIDs(feedIDs), mScript(script),
      mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentScriptAdd::run()
{
    auto source = Source::getSource(mSourceID);
    if (source.has_value())
    {
        source.value()->addScript(mType, mTitle, mEnabled, mEvents, mFeedIDs, mScript);
        mFinishedCallback(mSourceID);
    }

    mIsDone = true;
}
