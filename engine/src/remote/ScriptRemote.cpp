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

#define FMT_HEADER_ONLY
#include <fmt/core.h>

#include <Poco/Net/HTTPRequest.h>

#include "ZapFR/Helpers.h"
#include "ZapFR/remote/ScriptRemote.h"
#include "ZapFR/remote/SourceRemote.h"

ZapFR::Engine::ScriptRemote::ScriptRemote(uint64_t id, Source* parentSource) : Script(id, parentSource)
{
}

void ZapFR::Engine::ScriptRemote::update(Type /*type*/, const std::string& title, bool enabled, const std::unordered_set<Event>& events,
                                         const std::optional<std::unordered_set<uint64_t>>& feedIDs, const std::string& script)
{
    auto remoteSource = dynamic_cast<SourceRemote*>(mParentSource);
    auto uri = remoteSource->remoteURL();
    if (remoteSource->remoteURLIsValid())
    {
        uri.setPath(fmt::format("/script/{}", mID));
        auto creds = Poco::Net::HTTPCredentials(remoteSource->remoteLogin(), remoteSource->remotePassword());

        std::map<std::string, std::string> params;
        params[HTTPParam::Script::Type] = Script::msTypeLuaIdentifier; // forced to lua
        params[HTTPParam::Script::Title] = title;
        params[HTTPParam::Script::IsEnabled] = enabled ? HTTPParam::True : HTTPParam::False;
        params[HTTPParam::Script::RunOnEvents] = Script::runOnEventsString(events);
        params[HTTPParam::Script::RunOnFeedIDs] = Script::runOnFeedIDsString(feedIDs);
        params[HTTPParam::Script::Script] = script;

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_PATCH, creds, params);
    }
}

std::unique_ptr<ZapFR::Engine::Script> ZapFR::Engine::ScriptRemote::fromJSON(Source* parentSource, const Poco::JSON::Object::Ptr o)
{
    auto scriptID = o->getValue<uint64_t>(JSON::Script::ID);

    auto script = std::make_unique<ScriptRemote>(scriptID, parentSource);

    auto type = o->getValue<std::string>(JSON::Script::Type);
    if (type == Script::msTypeLuaIdentifier)
    {
        script->setType(Script::Type::Lua);
    }
    else
    {
        throw std::runtime_error("Unknown script type");
    }

    script->setTitle(o->getValue<std::string>(JSON::Script::Title));
    script->setIsEnabled(o->getValue<bool>(JSON::Script::IsEnabled));
    script->setScript(o->getValue<std::string>(JSON::Script::Script));
    script->setRunOnEvents(Script::parseRunOnEvents(o->getValue<std::string>(JSON::Script::RunOnEvents)));
    if (o->has(JSON::Script::RunOnFeedIDs))
    {
        script->setRunOnFeedIDs(Script::parseRunOnFeedIDs(o->getValue<std::string>(JSON::Script::RunOnFeedIDs)));
    }

    return script;
}
