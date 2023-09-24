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
#include "ZapFR/base/Feed.h"
#include "ZapFR/base/Post.h"
#include "ZapFR/base/Source.h"

int ZapFR::Engine::LuaProxyPost::setTitle(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TSTRING);

    auto [source, feed, post] = lookupPostPointer(L);
    if (post != nullptr)
    {
        auto newTitle = std::string(lua_tostring(L, 2));
        post->setTitle(newTitle);
    }

    return 0;
}

int ZapFR::Engine::LuaProxyPost::setLink(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TSTRING);

    auto [source, feed, post] = lookupPostPointer(L);
    if (post != nullptr)
    {
        auto newLink = std::string(lua_tostring(L, 2));
        post->setLink(newLink);
    }

    return 0;
}

int ZapFR::Engine::LuaProxyPost::setContent(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TSTRING);

    auto [source, feed, post] = lookupPostPointer(L);
    if (post != nullptr)
    {
        auto newContent = std::string(lua_tostring(L, 2));
        post->setContent(newContent);
    }

    return 0;
}

int ZapFR::Engine::LuaProxyPost::setAuthor(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TSTRING);

    auto [source, feed, post] = lookupPostPointer(L);
    if (post != nullptr)
    {
        auto newAuthor = std::string(lua_tostring(L, 2));
        post->setAuthor(newAuthor);
    }

    return 0;
}

int ZapFR::Engine::LuaProxyPost::setCommentsURL(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TSTRING);

    auto [source, feed, post] = lookupPostPointer(L);
    if (post != nullptr)
    {
        auto newCommentsURL = std::string(lua_tostring(L, 2));
        post->setCommentsURL(newCommentsURL);
    }

    return 0;
}

int ZapFR::Engine::LuaProxyPost::markAsRead(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);

    auto [source, feed, post] = lookupPostPointer(L);
    if (source != nullptr)
    {
        source->setPostsReadStatus(true, {{post->feedID(), post->id()}});
    }

    return 0;
}

int ZapFR::Engine::LuaProxyPost::markAsUnread(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);

    auto [source, feed, post] = lookupPostPointer(L);
    if (source != nullptr)
    {
        source->setPostsReadStatus(false, {{post->feedID(), post->id()}});
    }

    return 0;
}

int ZapFR::Engine::LuaProxyPost::flag(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TSTRING);

    auto [source, feed, post] = lookupPostPointer(L);
    if (source != nullptr)
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
        source->setPostsFlagStatus(true, {flagColor}, {{post->feedID(), post->id()}});
    }

    return 0;
}

int ZapFR::Engine::LuaProxyPost::unflag(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TSTRING);

    auto [source, feed, post] = lookupPostPointer(L);
    if (source != nullptr)
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
        source->setPostsFlagStatus(false, {flagColor}, {{post->feedID(), post->id()}});
    }

    return 0;
}

int ZapFR::Engine::LuaProxyPost::assignToScriptFolder(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TNUMBER);

    auto [source, feed, post] = lookupPostPointer(L);
    if (source != nullptr)
    {
        int success;
        auto scriptFolderID = static_cast<uint64_t>(lua_tointegerx(L, 2, &success));
        if (success != 0)
        {
            source->assignPostsToScriptFolder(scriptFolderID, true, {{post->feedID(), post->id()}});
        }
    }

    return 0;
}

int ZapFR::Engine::LuaProxyPost::unassignFromScriptFolder(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TNUMBER);

    auto [source, feed, post] = lookupPostPointer(L);
    if (source != nullptr)
    {
        int success;
        auto scriptFolderID = static_cast<uint64_t>(lua_tointegerx(L, 2, &success));
        if (success != 0)
        {
            source->assignPostsToScriptFolder(scriptFolderID, false, {{post->feedID(), post->id()}});
        }
    }

    return 0;
}

std::tuple<ZapFR::Engine::Source*, ZapFR::Engine::Feed*, ZapFR::Engine::Post*> ZapFR::Engine::LuaProxyPost::lookupPostPointer(lua_State* L)
{
    Source* source{nullptr};
    Feed* feed{nullptr};
    Post* post{nullptr};

    lua_pushvalue(L, 1);
    lua_pushnil(L);
    while (lua_next(L, -2) != 0)
    {
        lua_pushvalue(L, -2); // make a copy of the key
        auto key = std::string(lua_tostring(L, -1));
        if (key == "_source_ptr")
        {
            source = static_cast<Source*>(lua_touserdata(L, -2));
        }
        else if (key == "_feed_ptr")
        {
            feed = static_cast<Feed*>(lua_touserdata(L, -2));
        }
        else if (key == "_post_ptr")
        {
            post = static_cast<Post*>(lua_touserdata(L, -2));
        }

        lua_pop(L, 2); // pop the table value and the cloned key
    }
    lua_pop(L, 1);
    return std::make_tuple(source, feed, post);
}
