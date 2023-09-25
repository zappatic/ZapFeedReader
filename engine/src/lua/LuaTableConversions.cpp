/* **************************************************************************** */
/* **** THIS FILE IS AUTO GENERATED                                        **** */
/* **** DO NOT MODIFY THIS FILE, ANY CHANGES WILL BE UNDONE UPON RECOMPILE **** */
/* **** USE LuaTableConversions.cpp.tpl INSTEAD                            **** */
/* **************************************************************************** */



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
    lua_createtable(L, 0, 3 + 19);

    lua_pushlightuserdata(L, static_cast<void*>(source));
    lua_setfield(L, -2, "_source_ptr");

    lua_pushlightuserdata(L, static_cast<void*>(feed));
    lua_setfield(L, -2, "_feed_ptr");

    lua_pushlightuserdata(L, static_cast<void*>(post));
    lua_setfield(L, -2, "_post_ptr");

    // :markAsRead - Marks the post as read
    lua_pushcfunction(L, markAsRead);
    lua_setfield(L, -2, "markAsRead");

    // :markAsUnread - Marks the post as unread
    lua_pushcfunction(L, markAsUnread);
    lua_setfield(L, -2, "markAsUnread");

    // :flag - Assigns a flag of the specified color to the post. Valid colors are "blue", "green", "yellow", "orange", "red", "purple"
    lua_pushcfunction(L, flag);
    lua_setfield(L, -2, "flag");

    // :unflag - Unassigns a flag of a specified color from the post. Valid colors are "blue", "green", "yellow", "orange", "red", "purple"
    lua_pushcfunction(L, unflag);
    lua_setfield(L, -2, "unflag");

    // :assignToScriptFolder - Assigns the post to the specified script folder
    lua_pushcfunction(L, assignToScriptFolder);
    lua_setfield(L, -2, "assignToScriptFolder");

    // :unassignFromScriptFolder - Unassigns the post from the specified script folder
    lua_pushcfunction(L, unassignFromScriptFolder);
    lua_setfield(L, -2, "unassignFromScriptFolder");

    // :getTitle - Gets the title of the post
    lua_pushcfunction(L, getTitle);
    lua_setfield(L, -2, "getTitle");

    // :setTitle - Sets the title of the post
    lua_pushcfunction(L, setTitle);
    lua_setfield(L, -2, "setTitle");

    // :setLink - Sets the link of the post
    lua_pushcfunction(L, setLink);
    lua_setfield(L, -2, "setLink");

    // :getLink - Gets the link of the post
    lua_pushcfunction(L, getLink);
    lua_setfield(L, -2, "getLink");

    // :getLink - Gets the link of the post
    lua_pushcfunction(L, getLink);
    lua_setfield(L, -2, "getLink");

    // :getContent - Gets the content of the post
    lua_pushcfunction(L, getContent);
    lua_setfield(L, -2, "getContent");

    // :setContent - Sets the content of the post
    lua_pushcfunction(L, setContent);
    lua_setfield(L, -2, "setContent");

    // :setAuthor - Sets the author of the post
    lua_pushcfunction(L, setAuthor);
    lua_setfield(L, -2, "setAuthor");

    // :getAuthor - Gets the author of the post
    lua_pushcfunction(L, getAuthor);
    lua_setfield(L, -2, "getAuthor");

    // :getCommentsURL - Gets the comment URL of the post
    lua_pushcfunction(L, getCommentsURL);
    lua_setfield(L, -2, "getCommentsURL");

    // :setCommentsURL - Sets the comment URL of the post
    lua_pushcfunction(L, setCommentsURL);
    lua_setfield(L, -2, "setCommentsURL");

    // :getEnclosures - Gets the enclosures of a post
    lua_pushcfunction(L, getEnclosures);
    lua_setfield(L, -2, "getEnclosures");

    // :addEnclosure - Adds a new enclosure
    lua_pushcfunction(L, addEnclosure);
    lua_setfield(L, -2, "addEnclosure");


}

void ZapFR::Engine::LuaProxyPost::convertPostEnclosuresToTable(lua_State* L, Post* post)
{
    const auto& enclosures = post->enclosures();
    auto enclosureCount = static_cast<int32_t>(enclosures.size());

    lua_createtable(L, enclosureCount, 0);

    for (auto i = 1; i <= enclosureCount; ++i)
    {
        lua_createtable(L, 0, 2 + 7);

        lua_pushlightuserdata(L, static_cast<void*>(post));
        lua_setfield(L, -2, "_post_ptr");

        lua_pushnumber(L, i - 1);
        lua_setfield(L, -2, "_index");

        // :setURL - Sets the URL of the post enclosure
        lua_pushcfunction(L, setEnclosureURL);
        lua_setfield(L, -2, "setURL");

        // :getURL - Gets the URL of the post enclosure
        lua_pushcfunction(L, getEnclosureURL);
        lua_setfield(L, -2, "getURL");

        // :setMimeType - Sets the mimetype of the post enclosure
        lua_pushcfunction(L, setEnclosureMimeType);
        lua_setfield(L, -2, "setMimeType");

        // :getMimeType - Gets the mimetype of the post enclosure
        lua_pushcfunction(L, getEnclosureMimeType);
        lua_setfield(L, -2, "getMimeType");

        // :setSize - Sets the size (in bytes) of the post enclosure
        lua_pushcfunction(L, setEnclosureSize);
        lua_setfield(L, -2, "setSize");

        // :getSize - Gets the size (in bytes) of the post enclosure
        lua_pushcfunction(L, getEnclosureSize);
        lua_setfield(L, -2, "getSize");

        // :remove - Removes the post enclosure
        lua_pushcfunction(L, removeEnclosure);
        lua_setfield(L, -2, "remove");



        lua_rawseti(L, -2, i);
    }
}
