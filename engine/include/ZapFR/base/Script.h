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

#include <optional>
#include <unordered_set>

#include <Poco/JSON/Object.h>

namespace ZapFR
{
    namespace Engine
    {
        class Source;

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

            Script(uint64_t id, Source* parentSource);
            virtual ~Script() = default;
            Script(const Script& e) = delete;
            Script& operator=(const Script&) = delete;
            Script(Script&&) = delete;
            Script& operator=(Script&&) = delete;

            const uint64_t& id() const noexcept { return mID; }
            const std::string& title() const noexcept { return mTitle; }
            const Type& type() const noexcept { return mType; }
            bool isEnabled() const noexcept { return mIsEnabled; }
            const std::unordered_set<Event>& runOnEvents() const noexcept { return mRunOnEvents; }
            const std::optional<std::unordered_set<uint64_t>>& runOnFeedIDs() const noexcept { return mRunOnFeedIDs; }
            const std::string& script() const noexcept { return mScript; }

            void setType(Type t) { mType = t; }
            void setTitle(const std::string& t) { mTitle = t; }
            void setIsEnabled(bool b) { mIsEnabled = b; }
            void setRunOnEvents(std::unordered_set<Event> events) { mRunOnEvents = events; }
            void setRunOnFeedIDs(std::unordered_set<uint64_t> feedIDs) { mRunOnFeedIDs = feedIDs; }
            void setScript(const std::string& s) { mScript = s; }

            virtual void update(Type type, const std::string& title, bool enabled, const std::unordered_set<Event>& events,
                                const std::optional<std::unordered_set<uint64_t>>& feedIDs, const std::string& script) = 0;

            static std::unordered_set<Event> parseRunOnEvents(const std::string& str);
            static std::unordered_set<uint64_t> parseRunOnFeedIDs(const std::string& str);
            static std::string runOnEventsString(const std::unordered_set<Event>& runOnEvents);
            static std::string runOnFeedIDsString(const std::optional<std::unordered_set<uint64_t>>& runOnFeedIDs);

            static std::string msEventNewPostIdentifier;
            static std::string msEventUpdatePostIdentifier;
            static std::string msTypeLuaIdentifier;

            virtual Poco::JSON::Object toJSON();
            static constexpr const char* JSONIdentifierScriptID{"id"};
            static constexpr const char* JSONIdentifierScriptTitle{"title"};
            static constexpr const char* JSONIdentifierScriptType{"type"};
            static constexpr const char* JSONIdentifierScriptIsEnabled{"isEnabled"};
            static constexpr const char* JSONIdentifierScriptRunOnEvents{"runOnEvents"};
            static constexpr const char* JSONIdentifierScriptRunOnFeedIDs{"runOnFeedIDs"};
            static constexpr const char* JSONIdentifierScriptScript{"script"};

          protected:
            uint64_t mID{0};
            Source* mParentSource{nullptr};
            std::string mTitle{""};
            Type mType{Type::Lua};
            bool mIsEnabled{false};
            std::unordered_set<Event> mRunOnEvents{};
            std::optional<std::unordered_set<uint64_t>> mRunOnFeedIDs{};
            std::string mScript{""};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_SCRIPT_H