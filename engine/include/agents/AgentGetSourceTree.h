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

#ifndef ZAPFR_ENGINE_AGENTGETSOURCETREE_H
#define ZAPFR_ENGINE_AGENTGETSOURCETREE_H

#include "AgentRunnable.h"
#include "Global.h"
#include "Post.h"

namespace ZapFR
{
    namespace Engine
    {
        class Feed;
        class Folder;

        class AgentGetSourceTree : public AgentRunnable
        {
          public:
            explicit AgentGetSourceTree(uint64_t sourceID,
                                        std::function<void(uint64_t, const std::string&, const std::vector<Folder*>&, const std::vector<Feed*>& feeds)> finishedCallback);
            virtual ~AgentGetSourceTree() = default;

            void run() override;

          private:
            uint64_t mSourceID{0};
            std::function<void(uint64_t, const std::string&, const std::vector<Folder*>&, const std::vector<Feed*>& feeds)> mFinishedCallback{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_AGENTGETSOURCETREE_H