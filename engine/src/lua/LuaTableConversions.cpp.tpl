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

#include "ZapFR/base/Feed.h"
#include "ZapFR/base/Post.h"
#include "ZapFR/base/Source.h"
#include "ZapFR/lua/LuaProxyPost.h"

void ZapFR::Engine::LuaProxyPost::convertPostToTable(lua_State* L, Source* source, Feed* feed, Post* post)
{
    lua_createtable(L, 0, 3 + %POSTTABLECOUNT%);

    lua_pushlightuserdata(L, static_cast<void*>(source));
    lua_setfield(L, -2, "_source_ptr");

    lua_pushlightuserdata(L, static_cast<void*>(feed));
    lua_setfield(L, -2, "_feed_ptr");

    lua_pushlightuserdata(L, static_cast<void*>(post));
    lua_setfield(L, -2, "_post_ptr");

%POSTTABLE%
}

void ZapFR::Engine::LuaProxyPost::convertPostEnclosuresToTable(lua_State* L, Post* post)
{
    const auto& enclosures = post->enclosures();
    auto enclosureCount = static_cast<int32_t>(enclosures.size());

    lua_createtable(L, enclosureCount, 0);

    for (auto i = 1; i <= enclosureCount; ++i)
    {
        lua_createtable(L, 0, 2 + %POSTENCLOSURESTABLECOUNT%);

        lua_pushlightuserdata(L, static_cast<void*>(post));
        lua_setfield(L, -2, "_post_ptr");

        lua_pushnumber(L, i - 1);
        lua_setfield(L, -2, "_index");

%POSTENCLOSURESTABLE%

        lua_rawseti(L, -2, i);
    }
}
