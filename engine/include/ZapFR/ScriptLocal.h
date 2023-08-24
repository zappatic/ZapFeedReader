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

#ifndef ZAPFR_ENGINE_SCRIPTLOCAL_H
#define ZAPFR_ENGINE_SCRIPTLOCAL_H

#include "Script.h"

namespace ZapFR
{
    namespace Engine
    {
        class ScriptLocal : public Script
        {
          public:
            explicit ScriptLocal(uint64_t id);
            ~ScriptLocal() = default;

            std::string scriptContents() const override;
            void update(Type type, const std::string& filename, bool enabled, const std::unordered_set<Event>& events,
                        const std::optional<std::unordered_set<uint64_t>>& feedIDs) override;

            static void setScriptDir(const std::string& scriptDir);
            static std::string msScriptDir;

            static std::vector<std::unique_ptr<Script>> queryMultiple(const std::vector<std::string>& whereClause, const std::string& orderClause,
                                                                      const std::string& limitClause, const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings);
            static std::optional<std::unique_ptr<Script>> querySingle(const std::vector<std::string>& whereClause,
                                                                      const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings);
            static void remove(uint64_t scriptID);
            static void create(Script::Type type, const std::string& filename, bool enabled, const std::unordered_set<Script::Event>& events,
                               const std::optional<std::unordered_set<uint64_t>>& feedIDs);

          protected:
            bool exists() const override;
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_SCRIPTLOCAL_H