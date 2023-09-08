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

#ifndef ZAPFR_ENGINE_AGENTPOSTGET_H
#define ZAPFR_ENGINE_AGENTPOSTGET_H

#include "ZapFR/AgentRunnable.h"
#include "ZapFR/Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class Post;

        class AgentPostGet : public AgentRunnable
        {
          public:
            explicit AgentPostGet(uint64_t sourceID, uint64_t feedID, uint64_t postID, std::function<void(std::unique_ptr<ZapFR::Engine::Post>)> finishedCallback);
            virtual ~AgentPostGet() = default;

            void run() override;
            Type type() const noexcept override { return Type::PostGet; }

          private:
            uint64_t mSourceID{0};
            uint64_t mFeedID{0};
            uint64_t mPostID{0};
            std::function<void(std::unique_ptr<Post>)> mFinishedCallback{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_AGENTPOSTGET_H