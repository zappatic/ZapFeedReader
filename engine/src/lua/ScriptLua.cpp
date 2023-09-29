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

#include "ZapFR/lua/ScriptLua.h"
#include "ZapFR/base/Feed.h"
#include "ZapFR/base/Post.h"
#include "ZapFR/base/Source.h"
#include "ZapFR/lua/LuaProxyPost.h"
#include "ZapFR/Global.h"

namespace
{
    static const std::function<void(const std::string&)>* gsPrintCallback{nullptr};
    static const struct luaL_Reg printTrapLib[] = {{"print", ZapFR::Engine::ScriptLua::printTrap}, {nullptr, nullptr}};
} // namespace

ZapFR::Engine::ScriptLua::ScriptLua()
{
}

ZapFR::Engine::ScriptLua* ZapFR::Engine::ScriptLua::getInstance()
{
    static ScriptLua instance{};
    return &instance;
}

void ZapFR::Engine::ScriptLua::runPostScript(const std::string& script, Source* source, Feed* feed, Post* post,
                                             const std::optional<const std::function<void(const std::string&)>>& printCallback)
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    lua_pushinteger(L, ZapFR::Engine::APIVersion);
    lua_setglobal(L, "ZAPFR_VERSION");

    LuaProxyPost::convertPostToTable(L, source, feed, post);
    lua_setglobal(L, "CurrentPost");

    if (printCallback.has_value())
    {
        gsPrintCallback = &(printCallback.value());

        lua_getglobal(L, "_G");
        luaL_setfuncs(L, printTrapLib, 0);
        lua_pop(L, 1);
    }

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

int ZapFR::Engine::ScriptLua::printTrap(lua_State* L)
{
    int argCount = lua_gettop(L);

    for (auto i = 1; i <= argCount; i++)
    {
        if (lua_isstring(L, i))
        {
            auto message = std::string(lua_tostring(L, i));
            if (gsPrintCallback != nullptr)
            {
                (*gsPrintCallback)(message);
            }
        }
    }
    return 0;
}
