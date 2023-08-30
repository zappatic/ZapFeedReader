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
import re
import json
import argparse
import pathlib

# parse CLI arguments
parser = argparse.ArgumentParser()
parser.add_argument("-c", "--config", required=True, help="The path to the config file to use")
parser.add_argument("-r", "--root", required=True, help="The path to the root of the project, all paths in the config are relative to this root")
args = parser.parse_args()
config = {}

with open(args.config, "r") as fp:
    config = json.load(fp)

current_dir = pathlib.Path(__file__).parent.resolve()
api_json_root = os.path.join(args.root, config["api-json-files-root"])
api = {}
handler_paths = [] # path of the handlers relative to the src directory
re_api_tags = re.compile(r"(\/\/ ::API.*?\/\/ API::\n)", re.MULTILINE | re.DOTALL)

# Load API from api(-*)?.json
for filename in [f for f in os.listdir(api_json_root) if os.path.isfile(os.path.join(api_json_root, f))]:
    if filename.startswith(config["api-prefix"]) and filename.endswith(config["api-suffix"]):
        json_file = os.path.join(api_json_root, filename)
        with open(json_file, "r") as fp:
            api_text = fp.read()
        entries = json.loads(api_text)
        for k in entries:
            if k in api:
                raise RuntimeError("Double entry found in " + filename+ " (" + k + ")")
            api[k] = entries[k]
api = dict(sorted(api.items())) 

# Load APIRequestHandlerRegistration.cpp.tpl
cpp_template_file = os.path.join(current_dir, "..", "src", "APIRequestHandlerRegistration.cpp.tpl")
with open(cpp_template_file, "r") as fp:
    cpp_template = fp.read()

# Load APIHandlers.h.tpl
h_template_file = os.path.join(current_dir, "..", "include", "APIHandlers.h.tpl")
with open(h_template_file, "r") as fp:
    h_template = fp.read()

# Load CMakeLists.txt.tpl
cml_template_file = os.path.join(args.root, config["dest-src"], "CMakeLists.txt.tpl")
with open(cml_template_file, "r") as fp:
    cml_template = fp.read()

# Generate the cpp code
cpp_code = []
h_code = []
for key in api:
    entry = "\t\t{\n"

    entry += f"""\t\t\t\tauto entry = std::make_unique<ZapFR::Server::API>(daemon, R"({api[key]["section"]})", R"({api[key]["description"]})");\n"""

    entry += f"""\t\t\t\tentry->setMethod("{api[key]["method"]}");\n"""

    entry += f"""\t\t\t\tentry->setPath(R"({api[key]["path"]})", R"({api[key]["prettyPath"]})");\n"""

    for uriParameter in api[key]["uriParameters"]:
        entry += f"""\t\t\t\tentry->addURIParameter({{R"({uriParameter["name"]})", R"({uriParameter["description"]})"}});\n"""

    for bodyParameter in api[key]["parameters"]:
        isRequired = "true" if bodyParameter["required"] else "false"
        entry += f"""\t\t\t\tentry->addBodyParameter({{R"({bodyParameter["name"]})", {isRequired}, R"({bodyParameter["description"]})"}});\n"""

    requiresCredentials = "true" if api[key]["requireCredentials"] else "false"
    entry += f"""\t\t\t\tentry->setRequiresCredentials({requiresCredentials});\n"""

    entry += f"""\t\t\t\tentry->setContentType(R"({api[key]["contentType"]})");\n"""

    if 'jsonOutput' in api[key]:
        entry += f"""\t\t\t\tentry->setJSONOutput(R"({api[key]["jsonOutput"]})");\n"""

    safe_key = key.replace('-', '_')
    entry += f"""\t\t\t\tentry->setHandler(ZapFR::Server::APIHandler_{safe_key});\n"""

    if "acceptsFileUploads" in api[key]:
        acceptsFileUploads = "true" if api[key]["acceptsFileUploads"] else "false"
        entry += f"""\t\t\t\tentry->setAcceptsFileUploads({acceptsFileUploads});\n"""
    
    entry += "\t\t\t\tmsAPIs.emplace_back(std::move(entry));\n"

    entry += "\t\t\t}\n"

    cpp_code.append(entry)

    headerline = f"""\t\tPoco::Net::HTTPResponse::HTTPStatus APIHandler_{safe_key}(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);"""
    h_code.append(headerline)

    # Check whether the actual handler cpp file exists somewhere in the handlers tree, if not create a blank file
    handler_path = ""
    handler_filename = f"""APIHandler_{safe_key}.cpp"""
    handler_exists = False
    handler_root = os.path.realpath(os.path.join(args.root, config["dest-handlers"]))
    for root, subdirs, files in os.walk(handler_root):
        if handler_filename in files:
            handler_path = os.path.join(root, handler_filename)
            handler_paths.append(os.path.join(root, handler_filename).replace(os.path.realpath(os.path.join(args.root, config["dest-src"])) + "/", ""))
            handler_exists = True
            break
    if not handler_exists:
        cpp_handler = os.path.join(handler_root, handler_filename)
        handler_path = cpp_handler
        handler_paths.append(os.path.join(handler_root, handler_filename).replace(os.path.realpath(os.path.join(args.root, config["dest-src"])) + "/", ""))
        with open(cpp_handler, "w") as fp:
            fp.write(config["handler-copyright"])
            fp.write(f"""#include "API.h"\n""")
            fp.write(f"""#include "APIRequest.h"\n""")
            fp.write(f"""#include "APIHandlers.h"\n\n""")
            fp.write(f"""// ::API>\n""")
            fp.write(f"""// API::\n\n""")
            fp.write(f"""Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_{safe_key}([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)\n""")
            fp.write(f"""{{\n""")

            uriParamCounter = 1
            for uriParameter in api[key]["uriParameters"]:
                fp.write(f"""   const auto {uriParameter["name"]} = apiRequest->pathComponentAt({uriParamCounter});\n""")
                uriParamCounter = uriParamCounter + 1

            for bodyParameter in api[key]["parameters"]:
                fp.write(f"""   const auto {bodyParameter["name"]} = apiRequest->parameter("{bodyParameter["name"]}");\n""")

            fp.write(f"""\n\n   Poco::JSON::Object o;\n""")
            fp.write(f"""   o.set("success", true);\n\n""")
            fp.write(f"""   Poco::JSON::Stringifier::stringify(o, response.send());\n\n""")
            fp.write(f"""   return Poco::Net::HTTPResponse::HTTP_OK;\n""")
            fp.write(f"""\n}}""")

    # insert the API information if the tags are present
    handler_contents = ""
    with open(handler_path, "r") as fp:
        handler_contents = fp.read()

    if re_api_tags.search(handler_contents):
        replacement  = f"""// ::API\n//\n"""
        replacement += f"""//\t{api[key]["description"]}\n"""
        replacement += f"""//\t{api[key]["prettyPath"]} ({api[key]["method"]})\n//\n"""

        if len(api[key]["uriParameters"]) > 0:
            replacement += f"""//\tURI parameters:\n"""
            counter = 1
            for uriParameter in api[key]["uriParameters"]:
                replacement += f"""//\t\t{uriParameter["name"]} - {uriParameter["description"]} - apiRequest->pathComponentAt({counter})\n"""
                counter = counter + 1
            replacement += f"""//\n"""

        if len(api[key]["parameters"]) > 0:
            replacement += f"""//\tParameters:\n"""
            for bodyParameter in api[key]["parameters"]:
                isRequired = " (REQD)" if bodyParameter["required"] else ""
                replacement += f"""//\t\t{bodyParameter["name"]}{isRequired} - {bodyParameter["description"]} - apiRequest->parameter("{bodyParameter["name"]}")\n"""
            replacement += f"""//\n"""

        replacement += f"""//\tContent-Type: {api[key]["contentType"]}\n"""

        if 'jsonOutput' in api[key]:
            replacement += f"""//\tJSON output: {api[key]["jsonOutput"]}\n"""

        if "acceptsFileUploads" in api[key]:
            acceptsFileUploads = f"""//\tAccepts file uploads\n""" if api[key]["acceptsFileUploads"] else ""
            if len(acceptsFileUploads) > 0:
                replacement += acceptsFileUploads

        replacement += f"""//\n// API::\n"""

        handler_contents = re_api_tags.sub(replacement, handler_contents)
        with open(handler_path, "w") as fp:
            fp.write(handler_contents)
    else:
        print("::API:: tag missing for ", handler_path)

