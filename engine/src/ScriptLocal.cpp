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

#include "ZapFR/ScriptLocal.h"
#include "ZapFR/Database.h"
#include "ZapFR/Helpers.h"

using namespace Poco::Data::Keywords;

std::string ZapFR::Engine::ScriptLocal::msScriptDir{""};

ZapFR::Engine::ScriptLocal::ScriptLocal(uint64_t id) : Script(id)
{
}

std::string ZapFR::Engine::ScriptLocal::scriptContents() const
{
    std::string script = "";
    if (exists())
    {
        auto scriptFile = Poco::File(msScriptDir + Poco::Path::separator() + mFilename);
        Poco::FileInputStream fis(scriptFile.path());
        Poco::StreamCopier::copyToString(fis, script);
        fis.close();
    }
    return script;
}

bool ZapFR::Engine::ScriptLocal::exists() const
{
    if (mFilename.empty())
    {
        return false;
    }

    auto scriptFile = Poco::File(msScriptDir + Poco::Path::separator() + mFilename);
    if (!scriptFile.exists())
    {
        return false;
    }
    return true;
}

void ZapFR::Engine::ScriptLocal::setScriptDir(const std::string& iconDir)
{
    msScriptDir = iconDir;
    auto d = Poco::File(msScriptDir);
    if (!d.exists())
    {
        d.createDirectories();
    }
}

void ZapFR::Engine::ScriptLocal::update(Type /*type*/, const std::string& filename, bool enabled, const std::unordered_set<Event>& events,
                                        const std::optional<std::unordered_set<uint64_t>>& feedIDs)
{
    // join all the events into a comma separated identifier string
    std::vector<std::string> eventStrings;
    if (events.contains(Event::NewPost))
    {
        eventStrings.emplace_back(msEventNewPostIdentifier);
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
                  " filename=?"
                  ",isEnabled=?"
                  ",runOnEvents=?"
                  ",runOnFeedIDs=?"
                  " WHERE id=?",
        useRef(filename), use(enabled), useRef(joinedEvents), use(joinedFeedIDs), use(mID), now;
}
