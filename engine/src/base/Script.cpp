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

#include <Poco/StringTokenizer.h>

#include "ZapFR/Helpers.h"
#include "ZapFR/base/Script.h"
#include "ZapFR/base/Source.h"

std::string ZapFR::Engine::Script::msEventNewPostIdentifier{"newpost"};
std::string ZapFR::Engine::Script::msEventUpdatePostIdentifier{"updatepost"};

std::string ZapFR::Engine::Script::msTypeLuaIdentifier{"lua"};

ZapFR::Engine::Script::Script(uint64_t id, Source* parentSource) : mID(id), mParentSource(parentSource)
{
}

std::unordered_set<ZapFR::Engine::Script::Event> ZapFR::Engine::Script::parseRunOnEvents(const std::string& str)
{
    std::unordered_set<Event> events;
    Poco::StringTokenizer tok(str, ",", Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);
    for (const auto& entry : tok)
    {
        if (entry == msEventNewPostIdentifier)
        {
            events.insert(Event::NewPost);
        }
        else if (entry == msEventUpdatePostIdentifier)
        {
            events.insert(Event::UpdatePost);
        }
        else
        {
            // silently ignore unknown values
        }
    }
    return events;
}

std::unordered_set<uint64_t> ZapFR::Engine::Script::parseRunOnFeedIDs(const std::string& str)
{
    std::unordered_set<uint64_t> feedIDs;
    Poco::StringTokenizer tok(str, ",", Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);
    for (const auto& entry : tok)
    {
        uint64_t feedID{0};
        if (!Poco::NumberParser::tryParseUnsigned64(entry, feedID))
        {
            continue; // silently ignore unparseable feedIDs
        }
        feedIDs.insert(feedID);
    }
    return feedIDs;
}

std::string ZapFR::Engine::Script::runOnEventsString(const std::unordered_set<Event>& runOnEvents)
{
    std::vector<std::string> events;
    events.reserve(runOnEvents.size());
    for (const auto& event : runOnEvents)
    {
        switch (event)
        {
            case Event::NewPost:
            {
                events.emplace_back(msEventNewPostIdentifier);
                break;
            }
            case Event::UpdatePost:
            {
                events.emplace_back(msEventUpdatePostIdentifier);
                break;
            }
        }
    }
    return Helpers::joinString(events, ",");
}

std::string ZapFR::Engine::Script::runOnFeedIDsString(const std::optional<std::unordered_set<uint64_t>>& runOnFeedIDs)
{
    if (runOnFeedIDs.has_value())
    {
        std::vector<uint64_t> feedIDs;
        feedIDs.reserve(runOnFeedIDs.value().size());
        feedIDs.insert(feedIDs.end(), runOnFeedIDs.value().begin(), runOnFeedIDs.value().end());
        return Helpers::joinIDNumbers(feedIDs, ",");
    }
    else
    {
        return "";
    }
}

Poco::JSON::Object ZapFR::Engine::Script::toJSON()
{
    Poco::JSON::Object o;

    o.set(JSONIdentifierScriptID, mID);
    switch (mType)
    {
        case Type::Lua:
        {
            o.set(JSONIdentifierScriptType, Script::msTypeLuaIdentifier);
            break;
        }
    }
    o.set(JSONIdentifierScriptTitle, mTitle);
    o.set(JSONIdentifierScriptIsEnabled, mIsEnabled);
    o.set(JSONIdentifierScriptRunOnEvents, runOnEventsString(mRunOnEvents));

    if (mRunOnFeedIDs.has_value())
    {
        o.set(JSONIdentifierScriptRunOnFeedIDs, runOnFeedIDsString(mRunOnFeedIDs));
    }

    o.set(JSONIdentifierScriptScript, mScript);

    return o;
}