# Write out the template to APIRequestHandlerRegistration.cpp
cpp_template_destination_file = os.path.join(args.root, config["dest-src"], "APIRequestHandlerRegistration.cpp")
with open(cpp_template_destination_file, "w") as fp:
    fp.write("/* **************************************************************************** */\n")
    fp.write("/* **** THIS FILE IS AUTO GENERATED                                        **** */\n")
    fp.write("/* **** DO NOT MODIFY THIS FILE, ANY CHANGES WILL BE UNDONE UPON RECOMPILE **** */\n")
    fp.write("/* **** USE APIRequestHandlerRegistration.cpp.tpl INSTEAD                  **** */\n")
    fp.write("/* **************************************************************************** */\n\n\n\n")
    fp.write(cpp_template.replace("%REGISTERAPIS%", "\n".join(cpp_code)))

# Write out the template to APIHandlers.h
h_template_destination_file = os.path.join(args.root, config["dest-include"], "APIHandlers.h")
with open(h_template_destination_file, "w") as fp:
    fp.write("/* **************************************************************************** */\n")
    fp.write("/* **** THIS FILE IS AUTO GENERATED                                        **** */\n")
    fp.write("/* **** DO NOT MODIFY THIS FILE, ANY CHANGES WILL BE UNDONE UPON RECOMPILE **** */\n")
    fp.write("/* **** USE APIHandlers.h.tpl INSTEAD                                      **** */\n")
    fp.write("/* **************************************************************************** */\n\n\n\n")
    fp.write(h_template.replace("%APIHANDLERHEADERS%", "\n".join(h_code)))

# Write out the CMakeLists.txt and include all the handlers
cml_template_destination_file = os.path.join(args.root, config["dest-src"], "CMakeLists.txt")
with open(cml_template_destination_file, "w") as fp:
    fp.write("##############################################################################\n")
    fp.write("##### THIS FILE IS AUTO GENERATED                                        #####\n")
    fp.write("##### DO NOT MODIFY THIS FILE, ANY CHANGES WILL BE UNDONE UPON RECOMPILE #####\n")
    fp.write("##### USE CMakeLists.txt.tpl INSTEAD                                     #####\n")
    fp.write("##############################################################################\n\n\n\n")
    fp.write(cml_template.replace("%HANDLERS%", "\n\t".join(sorted(handler_paths))))
