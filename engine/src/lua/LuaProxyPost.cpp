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
#include "ZapFR/local/FeedLocal.h"
#include "ZapFR/local/PostLocal.h"
#include "ZapFR/local/SourceLocal.h"

void ZapFR::Engine::LuaProxyPost::convertPostToTable(lua_State* L, SourceLocal* source, FeedLocal* feed, PostLocal* post)
{
    lua_createtable(L, 0, 10);

    lua_pushlightuserdata(L, static_cast<void*>(source));
    lua_setfield(L, -2, "_source_ptr");

    lua_pushlightuserdata(L, static_cast<void*>(feed));
    lua_setfield(L, -2, "_feed_ptr");

    lua_pushlightuserdata(L, static_cast<void*>(post));
    lua_setfield(L, -2, "_post_ptr");

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

    auto [source, feed, post] = lookupPostPointer(L);
    if (post != nullptr)
    {
        post->markAsRead();
    }

    return 0;
}

int ZapFR::Engine::LuaProxyPost::markAsUnread(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);

    auto [source, feed, post] = lookupPostPointer(L);
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

    auto [source, feed, post] = lookupPostPointer(L);
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

    auto [source, feed, post] = lookupPostPointer(L);
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

    auto [source, feed, post] = lookupPostPointer(L);
    if (post != nullptr)
    {
        int success;
        auto scriptFolderID = static_cast<uint64_t>(lua_tointegerx(L, 2, &success));
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

    auto [source, feed, post] = lookupPostPointer(L);
    if (post != nullptr)
    {
        int success;
        auto scriptFolderID = static_cast<uint64_t>(lua_tointegerx(L, 2, &success));
        if (success != 0)
        {
            post->unassignFromScriptFolder(scriptFolderID);
        }
    }

    return 0;
}

std::tuple<ZapFR::Engine::SourceLocal*, ZapFR::Engine::FeedLocal*, ZapFR::Engine::PostLocal*> ZapFR::Engine::LuaProxyPost::lookupPostPointer(lua_State* L)
{
    SourceLocal* source{nullptr};
    FeedLocal* feed{nullptr};
    PostLocal* post{nullptr};

    lua_pushvalue(L, 1);
    lua_pushnil(L);
    while (lua_next(L, -2) != 0)
    {
        lua_pushvalue(L, -2); // make a copy of the key
        auto key = std::string(lua_tostring(L, -1));
        if (key == "_source_ptr")
        {
            source = static_cast<SourceLocal*>(lua_touserdata(L, -2));
        }
        else if (key == "_feed_ptr")
        {
            feed = static_cast<FeedLocal*>(lua_touserdata(L, -2));
        }
        else if (key == "_post_ptr")
        {
            post = static_cast<PostLocal*>(lua_touserdata(L, -2));
        }

        lua_pop(L, 2); // pop the table value and the cloned key
    }
    lua_pop(L, 1);
    return std::make_tuple(source, feed, post);
}
