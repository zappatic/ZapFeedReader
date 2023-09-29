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

#include <Poco/Data/RecordSet.h>

#include "ZapFR/Database.h"
#include "ZapFR/Helpers.h"
#include "ZapFR/base/Source.h"
#include "ZapFR/local/ScriptLocal.h"

using namespace Poco::Data::Keywords;

ZapFR::Engine::ScriptLocal::ScriptLocal(uint64_t id, Source* parentSource) : Script(id, parentSource)
{
}

void ZapFR::Engine::ScriptLocal::update(Type /*type*/, const std::string& title, bool enabled, const std::unordered_set<Event>& events,
                                        const std::optional<std::unordered_set<uint64_t>>& feedIDs, const std::string& script)
{
    // join all the events into a comma separated identifier string
    std::vector<std::string> eventStrings;
    if (events.contains(Event::NewPost))
    {
        eventStrings.emplace_back(msEventNewPostIdentifier);
    }
    if (events.contains(Event::UpdatePost))
    {
        eventStrings.emplace_back(msEventUpdatePostIdentifier);
    }
    auto joinedEvents = Helpers::joinString(eventStrings, ",");

    // join all the selected feedIDs into a comma separated string
    Poco::Nullable<std::string> joinedFeedIDs;
    if (feedIDs.has_value())
    {
        std::vector<uint64_t> f;
        for (const auto& feedID : feedIDs.value())
        {
            f.emplace_back(feedID);
        }
        joinedFeedIDs = Helpers::joinIDNumbers(f, ",");
    }

    Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
    updateStmt << "UPDATE scripts SET" // type is not set (yet), default = Lua
                  " title=?"
                  ",isEnabled=?"
                  ",runOnEvents=?"
                  ",runOnFeedIDs=?"
                  ",script=?"
                  " WHERE id=?",
        useRef(title), use(enabled), useRef(joinedEvents), use(joinedFeedIDs), useRef(script), use(mID), now;
}

void ZapFR::Engine::ScriptLocal::remove(uint64_t scriptID)
{
    Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
    deleteStmt << "DELETE FROM scripts WHERE id=?", use(scriptID), now;
}

void ZapFR::Engine::ScriptLocal::create(Script::Type type, const std::string& title, bool enabled, const std::unordered_set<Script::Event>& events,
                                        const std::optional<std::unordered_set<uint64_t>>& feedIDs, const std::string& script)
{
    std::string typeStr;
    switch (type)
    {
        case Script::Type::Lua:
        {
            typeStr = Script::msTypeLuaIdentifier;
            break;
        }
    }

    // join all the events into a comma separated identifier string
    std::vector<std::string> eventStrings;
    if (events.contains(Script::Event::NewPost))
    {
        eventStrings.emplace_back(Script::msEventNewPostIdentifier);
    }
    if (events.contains(Script::Event::UpdatePost))
    {
        eventStrings.emplace_back(Script::msEventUpdatePostIdentifier);
    }
    auto joinedEvents = Helpers::joinString(eventStrings, ",");

    // join all the selected feedIDs into a comma separated string
    Poco::Nullable<std::string> joinedFeedIDs;
    if (feedIDs.has_value())
    {
        std::vector<uint64_t> f;
        for (const auto& feedID : feedIDs.value())
        {
            f.emplace_back(feedID);
        }
        joinedFeedIDs = Helpers::joinIDNumbers(f, ",");
    }

    Poco::Data::Statement insertStmt(*(Database::getInstance()->session()));
    insertStmt << "INSERT INTO scripts"
                  " (type,title,isEnabled,runOnEvents,runOnFeedIDs, script)"
                  " VALUES"
                  " (?,?,?,?,?,?)",
        useRef(typeStr), useRef(title), use(enabled), useRef(joinedEvents), use(joinedFeedIDs), useRef(script), now;
}

