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

#ifndef ZAPFR_ENGINE_AGENTSCRIPTADD_H
#define ZAPFR_ENGINE_AGENTSCRIPTADD_H

#include "ZapFR/AgentRunnable.h"
#include "ZapFR/Global.h"
#include "ZapFR/base/Script.h"

namespace ZapFR
{
    namespace Engine
    {
        class AgentScriptAdd : public AgentRunnable
        {
          public:
            explicit AgentScriptAdd(uint64_t sourceID, Script::Type type, const std::string& filename, bool enabled, const std::unordered_set<Script::Event>& events,
                                    const std::optional<std::unordered_set<uint64_t>>& feedIDs, std::function<void(uint64_t)> finishedCallback);
            virtual ~AgentScriptAdd() = default;

            void run() override;
            Type type() const noexcept override { return Type::ScriptAdd; }

          private:
            uint64_t mSourceID{0};
            Script::Type mType{Script::Type::Lua};
            std::string mFilename{""};
            bool mEnabled{false};
            std::unordered_set<Script::Event> mEvents;
            std::optional<std::unordered_set<uint64_t>> mFeedIDs;
            std::function<void(uint64_t)> mFinishedCallback{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_AGENTADDSCRIPT_H