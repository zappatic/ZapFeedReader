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

#ifndef ZAPFR_ENGINE_AGENTSOURCECLEARLOGS_H
#define ZAPFR_ENGINE_AGENTSOURCECLEARLOGS_H

#include "ZapFR/AgentRunnable.h"

namespace ZapFR
{
    namespace Engine
    {
        class Feed;

        class AgentSourceClearLogs : public AgentRunnable
        {
          public:
            explicit AgentSourceClearLogs(uint64_t sourceID, std::function<void()> finishedCallback);
            virtual ~AgentSourceClearLogs() = default;

            void payload(Source* source) override;
            Type type() const noexcept override { return Type::SourceClearLogs; }

          private:
            std::function<void()> mFinishedCallback{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_AGENTSOURCECLEARLOGS_H