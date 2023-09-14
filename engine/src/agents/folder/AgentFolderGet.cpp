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

#include "ZapFR/agents/folder/AgentFolderGet.h"
#include "ZapFR/base/Folder.h"
#include "ZapFR/base/Source.h"

ZapFR::Engine::AgentFolderGet::AgentFolderGet(uint64_t sourceID, uint64_t folderID, std::function<void(Folder*)> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mFolderID(folderID), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentFolderGet::run()
{
    auto source = Source::getSource(mSourceID);
    if (source.has_value())
    {
        std::optional<std::unique_ptr<Folder>> folder;
        try
        {
            folder = source.value()->getFolder(mFolderID, ZapFR::Engine::Source::FetchInfo::Statistics);
        }
        CATCH_AND_LOG_EXCEPTION_IN_SOURCE
        if (folder.has_value())
        {
            mFinishedCallback(folder.value().get());
        }
    }

    mIsDone = true;
}
