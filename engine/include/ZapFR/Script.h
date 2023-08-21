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

#ifndef ZAPFR_ENGINE_SCRIPT_H
#define ZAPFR_ENGINE_SCRIPT_H

#include "Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class Script
        {
          public:
            enum class Type
            {
                Lua,
            };

            enum class Event
            {
                NewPost,
            };

            explicit Script(uint64_t id);
            virtual ~Script() = default;

            uint64_t id() const noexcept { return mID; }
            Type type() const noexcept { return mType; }
            std::string filename() const noexcept { return mFilename; }
            bool isEnabled() const noexcept { return mIsEnabled; }
            std::unordered_set<Event> runOnEvents() const noexcept { return mRunOnEvents; }
            std::optional<std::unordered_set<uint64_t>> runOnFeedIDs() const noexcept { return mRunOnFeedIDs; }

            void setType(Type t) { mType = t; }
            void setFilename(const std::string& f) { mFilename = f; }
            void setIsEnabled(bool b) { mIsEnabled = b; }
            void parseRunOnEvents(const std::string& str);
            void parseRunOnFeedIDs(const std::string& str);

            std::string scriptContents() const;
            void update(Type type, const std::string& filename, bool enabled, const std::unordered_set<Event>& events,
                        const std::optional<std::unordered_set<uint64_t>>& feedIDs);

            static void setScriptDir(const std::string& scriptDir);

          protected:
            uint64_t mID{0};
            Type mType{Type::Lua};
            std::string mFilename{""};
            bool mIsEnabled{false};
            std::unordered_set<Event> mRunOnEvents{};
            std::optional<std::unordered_set<uint64_t>> mRunOnFeedIDs{};

            static std::string msScriptDir;

          private:
            static std::string msEventNewPostIdentifier;
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_SCRIPT_H