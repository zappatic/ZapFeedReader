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

#include "ZapFR/ScriptLua.h"
#include "ZapFR/Post.h"
#include "ZapFR/lua/LuaProxyPost.h"

extern "C"
{
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

ZapFR::Engine::ScriptLua::ScriptLua()
{
}

ZapFR::Engine::ScriptLua* ZapFR::Engine::ScriptLua::getInstance()
{
    static ScriptLua instance{};
    return &instance;
}

void ZapFR::Engine::ScriptLua::runPostScript(const std::string& script, Post* post)
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    lua_pushinteger(L, msScriptEngineVersion);
    lua_setglobal(L, "ZAPFR_VERSION");

    LuaProxyPost::convertPostToTable(L, post);
    lua_setglobal(L, "CurrentPost");

    auto loadResult = luaL_loadstring(L, script.c_str());
    if (loadResult != LUA_OK)
    {
        auto errorMessage = std::string(lua_tostring(L, 1));
        lua_close(L);
        throw std::runtime_error(errorMessage);
    }

    auto executeResult = lua_pcall(L, 0, 0, 0);
    if (executeResult != LUA_OK)
    {
        auto errorMessage = std::string(lua_tostring(L, 1));
        lua_close(L);
        throw std::runtime_error(errorMessage);
    }

    lua_close(L);
}
