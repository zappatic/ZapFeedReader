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

#ifndef ZAPFR_ENGINE_FEEDLOCAL_H
#define ZAPFR_ENGINE_FEEDLOCAL_H

#include "Feed.h"
#include "Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class FeedParser;

        class FeedLocal : public Feed
        {
          public:
            explicit FeedLocal(uint64_t id);
            virtual ~FeedLocal() = default;

            std::vector<std::unique_ptr<Post>> getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter, FlagColor flagColor) override;
            uint64_t getTotalPostCount(bool showOnlyUnread, const std::string& searchFilter, FlagColor flagColor) override;
            std::optional<std::unique_ptr<Post>> getPost(uint64_t postID) override;

            std::vector<std::unique_ptr<Log>> getLogs(uint64_t perPage, uint64_t page) override;
            uint64_t getTotalLogCount() override;

            bool fetchData() override;
            bool refresh(const std::optional<std::string>& feedXML) override;
            void markAllAsRead() override;
            void markAsRead(uint64_t postID) override;
            void markAsUnread(uint64_t postID) override;
            void refreshIcon() override;
            void removeIcon() override;

            std::string icon() const override;

            void processItems(FeedParser* parsedFeed);

            static void setIconDir(const std::string& iconDir);

            static std::vector<std::unique_ptr<Feed>> queryMultiple(const std::vector<std::string>& whereClause, const std::string& orderClause,
                                                                    const std::string& limitClause, const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings);
            static std::optional<std::unique_ptr<ZapFR::Engine::Feed>> querySingle(const std::vector<std::string>& whereClause,
                                                                                   const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings);
            static uint64_t nextSortOrder(uint64_t folderID);
            static std::unique_ptr<FeedLocal> create(const std::string& url, const std::string& title, uint64_t parentFolderID);
            static void move(uint64_t feedID, uint64_t newFolder, uint64_t newSortOrder);
            static void resort(uint64_t folder);
            static void remove(uint64_t feedID);

            void updateProperties(const std::string feedURL, std::optional<uint64_t> refreshIntervalInSeconds) override;

            void update(const std::string& iconURL, const std::string& guid, const std::string& title, const std::string& subtitle, const std::string& link,
                        const std::string& description, const std::string& language, const std::string& copyright);

          private:
            static std::string msIconDir;
            static std::mutex msCreateFeedMutex;

            Poco::File iconFile() const;
            std::optional<std::unique_ptr<Post>> getPostByGuid(const std::string& guid);
            void fetchUnreadCount();
            void updateAndLogLastRefreshError(const std::string& error);
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FEEDLOCAL_H
