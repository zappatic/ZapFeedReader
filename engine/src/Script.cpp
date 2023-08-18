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

#include "ZapFR/Script.h"

std::string ZapFR::Engine::Script::msScriptDir{""};

ZapFR::Engine::Script::Script(uint64_t id) : mID(id)
{
}

std::string ZapFR::Engine::Script::scriptContents() const
{
    if (mFilename.empty())
    {
        throw std::runtime_error(fmt::format("No script filename provided for script {}", mID));
    }

    auto scriptFile = Poco::File(msScriptDir + Poco::Path::separator() + mFilename);
    if (!scriptFile.exists())
    {
        throw std::runtime_error(fmt::format("Script '{}' not found in {}", mFilename, msScriptDir));
    }

    Poco::FileInputStream fis(scriptFile.path());
    std::string script;
    Poco::StreamCopier::copyToString(fis, script);
    fis.close();
    return script;
}

void ZapFR::Engine::Script::setScriptDir(const std::string& iconDir)
{
    msScriptDir = iconDir;
    auto d = Poco::File(msScriptDir);
    if (!d.exists())
    {
        d.createDirectories();
    }
}

void ZapFR::Engine::Script::parseRunOnEvents(const std::string& str)
{
    mRunOnEvents.clear();
    Poco::StringTokenizer tok(str, ",", Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);
    for (const auto& entry : tok)
    {
        if (entry == "newpost")
        {
            mRunOnEvents.insert(Event::NewPost);
        }
        else
        {
            // silently ignore unknown values
        }
    }
}

void ZapFR::Engine::Script::parseRunOnFeedIDs(const std::string& str)
{
    mRunOnFeedIDs = std::unordered_set<uint64_t>();
    Poco::StringTokenizer tok(str, ",", Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);
    for (const auto& entry : tok)
    {
        uint64_t feedID{0};
        if (!Poco::NumberParser::tryParseUnsigned64(entry, feedID))
        {
            continue; // silently ignore unparseable feedIDs
        }
        mRunOnFeedIDs.value().insert(feedID);
    }
}
