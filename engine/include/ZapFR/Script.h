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
                UpdatePost,
            };

            explicit Script(uint64_t id) : mID(id) {}
            virtual ~Script() = default;

            uint64_t id() const noexcept { return mID; }
            Type type() const noexcept { return mType; }
            std::string filename() const noexcept { return mFilename; }
            bool isEnabled() const noexcept { return mIsEnabled; }
            std::unordered_set<Event> runOnEvents() const noexcept { return mRunOnEvents; }
            std::optional<std::unordered_set<uint64_t>> runOnFeedIDs() const noexcept { return mRunOnFeedIDs; }
            bool existsOnDisk() const noexcept { return mExistsOnDisk; }

            void setType(Type t) { mType = t; }
            void setFilename(const std::string& f)
            {
                mFilename = f;
                mExistsOnDisk = exists();
            }
            void setIsEnabled(bool b) { mIsEnabled = b; }
            void parseRunOnEvents(const std::string& str);
            void parseRunOnFeedIDs(const std::string& str);

            virtual std::string scriptContents() const = 0;
            virtual void update(Type type, const std::string& filename, bool enabled, const std::unordered_set<Event>& events,
                                const std::optional<std::unordered_set<uint64_t>>& feedIDs) = 0;

            static std::string msEventNewPostIdentifier;
            static std::string msEventUpdatePostIdentifier;
            static std::string msTypeLuaIdentifier;

          protected:
            uint64_t mID{0};
            Type mType{Type::Lua};
            std::string mFilename{""};
            bool mIsEnabled{false};
            std::unordered_set<Event> mRunOnEvents{};
            std::optional<std::unordered_set<uint64_t>> mRunOnFeedIDs{};
            bool mExistsOnDisk{false};

            virtual bool exists() const = 0;
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_SCRIPT_H