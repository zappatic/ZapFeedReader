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

#ifndef ZAPFR_ENGINE_AGENTFEEDGET_H
#define ZAPFR_ENGINE_AGENTFEEDGET_H

#include "ZapFR/AgentRunnable.h"
#include "ZapFR/Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class Feed;

        class AgentFeedGet : public AgentRunnable
        {
          public:
            explicit AgentFeedGet(uint64_t sourceID, uint64_t feedID, std::function<void(uint64_t, Feed*)> finishedCallback);
            virtual ~AgentFeedGet() = default;

            void payload(Source* source) override;
            Type type() const noexcept override { return Type::FeedGet; }

          private:
            uint64_t mFeedID{0};
            std::function<void(uint64_t, Feed*)> mFinishedCallback{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_AGENTFEEDGET_H