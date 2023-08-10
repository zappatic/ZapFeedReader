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

#ifndef ZAPFR_ENGINE_AGENTGETFEEDUNREADCOUNT_H
#define ZAPFR_ENGINE_AGENTGETFEEDUNREADCOUNT_H

#include "AgentRunnable.h"
#include "Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class Feed;

        class AgentGetFeedUnreadCount : public AgentRunnable
        {
          public:
            explicit AgentGetFeedUnreadCount(uint64_t sourceID, uint64_t feedID, std::function<void(uint64_t, uint64_t, uint64_t)> finishedCallback);
            virtual ~AgentGetFeedUnreadCount() = default;

            void run() override;

          private:
            uint64_t mSourceID{0};
            uint64_t mFeedID{0};
            std::function<void(uint64_t, uint64_t, uint64_t)> mFinishedCallback{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_AGENTGETFEEDUNREADCOUNT_H