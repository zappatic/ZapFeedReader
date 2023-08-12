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

#ifndef ZAPFR_ENGINE_AGENTGETFOLDERFLAGGEDPOSTS_H
#define ZAPFR_ENGINE_AGENTGETFOLDERFLAGGEDPOSTS_H

#include "ZapFR/AgentRunnable.h"
#include "ZapFR/Global.h"
#include "ZapFR/Post.h"

namespace ZapFR
{
    namespace Engine
    {
        class AgentGetFolderFlaggedPosts : public AgentRunnable
        {
          public:
            explicit AgentGetFolderFlaggedPosts(FlagColor flagColor, uint64_t sourceID, uint64_t folderID, uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                                std::function<void(uint64_t, const std::vector<ZapFR::Engine::Post*>&, uint64_t, uint64_t)> finishedCallback);
            virtual ~AgentGetFolderFlaggedPosts() = default;

            void run() override;

          private:
            FlagColor mFlagColor{FlagColor::Gray};
            uint64_t mSourceID{0};
            uint64_t mFolderID{0};
            uint64_t mPerPage{0};
            uint64_t mPage{0};
            bool mShowOnlyUnread{false};
            std::function<void(uint64_t, const std::vector<ZapFR::Engine::Post*>&, uint64_t, uint64_t)> mFinishedCallback{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_AGENTGETFOLDERFLAGGEDPOSTS_H