std::vector<std::unique_ptr<ZapFR::Engine::Script>> ZapFR::Engine::ScriptLocal::queryMultiple(Source* parentSource, const std::vector<std::string>& whereClause,
                                                                                              const std::string& orderClause, const std::string& limitClause,
                                                                                              const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings)
{
    std::vector<std::unique_ptr<Script>> scripts;

    uint64_t id{0};
    std::string type{""};
    std::string title{""};
    bool isEnabled{false};
    std::string runOnEvents{""};
    Poco::Nullable<std::string> runOnFeedIDs{};
    std::string script;

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT scripts.id"
          ",scripts.type"
          ",scripts.title"
          ",scripts.isEnabled"
          ",scripts.runOnEvents"
          ",scripts.runOnFeedIDs"
          ",scripts.script"
          " FROM scripts";
    if (!whereClause.empty())
    {
        ss << " WHERE ";
        ss << Helpers::joinString(whereClause, " AND ");
    }
    ss << " " << orderClause << " " << limitClause;

    auto sql = ss.str();

    selectStmt << sql, range(0, 1);

    for (const auto& binding : bindings)
    {
        selectStmt.addBind(binding);
    }

    selectStmt.addExtract(into(id));
    selectStmt.addExtract(into(type));
    selectStmt.addExtract(into(title));
    selectStmt.addExtract(into(isEnabled));
    selectStmt.addExtract(into(runOnEvents));
    selectStmt.addExtract(into(runOnFeedIDs));
    selectStmt.addExtract(into(script));

    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            if (type == Script::msTypeLuaIdentifier) // force lua for now
            {
                auto s = std::make_unique<ScriptLocal>(id, parentSource);
                s->setType(Script::Type::Lua);
                s->setTitle(title);
                s->setIsEnabled(isEnabled);
                s->setRunOnEvents(Script::parseRunOnEvents(runOnEvents));
                if (!runOnFeedIDs.isNull())
                {
                    auto feedIDs = Script::parseRunOnFeedIDs(runOnFeedIDs.value());
                    if (feedIDs.size() > 0)
                    {
                        s->setRunOnFeedIDs(feedIDs);
                    }
                }
                s->setScript(script);
                scripts.emplace_back(std::move(s));
            }
        }
    }
    return scripts;
}

std::optional<std::unique_ptr<ZapFR::Engine::Script>> ZapFR::Engine::ScriptLocal::querySingle(Source* parentSource, const std::vector<std::string>& whereClause,
                                                                                              const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings)
{
    uint64_t id{0};
    std::string type{""};
    std::string title{""};
    bool isEnabled{false};
    std::string runOnEvents{""};
    Poco::Nullable<std::string> runOnFeedIDs{};
    std::string script;

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    std::stringstream ss;
    ss << "SELECT scripts.id"
          ",scripts.type"
          ",scripts.title"
          ",scripts.isEnabled"
          ",scripts.runOnEvents"
          ",scripts.runOnFeedIDs"
          ",scripts.script"
          " FROM scripts";
    if (!whereClause.empty())
    {
        ss << " WHERE ";
        ss << Helpers::joinString(whereClause, " AND ");
    }

    auto sql = ss.str();

    selectStmt << sql;

    for (const auto& binding : bindings)
    {
        selectStmt.addBind(binding);
    }

    selectStmt.addExtract(into(id));
    selectStmt.addExtract(into(type));
    selectStmt.addExtract(into(title));
    selectStmt.addExtract(into(isEnabled));
    selectStmt.addExtract(into(runOnEvents));
    selectStmt.addExtract(into(runOnFeedIDs));
    selectStmt.addExtract(into(script));

    selectStmt.execute();

    auto rs = Poco::Data::RecordSet(selectStmt);
    if (rs.rowCount() == 1)
    {
        if (type == Script::msTypeLuaIdentifier) // force lua for now
        {
            auto s = std::make_unique<ScriptLocal>(id, parentSource);
            s->setType(Script::Type::Lua);
            s->setTitle(title);
            s->setIsEnabled(isEnabled);
            s->setRunOnEvents(Script::parseRunOnEvents(runOnEvents));
            if (!runOnFeedIDs.isNull())
            {
                auto feedIDs = Script::parseRunOnFeedIDs(runOnFeedIDs.value());
                if (feedIDs.size() > 0)
                {
                    s->setRunOnFeedIDs(feedIDs);
                }
            }
            s->setScript(script);
            return s;
        }
    }

    return {};
}
