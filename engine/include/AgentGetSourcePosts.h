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

#ifndef ZAPFR_ENGINE_AGENTGETSOURCEPOSTS_H
#define ZAPFR_ENGINE_AGENTGETSOURCEPOSTS_H

#include "AgentRunnable.h"
#include "Global.h"
#include "Post.h"

namespace ZapFR
{
    namespace Engine
    {
        class Feed;

        class AgentGetSourcePosts : public AgentRunnable
        {
          public:
            explicit AgentGetSourcePosts(uint64_t sourceID, uint64_t perPage, uint64_t page,
                                         std::function<void(uint64_t, const std::vector<ZapFR::Engine::Post*>&)> finishedCallback);
            virtual ~AgentGetSourcePosts() = default;

            void run() override;

          private:
            uint64_t mSourceID{0};
            uint64_t mPerPage{0};
            uint64_t mPage{0};
            std::function<void(uint64_t, const std::vector<ZapFR::Engine::Post*>&)> mFinishedCallback{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_AGENTGETSOURCEPOSTS_H