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

#ifndef ZAPFR_ENGINE_FOLDER_H
#define ZAPFR_ENGINE_FOLDER_H

#include "Flag.h"
#include "Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class Post;
        class Log;

        class Folder
        {
          public:
            Folder(uint64_t id, uint64_t parent);
            virtual ~Folder() = default;

            enum class Statistic
            {
                FeedCount,
                PostCount,
                FlaggedPostCount,
                OldestPost,
                NewestPost,
            };

            uint64_t id() const noexcept { return mID; }
            uint64_t parentID() const noexcept { return mParent; }
            std::string title() const noexcept { return mTitle; }
            uint64_t sortOrder() const noexcept { return mSortOrder; }
            std::unordered_map<Statistic, std::string> statistics() { return mStatistics; }

            void setTitle(const std::string& title) { mTitle = title; }
            void setSortOrder(uint64_t sortOrder) { mSortOrder = sortOrder; }

            std::vector<Folder*> subfolders();
            virtual void fetchSubfolders() = 0;
            bool hasSubfolders();

            virtual std::vector<std::unique_ptr<Post>> getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter,
                                                                FlagColor flagColor) = 0;
            virtual uint64_t getTotalPostCount(bool showOnlyUnread, const std::string& searchFilter, FlagColor flagColor) = 0;
            virtual std::unordered_set<uint64_t> markAllAsRead() = 0;

            virtual std::vector<std::unique_ptr<Log>> getLogs(uint64_t perPage, uint64_t page) = 0;
            virtual uint64_t getTotalLogCount() = 0;

            virtual bool fetchData() = 0;
            virtual void fetchStatistics() = 0;
            void setDataFetched(bool b) { mDataFetched = b; }

            virtual std::vector<uint64_t> folderAndSubfolderIDs() const = 0;
            virtual std::vector<uint64_t> feedIDsInFoldersAndSubfolders() const = 0;

          protected:
            uint64_t mID{0};
            std::string mTitle{""};
            uint64_t mParent{0};
            uint64_t mSortOrder{0};
            std::vector<std::unique_ptr<Folder>> mSubfolders{};
            std::unordered_map<Statistic, std::string> mStatistics{};

            bool mDataFetched{false};
            bool mSubfoldersFetched{false};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FOLDER_H