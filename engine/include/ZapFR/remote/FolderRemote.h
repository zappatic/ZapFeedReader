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

#ifndef ZAPFR_ENGINE_FOLDERREMOTE_H
#define ZAPFR_ENGINE_FOLDERREMOTE_H

#include "ZapFR/base/Folder.h"

namespace ZapFR
{
    namespace Engine
    {
        class FolderRemote : public Folder
        {
          public:
            FolderRemote(uint64_t id, uint64_t parentFolderID, Source* parentSource);
            virtual ~FolderRemote() = default;

            std::tuple<uint64_t, std::vector<std::unique_ptr<Post>>> getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, bool showUnreadPostsAtTop,
                                                                              const std::string& searchFilter, uint64_t categoryFilterID, FlagColor flagColor) override;
            std::vector<uint64_t> markAsRead(uint64_t maxPostID) override;

            std::vector<std::unique_ptr<ZapFR::Engine::Category>> getCategories() override;

            std::tuple<uint64_t, std::vector<std::unique_ptr<Log>>> getLogs(uint64_t perPage, uint64_t page) override;
            void clearLogs() override;

            void update(const std::string& newTitle) override;
            std::tuple<const std::unordered_map<uint64_t, uint64_t>, const std::unordered_map<uint64_t, uint64_t>> sort(SortMethod sortMethod) override;

            static std::unique_ptr<Folder> fromJSON(Source* parentSource, const Poco::JSON::Object::Ptr o);
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FOLDERREMOTE_H