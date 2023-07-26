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

#include "AgentSubscribeFeed.h"
#include "Feed.h"
#include "Source.h"

ZapFR::Engine::AgentSubscribeFeed::AgentSubscribeFeed(uint64_t sourceID, const std::string& url, uint64_t folder, std::function<void()> finishedCallback)
    : AgentRunnable(), mSourceID(sourceID), mURL(url), mFolderID(folder), mFinishedCallback(finishedCallback)
{
}

void ZapFR::Engine::AgentSubscribeFeed::run()
{
    auto source = ZapFR::Engine::Source::getSource(mSourceID);
    if (source.has_value())
    {
        try
        {
            source.value()->addFeed(mURL, mFolderID);
            mFinishedCallback();
        }
        catch (Poco::Exception& e)
        {
            std::cout << "Poco Exception: " << e.what() << "\n" << e.displayText() << "\n";
        }
    }

    mIsDone = true;
}
