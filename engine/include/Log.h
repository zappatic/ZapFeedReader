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

#ifndef ZAPFR_ENGINE_LOG_H
#define ZAPFR_ENGINE_LOG_H

#include "Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class Log
        {
          public:
            explicit Log(uint64_t id) : mID(id) {}
            virtual ~Log() = default;

            uint64_t id() const noexcept { return mID; }
            std::string timestamp() const noexcept { return mTimestamp; }
            uint64_t level() const noexcept { return mLevel; }
            std::string message() const noexcept { return mMessage; }
            std::optional<uint64_t> feedID() const noexcept { return mFeedID; }
            std::optional<std::string> feedTitle() const noexcept { return mFeedTitle; }

            void setTimestamp(const std::string& timeStamp) { mTimestamp = timeStamp; }
            void setLevel(uint64_t level) { mLevel = level; }
            void setMessage(const std::string& message) { mMessage = message; }
            void setFeedID(uint64_t feedID) { mFeedID = feedID; }
            void setFeedTitle(std::string feedTitle) { mFeedTitle = feedTitle; }

          protected:
            uint64_t mID{0};
            std::string mTimestamp{""};
            uint64_t mLevel{0};
            std::string mMessage{""};
            std::optional<uint64_t> mFeedID{};
            std::optional<std::string> mFeedTitle{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_LOG_H