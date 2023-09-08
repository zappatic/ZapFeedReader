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

#ifndef ZAPFR_ENGINE_FEED_H
#define ZAPFR_ENGINE_FEED_H

#include "ZapFR/Flag.h"
#include "ZapFR/Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class Post;
        class Source;
        class Log;

        class Feed
        {
          public:
            Feed(uint64_t id, Source* parentSource);
            virtual ~Feed() = default;
            Feed(const Feed& e) = delete;
            Feed& operator=(const Feed&) = delete;
            Feed(Feed&&) = delete;
            Feed& operator=(Feed&&) = delete;

            enum class Statistic
            {
                PostCount,
                FlaggedPostCount,
                OldestPost,
                NewestPost,
            };

            uint64_t id() const noexcept { return mID; }
            std::string url() const noexcept { return mURL; }
            uint64_t folder() const noexcept { return mFolderID; }
            std::string guid() const noexcept { return mGuid; }
            std::string title() const noexcept { return mTitle; }
            std::string subtitle() const noexcept { return mSubtitle; }
            std::string link() const noexcept { return mLink; }
            std::string description() const noexcept { return mDescription; }
            std::string language() const noexcept { return mLanguage; }
            std::string copyright() const noexcept { return mCopyright; }
            std::string iconURL() const noexcept { return mIconURL; }
            std::string iconData() const { return mIconData; }
            std::string iconHash() const { return mIconHash; }
            std::string iconLastFetched() const noexcept { return mIconLastFetched; }
            std::string lastChecked() const noexcept { return mLastChecked; }
            uint64_t sortOrder() const noexcept { return mSortOrder; }
            uint64_t unreadCount() const noexcept { return mUnreadCount; }
            std::optional<std::string> lastRefreshError() { return mLastRefreshError; }
            std::optional<uint64_t> refreshInterval() { return mRefreshInterval; }
            std::unordered_map<Statistic, std::string> statistics() { return mStatistics; }

            void setURL(const std::string& url) { mURL = url; }
            void setIconURL(const std::string& iconURL) { mIconURL = iconURL; }
            void setIconHash(const std::string& iconHash) { mIconHash = iconHash; }
            void setIconData(const std::string& iconData) { mIconData = iconData; }
            void setIconLastFetched(const std::string& iconLastFetched) { mIconLastFetched = iconLastFetched; }
            void setFolder(uint64_t folder) { mFolderID = folder; }
            void setGuid(const std::string& guid) { mGuid = guid; }
            void setTitle(const std::string& title) { mTitle = title; }
            void setSubtitle(const std::string& subtitle) { mSubtitle = subtitle; }
            void setLink(const std::string& link) { mLink = link; }
            void setDescription(const std::string& description) { mDescription = description; }
            void setLanguage(const std::string& language) { mLanguage = language; }
            void setCopyright(const std::string& copyright) { mCopyright = copyright; }
            void setLastChecked(const std::string& lastChecked) { mLastChecked = lastChecked; }
            void setSortOrder(uint64_t sortOrder) noexcept { mSortOrder = sortOrder; }
            void setUnreadCount(uint64_t unreadCount) noexcept { mUnreadCount = unreadCount; }
            void setLastRefreshError(const std::string& e) { mLastRefreshError = e; }
            void setRefreshInterval(std::optional<uint64_t> ri) { mRefreshInterval = ri; }
            void setStatistics(const std::unordered_map<Statistic, std::string>& stats) { mStatistics = stats; }

            virtual std::tuple<uint64_t, std::vector<std::unique_ptr<Post>>> getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter,
                                                                                      FlagColor flagColor) = 0;
            virtual std::optional<std::unique_ptr<Post>> getPost(uint64_t postID) = 0;

            virtual std::tuple<uint64_t, std::vector<std::unique_ptr<Log>>> getLogs(uint64_t perPage, uint64_t page) = 0;

            virtual void updateProperties(const std::string& feedURL, std::optional<uint64_t> refreshIntervalInSeconds) = 0;
            virtual bool refresh() = 0;
            virtual void markAllAsRead() = 0;

            void setDataFetched(bool b) { mDataFetched = b; }

            virtual Poco::JSON::Object toJSON() const;
            static constexpr const char* JSONIdentifierFeedID{"id"};
            static constexpr const char* JSONIdentifierFeedURL{"url"};
            static constexpr const char* JSONIdentifierFeedFolder{"folder"};
            static constexpr const char* JSONIdentifierFeedGUID{"guid"};
            static constexpr const char* JSONIdentifierFeedIcon{"icon"};
            static constexpr const char* JSONIdentifierFeedTitle{"title"};
            static constexpr const char* JSONIdentifierFeedSubtitle{"subtitle"};
            static constexpr const char* JSONIdentifierFeedLink{"link"};
            static constexpr const char* JSONIdentifierFeedDescription{"description"};
            static constexpr const char* JSONIdentifierFeedLanguage{"language"};
            static constexpr const char* JSONIdentifierFeedCopyright{"copyright"};
            static constexpr const char* JSONIdentifierFeedLastRefreshError{"lastRefreshError"};
            static constexpr const char* JSONIdentifierFeedRefreshInterval{"refreshInterval"};
            static constexpr const char* JSONIdentifierFeedLastChecked{"lastChecked"};
            static constexpr const char* JSONIdentifierFeedSortOrder{"sortOrder"};
            static constexpr const char* JSONIdentifierFeedUnreadCount{"unreadCount"};
            static constexpr const char* JSONIdentifierFeedStatistics{"statistics"};
            static const std::unordered_map<std::string, Statistic> JSONIdentifierFeedStatisticMap;
            static const std::unordered_map<Statistic, std::string> FeedStatisticJSONIdentifierMap;

          protected:
            uint64_t mID{0};
            Source* mParentSource{nullptr};
            std::string mURL{""};
            std::string mIconURL{""};
            std::string mIconHash{""};
            std::string mIconLastFetched{""};
            std::string mIconData{""};
            uint64_t mFolderID{0};
            std::string mGuid{""};
            std::string mTitle{""};
            std::string mSubtitle{""};
            std::string mLink{""};
            std::string mDescription{""};
            std::string mLanguage{""};
            std::string mCopyright{""};
            std::string mLastChecked{""};
            std::optional<std::string> mLastRefreshError{};
            std::optional<uint64_t> mRefreshInterval{};
            uint64_t mSortOrder{0};
            uint64_t mUnreadCount{0};
            std::unordered_map<Statistic, std::string> mStatistics{};

            bool mDataFetched{false};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FEED_H