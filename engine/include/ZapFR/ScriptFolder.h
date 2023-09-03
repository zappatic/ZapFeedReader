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

#ifndef ZAPFR_ENGINE_SCRIPTFOLDER_H
#define ZAPFR_ENGINE_SCRIPTFOLDER_H

#include "Flag.h"
#include "Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class Post;

        class ScriptFolder
        {
          public:
            explicit ScriptFolder(uint64_t id) : mID(id) {}
            virtual ~ScriptFolder() = default;

            uint64_t id() const noexcept { return mID; }
            std::string title() const noexcept { return mTitle; }
            void setTitle(const std::string& title) { mTitle = title; }

            virtual std::tuple<uint64_t, std::vector<std::unique_ptr<Post>>> getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter,
                                                                                      FlagColor flagColor) = 0;

            virtual void update(const std::string& title) = 0;

          protected:
            uint64_t mID{0};
            std::string mTitle{""};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_SCRIPTFOLDER_H