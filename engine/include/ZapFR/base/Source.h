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

#include <optional>

#include "Script.h"
#include "ZapFR/Flag.h"
#include "ZapFR/Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class Feed;
        class Folder;
        class Post;
        class Log;
        class ScriptFolder;

        class Source
        {
          public:
            explicit Source(uint64_t id) : mID(id) {}
            virtual ~Source() = default;

            enum class Statistic : uint64_t
            {
                FeedCount,
                PostCount,
                FlaggedPostCount,
                OldestPost,
                NewestPost,
            };

            enum FetchInfo
            {
                None = 0,
                Data = 1,
                Statistics = 2,
                Icon = 4,
                FeedUnreadCount = 8,
                FolderFeedIDs = 16,
                Subfolders = 32,
                UnreadThumbnailData = 64,
            };

            const uint64_t& id() const noexcept { return mID; }
            const std::string& title() const noexcept { return mTitle; }
            const std::string& type() const noexcept { return mType; }
            const uint64_t& sortOrder() const noexcept { return mSortOrder; }
            const std::string& configData() const noexcept { return mConfigData; }
            const std::vector<ThumbnailData>& thumbnailData() { return mThumbnailData; }
            const std::unordered_map<Statistic, std::string>& statistics() { return mStatistics; }
            const std::string& lastError() const noexcept { return mLastError; }

            void setTitle(const std::string& title) { mTitle = title; }
            void setType(const std::string& type) { mType = type; }
            void setSortOrder(uint64_t sortOrder) noexcept { mSortOrder = sortOrder; }
            void setConfigData(const std::string& configData) { mConfigData = configData; }
            void setLastError(const std::string& error) { mLastError = error; }
            void updateLastError(const std::string& error);

            void update(const std::string& newTitle, const std::string& newConfigData);
            virtual std::unordered_set<uint64_t> importOPML(const std::string& opml, uint64_t parentFolderID) = 0;
            virtual void fetchStatistics() = 0;
            virtual void fetchThumbnailData() = 0;
            virtual void clearLogs() = 0;

            virtual std::vector<std::unique_ptr<Feed>> getFeeds(uint32_t fetchInfo) = 0;
            virtual std::optional<std::unique_ptr<Feed>> getFeed(uint64_t feedID, uint32_t fetchInfo) = 0;
            virtual std::optional<std::unique_ptr<Feed>> addFeed(const std::string& url, uint64_t folder) = 0;
            virtual std::unordered_map<uint64_t, uint64_t> moveFeed(uint64_t feedID, uint64_t newFolder, uint64_t newSortOrder) = 0;
            virtual void removeFeed(uint64_t feedID) = 0;

            virtual std::vector<std::unique_ptr<Folder>> getFolders(uint64_t parent, uint32_t fetchInfo) = 0;
            virtual std::optional<std::unique_ptr<Folder>> getFolder(uint64_t folderID, uint32_t fetchInfo) = 0;
            virtual std::tuple<uint64_t, uint64_t> addFolder(const std::string& title, uint64_t parentID) = 0;
            virtual void removeFolder(uint64_t folderID) = 0;
            virtual std::unordered_map<uint64_t, uint64_t> moveFolder(uint64_t folderID, uint64_t newParent, uint64_t newSortOrder) = 0;

            virtual std::tuple<uint64_t, std::vector<std::unique_ptr<Post>>> getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter,
                                                                                      FlagColor flagColor) = 0;
            virtual void markAsRead() = 0;
            virtual void setPostsReadStatus(bool markAsRead, const std::vector<std::tuple<uint64_t, uint64_t>>& feedsAndPostIDs) = 0;
            virtual void setPostsFlagStatus(bool markFlagged, const std::unordered_set<FlagColor>& flagColors,
                                            const std::vector<std::tuple<uint64_t, uint64_t>>& feedsAndPostIDs) = 0;
            virtual void assignPostsToScriptFolder(uint64_t scriptFolderID, bool assign, const std::vector<std::tuple<uint64_t, uint64_t>>& feedsAndPostIDs) = 0;
            virtual Poco::JSON::Object getStatus() = 0;

            virtual std::tuple<uint64_t, std::vector<std::unique_ptr<Log>>> getLogs(uint64_t perPage, uint64_t page) = 0;

            virtual std::unordered_set<FlagColor> getUsedFlagColors() = 0;

            virtual std::vector<std::unique_ptr<ScriptFolder>> getScriptFolders() = 0;
            virtual std::optional<std::unique_ptr<ScriptFolder>> getScriptFolder(uint64_t id, uint32_t fetchInfo) = 0;
            virtual void addScriptFolder(const std::string& title, bool showTotal, bool showUnread) = 0;
            virtual void removeScriptFolder(uint64_t scriptFolderID) = 0;

            virtual std::vector<std::unique_ptr<Script>> getScripts() = 0;
            virtual std::optional<std::unique_ptr<Script>> getScript(uint64_t scriptID, uint32_t fetchInfo) = 0;
            virtual void removeScript(uint64_t scriptID) = 0;
            virtual void addScript(Script::Type type, const std::string& title, bool enabled, const std::unordered_set<Script::Event>& events,
                                   const std::optional<std::unordered_set<uint64_t>>& feedIDs, const std::string& script) = 0;

            static std::optional<std::unique_ptr<Source>> getSource(uint64_t sourceID);
            static std::vector<std::unique_ptr<Source>> getSources(std::optional<std::string> typeFilter);
            static std::unique_ptr<ZapFR::Engine::Source> createSourceInstance(uint64_t id, const std::string& type);
            static std::optional<std::unique_ptr<Source>> create(const std::string& type, const std::string& title, const std::string& configData);
            static void removeSource(uint64_t id);
            static uint64_t nextSortOrder();

            static const std::unordered_map<Statistic, std::string> SourceStatisticJSONIdentifierMap;

          protected:
            uint64_t mID{0};
            std::string mTitle{""};
            std::string mType{""};
            uint64_t mSortOrder{0};
            std::string mConfigData{""};
            std::vector<ThumbnailData> mThumbnailData{};
            std::unordered_map<Statistic, std::string> mStatistics{};
            std::string mLastError{""};

            static std::mutex msCreateSourceMutex;
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_SOURCE_H