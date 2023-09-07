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

#include "../Feed.h"

namespace ZapFR
{
    namespace Engine
    {
        class FeedParser;

        class FeedLocal : public Feed
        {
          public:
            FeedLocal(uint64_t id, Source* parentSource);
            virtual ~FeedLocal() = default;

            std::tuple<uint64_t, std::vector<std::unique_ptr<Post>>> getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter,
                                                                              FlagColor flagColor) override;
            std::optional<std::unique_ptr<Post>> getPost(uint64_t postID) override;

            std::tuple<uint64_t, std::vector<std::unique_ptr<Log>>> getLogs(uint64_t perPage, uint64_t page) override;

            void fetchData();
            void fetchStatistics();
            bool refresh() override;
            void markAllAsRead() override;
            void markAsRead(uint64_t postID);
            void markAsUnread(uint64_t postID);
            void refreshIcon();
            void removeIcon();

            void processItems(FeedParser* parsedFeed);
            void fetchUnreadCount();

            static void setIconDir(const std::string& iconDir);

            static std::vector<std::unique_ptr<Feed>> queryMultiple(Source* parentSource, const std::vector<std::string>& whereClause, const std::string& orderClause,
                                                                    const std::string& limitClause, const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings,
                                                                    uint32_t fetchInfo);
            static std::optional<std::unique_ptr<ZapFR::Engine::Feed>> querySingle(Source* parentSource, const std::vector<std::string>& whereClause,
                                                                                   const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings, uint32_t fetchInfo);
            static uint64_t nextSortOrder(uint64_t folderID);
            static std::unique_ptr<FeedLocal> create(Source* parentSource, const std::string& url, const std::string& title, uint64_t parentFolderID);
            static void move(uint64_t feedID, uint64_t newFolder, uint64_t newSortOrder);
            static void resort(uint64_t folder);
            static void remove(Source* parentSource, uint64_t feedID);

            void updateProperties(const std::string& feedURL, std::optional<uint64_t> refreshIntervalInSeconds) override;

            void update(const std::string& iconURL, const std::string& guid, const std::string& title, const std::string& subtitle, const std::string& link,
                        const std::string& description, const std::string& language, const std::string& copyright);

          private:
            static std::string msIconDir;
            static std::mutex msCreateFeedMutex;
            static Poco::File iconFile(uint64_t feedID);

            std::optional<std::unique_ptr<Post>> getPostByGuid(const std::string& guid);
            void updateAndLogLastRefreshError(const std::string& error);
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FEEDLOCAL_H
