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
    lua_createtable(L, 0, 16);

    lua_pushlightuserdata(L, static_cast<void*>(source));
    lua_setfield(L, -2, "_source_ptr");

    lua_pushlightuserdata(L, static_cast<void*>(feed));
    lua_setfield(L, -2, "_feed_ptr");

    lua_pushlightuserdata(L, static_cast<void*>(post));
    lua_setfield(L, -2, "_post_ptr");

        // .title - Retrieves the title of the post
    lua_pushstring(L, post->title().c_str());
    lua_setfield(L, -2, "title");

    // .link - Retrieves the link of the post
    lua_pushstring(L, post->link().c_str());
    lua_setfield(L, -2, "link");

    // .content - Retrieves the content of the post
    lua_pushstring(L, post->content().c_str());
    lua_setfield(L, -2, "content");

    // .author - Retrieves the author of the post
    lua_pushstring(L, post->author().c_str());
    lua_setfield(L, -2, "author");

    // .commentsURL - Retrieves the comments URL of the post
    lua_pushstring(L, post->commentsURL().c_str());
    lua_setfield(L, -2, "commentsURL");

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

    // :setTitle - Sets the title of the post
    lua_pushcfunction(L, setTitle);
    lua_setfield(L, -2, "setTitle");

    // :setLink - Sets the link of the post
    lua_pushcfunction(L, setLink);
    lua_setfield(L, -2, "setLink");

    // :setContent - Sets the content of the post
    lua_pushcfunction(L, setContent);
    lua_setfield(L, -2, "setContent");

    // :setAuthor - Sets the author of the post
    lua_pushcfunction(L, setAuthor);
    lua_setfield(L, -2, "setAuthor");

    // :setCommentsURL - Sets the comment URL of the post
    lua_pushcfunction(L, setCommentsURL);
    lua_setfield(L, -2, "setCommentsURL");


}
