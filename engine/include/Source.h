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

#ifndef ZAPFR_ENGINE_SOURCE_H
#define ZAPFR_ENGINE_SOURCE_H

#include "Feed.h"
#include "Folder.h"
#include "Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class Database;

        class Source
        {
          public:
            explicit Source(uint64_t id) : mID(id) {}
            virtual ~Source() = default;

            uint64_t id() const noexcept { return mID; }
            std::string title() const noexcept { return mTitle; }
            uint64_t sortOrder() const noexcept { return mSortOrder; }
            std::string configData() const noexcept { return mConfigData; }

            void setTitle(const std::string& title) { mTitle = title; }
            void setSortOrder(uint64_t sortOrder) noexcept { mSortOrder = sortOrder; }
            void setConfigData(const std::string& configData) { mConfigData = configData; }

            virtual std::vector<std::unique_ptr<Feed>> getFeeds() = 0;
            virtual std::optional<std::unique_ptr<Feed>> getFeed(uint64_t feedID, bool fetchData = true) = 0;
            virtual uint64_t addFeed(const std::string& url, uint64_t folder) = 0;
            virtual void moveFeed(uint64_t feedID, uint64_t newFolder, uint64_t newSortOrder) = 0;
            virtual void removeFeed(uint64_t feedID) = 0;

            virtual std::vector<std::unique_ptr<Folder>> getFolders(uint64_t parent) = 0;
            virtual std::optional<std::unique_ptr<Folder>> getFolder(uint64_t folderID) = 0;
            virtual void getSubfolderIDs(uint64_t parent, std::vector<uint64_t>& ids, bool includeParent = true) = 0;
            virtual uint64_t addFolder(const std::string& title, uint64_t parentID) = 0;
            virtual void removeFolder(uint64_t folder) = 0;
            virtual void moveFolder(uint64_t folderID, uint64_t newParent, uint64_t newSortOrder) = 0;
            virtual uint64_t createFolderHierarchy(uint64_t parentID, const std::vector<std::string> folderHierarchy) = 0;

            virtual std::vector<std::unique_ptr<Post>> getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread) = 0;
            virtual uint64_t getTotalPostCount(bool showOnlyUnread) = 0;
            virtual void markAllAsRead() = 0;

            static std::optional<std::unique_ptr<Source>> getSource(uint64_t sourceID);
            static std::vector<std::unique_ptr<Source>> getSources(std::optional<std::string> typeFilter);
            static void registerDatabaseInstance(Database* db);
            static std::unique_ptr<ZapFR::Engine::Source> createSourceInstance(uint64_t id, const std::string& type);

          protected:
            uint64_t mID{0};
            std::string mTitle{""};
            uint64_t mSortOrder{0};
            std::string mConfigData{""};

            static Database* database() noexcept;
            static Database* msDatabase;
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_SOURCE_H