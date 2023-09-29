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

#ifndef ZAPFR_ENGINE_AGENTPOSTSMARKREAD_H
#define ZAPFR_ENGINE_AGENTPOSTSMARKREAD_H

#include "ZapFR/AgentRunnable.h"

namespace ZapFR
{
    namespace Engine
    {
        class AgentPostsMarkRead : public AgentRunnable
        {
          public:
            explicit AgentPostsMarkRead(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs,
                                        std::function<void(uint64_t, const std::vector<std::tuple<uint64_t, uint64_t>>&)> finishedCallback);
            virtual ~AgentPostsMarkRead() = default;

            void payload(Source* source) override;
            Type type() const noexcept override { return Type::PostsMarkRead; }

          private:
            std::vector<std::tuple<uint64_t, uint64_t>> mFeedAndPostIDs{};
            std::function<void(uint64_t, const std::vector<std::tuple<uint64_t, uint64_t>>&)> mFinishedCallback{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_AGENTMARKPOSTSREAD_H