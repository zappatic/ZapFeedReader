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

#ifndef ZAPFR_ENGINE_AGENTSCRIPTSGET_H
#define ZAPFR_ENGINE_AGENTSCRIPTSGET_H

#include "ZapFR/AgentRunnable.h"

namespace ZapFR
{
    namespace Engine
    {
        class Script;

        class AgentScriptsGet : public AgentRunnable
        {
          public:
            explicit AgentScriptsGet(uint64_t sourceID, std::function<void(uint64_t, const std::vector<Script*>&)> finishedCallback);
            virtual ~AgentScriptsGet() = default;

            void payload(Source* source) override;
            Type type() const noexcept override { return Type::ScriptsGet; }

          private:
            std::function<void(uint64_t, const std::vector<Script*>&)> mFinishedCallback{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_AGENTSCRIPTSGET_H