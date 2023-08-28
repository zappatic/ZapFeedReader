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

#include "Folder.h"
#include "Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class FolderLocal : public Folder
        {
          public:
            explicit FolderLocal(uint64_t id, uint64_t parent);
            virtual ~FolderLocal() = default;

            std::vector<std::unique_ptr<Post>> getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter, FlagColor flagColor) override;
            uint64_t getTotalPostCount(bool showOnlyUnread, const std::string& searchFilter, FlagColor flagColor) override;
            std::unordered_set<uint64_t> markAllAsRead() override;

            std::vector<std::unique_ptr<Log>> getLogs(uint64_t perPage, uint64_t page) override;
            uint64_t getTotalLogCount() override;

            void fetchSubfolders() override;
            bool fetchData() override;
            void fetchStatistics() override;

            std::vector<uint64_t> folderAndSubfolderIDs() const override;
            std::vector<uint64_t> feedIDsInFoldersAndSubfolders() const override;

            static std::vector<std::unique_ptr<Folder>> queryMultiple(const std::vector<std::string>& whereClause, const std::string& orderClause,
                                                                      const std::string& limitClause, const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings);
            static std::optional<std::unique_ptr<Folder>> querySingle(const std::vector<std::string>& whereClause,
                                                                      const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings);
            static uint64_t nextSortOrder(uint64_t folderID);
            static uint64_t create(uint64_t parentID, const std::string& title);
            static void remove(uint64_t folderID);
            static void resort(uint64_t parentID);
            static uint64_t createFolderHierarchy(uint64_t parentID, const std::vector<std::string> folderHierarchy);
            static void move(uint64_t folderID, uint64_t newParent, uint64_t newSortOrder);

          private:
            static std::mutex msCreateFolderMutex;
            static std::mutex msCreateFolderHierarchyMutex;
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FOLDERLOCAL_H