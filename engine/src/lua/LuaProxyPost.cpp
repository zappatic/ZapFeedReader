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

#include "ZapFR/lua/LuaProxyPost.h"
#include "ZapFR/Flag.h"
#include "ZapFR/Post.h"

void ZapFR::Engine::LuaProxyPost::convertPostToTable(lua_State* L, Post* post)
{
    lua_createtable(L, 0, 8);

    lua_pushlightuserdata(L, static_cast<void*>(post));
    lua_setfield(L, -2, "_ptr");

    lua_pushstring(L, post->title().c_str());
    lua_setfield(L, -2, "title");

    lua_pushcfunction(L, markAsRead);
    lua_setfield(L, -2, "markAsRead");

    lua_pushcfunction(L, markAsUnread);
    lua_setfield(L, -2, "markAsUnread");

    lua_pushcfunction(L, flag);
    lua_setfield(L, -2, "flag");

    lua_pushcfunction(L, unflag);
    lua_setfield(L, -2, "unflag");

    lua_pushcfunction(L, assignToScriptFolder);
    lua_setfield(L, -2, "assignToScriptFolder");

    lua_pushcfunction(L, unassignFromScriptFolder);
    lua_setfield(L, -2, "unassignFromScriptFolder");
}

int ZapFR::Engine::LuaProxyPost::markAsRead(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);

    auto post = lookupPostPointer(L);
    if (post != nullptr)
    {
        post->markAsRead();
    }

    return 0;
}

int ZapFR::Engine::LuaProxyPost::markAsUnread(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);

    auto post = lookupPostPointer(L);
    if (post != nullptr)
    {
        post->markAsUnread();
    }

    return 0;
}

int ZapFR::Engine::LuaProxyPost::flag(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TSTRING);

    auto post = lookupPostPointer(L);
    if (post != nullptr)
    {
        FlagColor flagColor;
        auto flagColorStr = std::string(lua_tostring(L, 2));
        try
        {
            flagColor = Flag::flagColorForName(flagColorStr);
        }
        catch (...)
        {
            return luaL_error(L, "Invalid flag color specified");
        }
        post->markFlagged(flagColor);
    }

    return 0;
}

int ZapFR::Engine::LuaProxyPost::unflag(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TSTRING);

    auto post = lookupPostPointer(L);
    if (post != nullptr)
    {
        FlagColor flagColor;
        auto flagColorStr = std::string(lua_tostring(L, 2));
        try
        {
            flagColor = Flag::flagColorForName(flagColorStr);
        }
        catch (...)
        {
            return luaL_error(L, "Invalid flag color specified");
        }
        post->markUnflagged(flagColor);
    }

    return 0;
}

int ZapFR::Engine::LuaProxyPost::assignToScriptFolder(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TNUMBER);

    auto post = lookupPostPointer(L);
    if (post != nullptr)
    {
        int success;
        auto scriptFolderID = lua_tointegerx(L, 2, &success);
        if (success != 0)
        {
            post->assignToScriptFolder(scriptFolderID);
        }
    }

    return 0;
}

int ZapFR::Engine::LuaProxyPost::unassignFromScriptFolder(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TNUMBER);

    auto post = lookupPostPointer(L);
    if (post != nullptr)
    {
        int success;
        auto scriptFolderID = lua_tointegerx(L, 2, &success);
        if (success != 0)
        {
            post->unassignFromScriptFolder(scriptFolderID);
        }
    }

    return 0;
}

ZapFR::Engine::Post* ZapFR::Engine::LuaProxyPost::lookupPostPointer(lua_State* L)
{
    Post* post{nullptr};

    lua_pushvalue(L, 1);
    lua_pushnil(L);
    while (lua_next(L, -2) != 0)
    {
        lua_pushvalue(L, -2); // make a copy of the key
        auto key = std::string(lua_tostring(L, -1));
        if (key == "_ptr")
        {
            post = static_cast<Post*>(lua_touserdata(L, -2));
        }

        lua_pop(L, 2); // pop the table value and the cloned key
    }
    lua_pop(L, 1);
    return post;
}
