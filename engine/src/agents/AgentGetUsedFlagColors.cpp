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

#include "agents/AgentGetUsedFlagColors.h"
#include "Feed.h"
#include "Folder.h"
#include "Source.h"

ZapFR::Engine::AgentGetUsedFlagColors::AgentGetUsedFlagColors(uint64_t sourceID, std::function<void(uint64_t, const std::unordered_set<FlagColor>&)> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentGetUsedFlagColors::run()
{
    auto source = Source::getSource(mSourceID);
    if (source.has_value())
    {
        mFinishedCallback(source.value()->id(), source.value()->getUsedFlagColors());
    }

    mIsDone = true;
}
