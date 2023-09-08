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

#ifndef ZAPFR_ENGINE_AGENTFEEDGETLOGS_H
#define ZAPFR_ENGINE_AGENTFEEDGETLOGS_H

#include "ZapFR/AgentRunnable.h"
#include "ZapFR/Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class Log;

        class AgentFeedGetLogs : public AgentRunnable
        {
          public:
            explicit AgentFeedGetLogs(uint64_t sourceID, uint64_t feedID, uint64_t perPage, uint64_t page,
                                      std::function<void(uint64_t, const std::vector<Log*>&, uint64_t, uint64_t)> finishedCallback);
            virtual ~AgentFeedGetLogs() = default;

            void run() override;
            Type type() const noexcept override { return Type::FeedGetLogs; }

          private:
            uint64_t mSourceID{0};
            uint64_t mFeedID{0};
            uint64_t mPerPage{0};
            uint64_t mPage{0};
            std::function<void(uint64_t, const std::vector<Log*>&, uint64_t, uint64_t)> mFinishedCallback{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_AGENTFEEDGETLOGS_H