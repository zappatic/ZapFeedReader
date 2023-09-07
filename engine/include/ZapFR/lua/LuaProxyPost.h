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

#ifndef ZAPFR_ENGINE_LUAPROXYPOST_H
#define ZAPFR_ENGINE_LUAPROXYPOST_H

#include "ZapFR/Global.h"

extern "C"
{
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

namespace ZapFR
{
    namespace Engine
    {
        class SourceLocal;
        class FeedLocal;
        class PostLocal;

        class LuaProxyPost
        {
          public:
            static void convertPostToTable(lua_State* L, SourceLocal* source, FeedLocal* feed, PostLocal* post);

          private:
            static int markAsRead(lua_State* L);
            static int markAsUnread(lua_State* L);
            static int flag(lua_State* L);
            static int unflag(lua_State* L);
            static int assignToScriptFolder(lua_State* L);
            static int unassignFromScriptFolder(lua_State* L);

            static std::tuple<ZapFR::Engine::SourceLocal*, ZapFR::Engine::FeedLocal*, ZapFR::Engine::PostLocal*> lookupPostPointer(lua_State* L);
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_LUAPROXYPOST_H