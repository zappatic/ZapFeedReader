/* **************************************************************************** */
/* **** THIS FILE IS AUTO GENERATED                                        **** */
/* **** DO NOT MODIFY THIS FILE, ANY CHANGES WILL BE UNDONE UPON RECOMPILE **** */
/* **** USE LuaSyntaxHighlighting.h.tpl INSTEAD                            **** */
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

#ifndef ZAPFR_ENGINE_LUASYNTAXHIGHLIGHTING_H
#define ZAPFR_ENGINE_LUASYNTAXHIGHLIGHTING_H

#include "ZapFR/Global.h"

namespace ZapFR
{
    namespace Engine
    {
        static std::array<std::string, 19> gsLuaPostSyntaxHighlighting{
            R"(:markAsRead\(\))", 
            R"(:markAsUnread\(\))", 
            R"(:flag\(.*?\))", 
            R"(:unflag\(.*?\))", 
            R"(:assignToScriptFolder\(.*?\))", 
            R"(:unassignFromScriptFolder\(.*?\))", 
            R"(:getTitle\(.*?\))", 
            R"(:setTitle\(.*?\))", 
            R"(:setLink\(.*?\))", 
            R"(:getLink\(.*?\))", 
            R"(:getLink\(.*?\))", 
            R"(:getContent\(.*?\))", 
            R"(:setContent\(.*?\))", 
            R"(:setAuthor\(.*?\))", 
            R"(:getAuthor\(.*?\))", 
            R"(:getCommentsURL\(.*?\))", 
            R"(:setCommentsURL\(.*?\))", 
            R"(:getEnclosures\(.*?\))", 
            R"(:addEnclosure\(.*?\))", 

        };

        static std::array<std::string, 7> gsLuaPostEnclosuresSyntaxHighlighting{
            R"(:setURL\(.*?\))", 
            R"(:getURL\(.*?\))", 
            R"(:setMimeType\(.*?\))", 
            R"(:getMimeType\(.*?\))", 
            R"(:setSize\(.*?\))", 
            R"(:getSize\(.*?\))", 
            R"(:remove\(.*?\))", 

        };
    }
} // namespace ZapFR

#endif // ZAPFR_ENGINE_LUASYNTAXHIGHLIGHTING_H
