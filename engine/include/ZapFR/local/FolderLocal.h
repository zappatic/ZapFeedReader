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

#ifndef ZAPFR_ENGINE_FOLDERLOCAL_H
#define ZAPFR_ENGINE_FOLDERLOCAL_H

#include <Poco/Data/AbstractBinding.h>

#include "ZapFR/base/Folder.h"

namespace ZapFR
{
    namespace Engine
    {
        class FolderLocal : public Folder
        {
          public:
            FolderLocal(uint64_t id, uint64_t parentFolderID, Source* parentSource);
            virtual ~FolderLocal() = default;

            std::tuple<uint64_t, std::vector<std::unique_ptr<Post>>> getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, bool showUnreadPostsAtTop,
                                                                              const std::string& searchFilter, uint64_t categoryFilterID, FlagColor flagColor) override;
            std::vector<uint64_t> markAsRead(uint64_t maxPostID) override;

            std::tuple<uint64_t, std::vector<std::unique_ptr<Log>>> getLogs(uint64_t perPage, uint64_t page) override;
            void clearLogs() override;

            std::vector<std::unique_ptr<ZapFR::Engine::Category>> getCategories() override;

            void update(const std::string& newTitle) override;
            std::tuple<const std::unordered_map<uint64_t, uint64_t>, const std::unordered_map<uint64_t, uint64_t>> sort(SortMethod sortMethod) override;

            void fetchSubfolders();
            void fetchStatistics();
            void fetchThumbnailData();

            std::vector<uint64_t> folderAndSubfolderIDs() const;
            const std::vector<uint64_t>& feedIDsInFoldersAndSubfolders() override;
            void fetchFeedIDsInFoldersAndSubfolders();

            static std::vector<std::unique_ptr<Folder>> queryMultiple(Source* parentSource, const std::vector<std::string>& whereClause, const std::string& orderClause,
                                                                      const std::string& limitClause, const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings);
            static std::optional<std::unique_ptr<Folder>> querySingle(Source* parentSource, const std::vector<std::string>& whereClause,
                                                                      const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings);
            static uint64_t nextSortOrder(uint64_t folderID);
            static std::tuple<uint64_t, uint64_t> create(uint64_t parentID, const std::string& title);
            static void remove(Source* parentSource, uint64_t folderID);
            static void resort(uint64_t parentID);
            static uint64_t createFolderHierarchy(Source* parentSource, uint64_t parentID, const std::vector<std::string> folderHierarchy);
            static std::unordered_map<uint64_t, uint64_t> move(Source* parentSource, uint64_t folderID, uint64_t newParent, uint64_t newSortOrder);

          private:
            mutable bool mFeedIDsFetched{false};

            static std::mutex msCreateFolderMutex;
            static std::mutex msCreateFolderHierarchyMutex;
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FOLDERLOCAL_H