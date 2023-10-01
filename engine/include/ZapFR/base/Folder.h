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

#include <Poco/JSON/Object.h>

#include "ZapFR/Flag.h"
#include "ZapFR/Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class Post;
        class Source;
        class Log;

        class Folder
        {
          public:
            Folder(uint64_t id, uint64_t parentFolderID, Source* parentSource);
            virtual ~Folder() = default;
            Folder(const Folder& e) = delete;
            Folder& operator=(const Folder&) = delete;
            Folder(Folder&&) = delete;
            Folder& operator=(Folder&&) = delete;

            enum class Statistic : uint64_t
            {
                FeedCount,
                PostCount,
                FlaggedPostCount,
                OldestPost,
                NewestPost,
            };

            const uint64_t& id() const noexcept { return mID; }
            const uint64_t& parentID() const noexcept { return mParentFolderID; }
            const std::string& title() const noexcept { return mTitle; }
            const uint64_t& sortOrder() const noexcept { return mSortOrder; }
            const std::unordered_map<Statistic, std::string>& statistics() { return mStatistics; }
            virtual const std::vector<uint64_t>& feedIDsInFoldersAndSubfolders() { return mFeedIDs; }
            const std::vector<ThumbnailData>& thumbnailData() { return mThumbnailData; }

            void setTitle(const std::string& title) { mTitle = title; }
            void setSortOrder(uint64_t sortOrder) { mSortOrder = sortOrder; }
            void setStatistics(const std::unordered_map<Statistic, std::string>& stats) { mStatistics = stats; }
            void setFeedIDsInFoldersAndSubfolders(const std::vector<uint64_t>& feedIDs) { mFeedIDs = feedIDs; }

            const std::vector<std::unique_ptr<Folder>>& subfolders() { return mSubfolders; }
            void appendSubfolder(std::unique_ptr<Folder> subfolder);

            virtual std::tuple<uint64_t, std::vector<std::unique_ptr<Post>>> getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter,
                                                                                      FlagColor flagColor) = 0;
            virtual std::unordered_set<uint64_t> markAsRead() = 0;

            virtual std::tuple<uint64_t, std::vector<std::unique_ptr<Log>>> getLogs(uint64_t perPage, uint64_t page) = 0;
            virtual void clearLogs() = 0;

            virtual void update(const std::string& newTitle) = 0;
            virtual void sort(SortMethod sortMethod) = 0;

            void setDataFetched(bool b) { mDataFetched = b; }

            virtual Poco::JSON::Object toJSON();
            static const std::unordered_map<std::string, Statistic> JSONIdentifierFolderStatisticMap;
            static const std::unordered_map<Statistic, std::string> FolderStatisticJSONIdentifierMap;

          protected:
            uint64_t mID{0};
            uint64_t mParentFolderID{0};
            Source* mParentSource{nullptr};
            std::string mTitle{""};
            uint64_t mSortOrder{0};
            std::vector<std::unique_ptr<Folder>> mSubfolders{};
            std::unordered_map<Statistic, std::string> mStatistics{};
            std::vector<uint64_t> mFeedIDs{};
            std::vector<ThumbnailData> mThumbnailData{};

            bool mDataFetched{false};
            bool mSubfoldersFetched{false};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FOLDER_H