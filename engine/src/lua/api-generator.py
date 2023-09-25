#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#    ZapFeedReader - RSS/Atom feed reader
#    Copyright (C) 2023-present  Kasper Nauwelaerts (zapfr at zappatic dot net)

#    ZapFeedReader is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.

#    ZapFeedReader is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with ZapFeedReader.  If not, see <https://www.gnu.org/licenses/>.

import os
import json

with open("lua-api.json", "r") as fp:
    api_json = fp.read()
api_entries = json.loads(api_json)

def dumpField(field, indent):
    spaces = " " * (indent * 4)
    str  = f"""{spaces}// .{field["name"]} - {field["description"]}\n"""
    if field["type"] == "string":
        str += f"""{spaces}lua_pushstring(L, {field["value"]});\n"""
    elif field["type"] == "table":
        str += f"""{spaces}{field["value"]};\n"""
    elif field["type"] == "integer":
        str += f"""{spaces}lua_pushinteger(L, {field["value"]});\n"""
    else:
        raise("Unknown field type")
    str += f"""{spaces}lua_setfield(L, -2, "{field["name"]}");\n\n"""
    return str

def dumpMethod(method, indent):
    spaces = " " * (indent * 4)
    str  = f"""{spaces}// :{method["name"]} - {method["description"]}\n"""
    str += f"""{spaces}lua_pushcfunction(L, {method["value"]});\n"""
    str += f"""{spaces}lua_setfield(L, -2, "{method["name"]}");\n\n"""
    return str


# POST TABLE
post_table_count = 0
post_table = ""
for field in api_entries["post"]["fields"]:
    post_table += dumpField(field, 1)
    post_table_count += 1
for method in api_entries["post"]["methods"]:
    post_table += dumpMethod(method, 1)
    post_table_count += 1

# POST ENCLOSURES TABLE
post_enclosures_table_count = 0
post_enclosures_table = ""
for field in api_entries["post-enclosure"]["fields"]:
    post_enclosures_table += dumpField(field, 2)
    post_enclosures_table_count += 1
for method in api_entries["post-enclosure"]["methods"]:
    post_enclosures_table += dumpMethod(method, 2)
    post_enclosures_table_count += 1

# POST SYNTAX HIGHLIGHTING
post_syntax_highlighting_count = 0
post_syntax_highlighting = ""
for field in api_entries["post"]["fields"]:
    post_syntax_highlighting += f"""            {field["highlight"]}, \n"""
    post_syntax_highlighting_count += 1
for field in api_entries["post"]["methods"]:
    post_syntax_highlighting += f"""            {field["highlight"]}, \n"""
    post_syntax_highlighting_count += 1

# POST ENCLOSURES SYNTAX HIGHLIGHTING
post_enclosures_syntax_highlighting_count = 0
post_enclosures_syntax_highlighting = ""
for field in api_entries["post-enclosure"]["fields"]:
    post_enclosures_syntax_highlighting += f"""            {field["highlight"]}, \n"""
    post_enclosures_syntax_highlighting_count += 1
for field in api_entries["post-enclosure"]["methods"]:
    post_enclosures_syntax_highlighting += f"""            {field["highlight"]}, \n"""
    post_enclosures_syntax_highlighting_count += 1

# CREATE THE NEW TEMPLATES
with open("LuaTableConversions.cpp.tpl", "r") as fp:
    cpp_template = fp.read()
cpp_template = cpp_template.replace("%POSTTABLECOUNT%", str(post_table_count))
cpp_template = cpp_template.replace("%POSTTABLE%", post_table)
cpp_template = cpp_template.replace("%POSTENCLOSURESTABLECOUNT%", str(post_enclosures_table_count))
cpp_template = cpp_template.replace("%POSTENCLOSURESTABLE%", post_enclosures_table)

with open("../../include/ZapFR/lua/LuaSyntaxHighlighting.h.tpl", "r") as fp:
    synhig_template = fp.read()
synhig_template = synhig_template.replace("%POSTSYNTAXHIGHLIGHTINGCOUNT%", str(post_syntax_highlighting_count))
synhig_template = synhig_template.replace("%POSTSYNTAXHIGHLIGHTING%", post_syntax_highlighting)
synhig_template = synhig_template.replace("%POSTENCLOSURESSYNTAXHIGHLIGHTINGCOUNT%", str(post_enclosures_syntax_highlighting_count))
synhig_template = synhig_template.replace("%POSTENCLOSURESSYNTAXHIGHLIGHTING%", post_enclosures_syntax_highlighting)

# WRITE OUT THE FILES
header  = "/* **************************************************************************** */\n"
header += "/* **** THIS FILE IS AUTO GENERATED                                        **** */\n"
header += "/* **** DO NOT MODIFY THIS FILE, ANY CHANGES WILL BE UNDONE UPON RECOMPILE **** */\n"
header += "/* **** USE LuaTableConversions.cpp.tpl INSTEAD                            **** */\n"
header += "/* **************************************************************************** */\n\n\n\n"
cpp_template = header + cpp_template

with open("LuaTableConversions.cpp", "r") as fp:
    existing_cpp_template = fp.read()
if existing_cpp_template != cpp_template:
    with open("LuaTableConversions.cpp", "w") as fp:
        fp.write(cpp_template)

header  = "/* **************************************************************************** */\n"
header += "/* **** THIS FILE IS AUTO GENERATED                                        **** */\n"
header += "/* **** DO NOT MODIFY THIS FILE, ANY CHANGES WILL BE UNDONE UPON RECOMPILE **** */\n"
header += "/* **** USE LuaSyntaxHighlighting.h.tpl INSTEAD                            **** */\n"
header += "/* **************************************************************************** */\n\n\n\n"
synhig_template = header + synhig_template
with open("../../include/ZapFR/lua/LuaSyntaxHighlighting.h", "r") as fp:
    existing_synhig_template = fp.read()
if existing_synhig_template != synhig_template:
    with open("../../include/ZapFR/lua/LuaSyntaxHighlighting.h", "w") as fp:
        fp.write(synhig_template)
