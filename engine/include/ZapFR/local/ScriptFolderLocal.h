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

#ifndef ZAPFR_ENGINE_SCRIPTFOLDERLOCAL_H
#define ZAPFR_ENGINE_SCRIPTFOLDERLOCAL_H

#include "../ScriptFolder.h"

namespace ZapFR
{
    namespace Engine
    {
        class ScriptFolderLocal : public ScriptFolder
        {
          public:
            explicit ScriptFolderLocal(uint64_t id);
            ~ScriptFolderLocal() = default;

            std::tuple<uint64_t, std::vector<std::unique_ptr<Post>>> getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter,
                                                                              FlagColor flagColor) override;

            void update(const std::string& title) override;

            static std::vector<std::unique_ptr<ScriptFolder>> queryMultiple(const std::vector<std::string>& whereClause, const std::string& orderClause,
                                                                            const std::string& limitClause, const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings);
            static std::optional<std::unique_ptr<ScriptFolder>> querySingle(const std::vector<std::string>& whereClause,
                                                                            const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings);
            static void create(const std::string& title);
            static void remove(uint64_t scriptFolderID);
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_POSTLOCAL_H