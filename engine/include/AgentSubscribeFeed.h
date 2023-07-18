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

#ifndef ZAPFR_ENGINE_AGENTSUBSCRIBEFEED_H
#define ZAPFR_ENGINE_AGENTSUBSCRIBEFEED_H

#include "AgentRunnable.h"
#include "Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class Feed;

        class AgentSubscribeFeed : public AgentRunnable
        {
          public:
            explicit AgentSubscribeFeed(uint64_t sourceID, const std::string& url, const std::string& folderHierarchy, std::function<void()> finishedCallback);
            virtual ~AgentSubscribeFeed() = default;

            void run() override;

          private:
            uint64_t mSourceID{0};
            std::string mURL{0};
            std::string mFolderHierarchy{""};
            std::function<void()> mFinishedCallback{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_AGENTSUBSCRIBEFEED_H