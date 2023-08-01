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

#include "Global.h"
#include "Post.h"

namespace ZapFR
{
    namespace Engine
    {
        class Database;
        class Feed
        {
          public:
            explicit Feed(uint64_t id);
            virtual ~Feed() = default;

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
            virtual std::string icon() const { return ""; }
            std::string iconHash() const { return mIconHash; }
            std::string iconLastFetched() const noexcept { return mIconLastFetched; }
            std::string lastChecked() const noexcept { return mLastChecked; }
            uint64_t sortOrder() const noexcept { return mSortOrder; }
            uint64_t unreadCount() const noexcept { return mUnreadCount; }

            void setURL(const std::string& url) { mURL = url; }
            void setIconURL(const std::string& iconURL) { mIconURL = iconURL; }
            void setIconHash(const std::string& iconHash) { mIconHash = iconHash; }
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

            virtual std::vector<std::unique_ptr<Post>> getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread) = 0;
            virtual std::optional<std::unique_ptr<Post>> getPost(uint64_t postID) = 0;
            virtual uint64_t getTotalPostCount(bool showOnlyUnread) = 0;

            virtual void refresh() = 0;
            virtual void markAllAsRead() = 0;
            virtual void markAsRead(uint64_t postID) = 0;
            virtual void markAsUnread(uint64_t postID) = 0;
            virtual void refreshIcon() = 0;
            virtual void removeIcon() = 0;

            virtual bool fetchData() = 0;
            void setDataFetched(bool b) { mDataFetched = b; }

            static void registerDatabaseInstance(Database* db);

          protected:
            uint64_t mID{0};
            std::string mURL{""};
            std::string mIconURL{""};
            std::string mIconHash{""};
            std::string mIconLastFetched{""};
            uint64_t mFolderID{0};
            std::string mGuid{""};
            std::string mTitle{""};
            std::string mSubtitle{""};
            std::string mLink{""};
            std::string mDescription{""};
            std::string mLanguage{""};
            std::string mCopyright{""};
            std::string mLastChecked{""};
            uint64_t mSortOrder{0};
            uint64_t mUnreadCount{0};

            bool mDataFetched{false};

            static Database* database() noexcept;
            static Database* msDatabase;
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FEED_H