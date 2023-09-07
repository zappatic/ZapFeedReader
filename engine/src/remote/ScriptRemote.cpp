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

#include "ZapFR/remote/ScriptRemote.h"
#include "ZapFR/Helpers.h"
#include "ZapFR/remote/SourceRemote.h"

ZapFR::Engine::ScriptRemote::ScriptRemote(uint64_t id, Source* parentSource) : Script(id, parentSource)
{
}

void ZapFR::Engine::ScriptRemote::update(Type /*type*/, const std::string& filename, bool enabled, const std::unordered_set<Event>& events,
                                         const std::optional<std::unordered_set<uint64_t>>& feedIDs)
{
    auto remoteSource = dynamic_cast<SourceRemote*>(mParentSource);
    auto uri = remoteSource->remoteURL();
    if (remoteSource->remoteURLIsValid())
    {
        uri.setPath(fmt::format("/script/{}", mID));
        auto creds = Poco::Net::HTTPCredentials(remoteSource->remoteLogin(), remoteSource->remotePassword());

        std::map<std::string, std::string> params;
        params["type"] = Script::msTypeLuaIdentifier; // forced to lua
        params["filename"] = filename;
        params["isEnabled"] = enabled ? "true" : "false";
        params["runOnEvents"] = Script::runOnEventsString(events);
        params["runOnFeedIDs"] = Script::runOnFeedIDsString(feedIDs);

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_PATCH, creds, params);
    }
}

std::unique_ptr<ZapFR::Engine::Script> ZapFR::Engine::ScriptRemote::fromJSON(Source* parentSource, const Poco::JSON::Object::Ptr o)
{
    auto scriptID = o->getValue<uint64_t>(Script::JSONIdentifierScriptID);

    auto script = std::make_unique<ScriptRemote>(scriptID, parentSource);

    auto type = o->getValue<std::string>(Script::JSONIdentifierScriptType);
    if (type == Script::msTypeLuaIdentifier)
    {
        script->setType(Script::Type::Lua);
    }
    else
    {
        throw std::runtime_error("Unknown script type");
    }

    script->setFilename(o->getValue<std::string>(Script::JSONIdentifierScriptFilename));
    script->setIsEnabled(o->getValue<bool>(Script::JSONIdentifierScriptIsEnabled));
    script->setExistsOnDisk(o->getValue<bool>(Script::JSONIdentifierScriptExistsOnDisk));
    script->setRunOnEvents(Script::parseRunOnEvents(o->getValue<std::string>(Script::JSONIdentifierScriptRunOnEvents)));
    if (o->has(Script::JSONIdentifierScriptRunOnFeedIDs))
    {
        script->setRunOnFeedIDs(Script::parseRunOnFeedIDs(o->getValue<std::string>(Script::JSONIdentifierScriptRunOnFeedIDs)));
    }

    return script;
}
