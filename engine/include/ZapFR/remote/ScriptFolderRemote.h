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

#ifndef ZAPFR_ENGINE_SCRIPTFOLDERREMOTE_H
#define ZAPFR_ENGINE_SCRIPTFOLDERREMOTE_H

#include "ZapFR/base/ScriptFolder.h"

namespace ZapFR
{
    namespace Engine
    {
        class ScriptFolderRemote : public ScriptFolder
        {
          public:
            explicit ScriptFolderRemote(uint64_t id, Source* parentSource);
            ~ScriptFolderRemote() = default;

            std::tuple<uint64_t, std::vector<std::unique_ptr<Post>>> getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter,
                                                                              uint64_t categoryFilterID, FlagColor flagColor) override;

            std::vector<std::unique_ptr<ZapFR::Engine::Category>> getCategories() override;

            std::vector<uint64_t> markAsRead(uint64_t maxPostID) override;
            void update(const std::string& title, bool showTotal, bool showUnread) override;

            static std::unique_ptr<ScriptFolder> fromJSON(Source* parentSource, const Poco::JSON::Object::Ptr o);
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_SCRIPTFOLDERREMOTE_H