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

#ifndef ZAPFR_ENGINE_AGENTUPDATESCRIPT_H
#define ZAPFR_ENGINE_AGENTUPDATESCRIPT_H

#include "ZapFR/AgentRunnable.h"
#include "ZapFR/Global.h"
#include "ZapFR/Script.h"

namespace ZapFR
{
    namespace Engine
    {
        class AgentUpdateScript : public AgentRunnable
        {
          public:
            explicit AgentUpdateScript(uint64_t sourceID, uint64_t scriptID, Script::Type type, const std::string& filename, bool enabled,
                                       const std::unordered_set<Script::Event>& events, const std::optional<std::unordered_set<uint64_t>>& feedIDs,
                                       std::function<void(uint64_t, uint64_t)> finishedCallback);
            virtual ~AgentUpdateScript() = default;

            void run() override;

          private:
            uint64_t mSourceID{0};
            uint64_t mScriptID{0};
            Script::Type mType{Script::Type::Lua};
            std::string mFilename{""};
            bool mEnabled{false};
            std::unordered_set<Script::Event> mEvents;
            std::optional<std::unordered_set<uint64_t>> mFeedIDs;
            std::function<void(uint64_t, uint64_t)> mFinishedCallback{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_AGENTUPDATESCRIPT_H