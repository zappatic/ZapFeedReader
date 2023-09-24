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

#ifndef ZAPFR_ENGINE_SCRIPTLUA_H
#define ZAPFR_ENGINE_SCRIPTLUA_H

#include "ZapFR/Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class Source;
        class Feed;
        class Post;

        class ScriptLua
        {
          public:
            ScriptLua(const ScriptLua&) = delete;
            ScriptLua& operator=(const ScriptLua&) = delete;
            virtual ~ScriptLua() = default;

            static ScriptLua* getInstance();

            static void runPostScript(const std::string& script, Source* source, Feed* feed, Post* post,
                                      const std::optional<const std::function<void(const std::string&)>>& printCallback = {});

            static int printTrap(lua_State* L);

          private:
            explicit ScriptLua();
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_SCRIPTLUA_H