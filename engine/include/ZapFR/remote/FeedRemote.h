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

#ifndef ZAPFR_ENGINE_FEEDREMOTE_H
#define ZAPFR_ENGINE_FEEDREMOTE_H

#include "../Feed.h"

namespace ZapFR
{
    namespace Engine
    {
        class FeedRemote : public Feed
        {
          public:
            explicit FeedRemote(uint64_t id);
            virtual ~FeedRemote() = default;

            std::vector<std::unique_ptr<Post>> getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter, FlagColor flagColor) override;
            uint64_t getTotalPostCount(bool showOnlyUnread, const std::string& searchFilter, FlagColor flagColor) override;
            std::optional<std::unique_ptr<Post>> getPost(uint64_t postID) override;

            std::vector<std::unique_ptr<Log>> getLogs(uint64_t perPage, uint64_t page) override;
            uint64_t getTotalLogCount() override;

            bool refresh(const std::optional<std::string>& feedXML) override;
            void markAllAsRead() override;
            void markAsRead(uint64_t postID) override;
            void markAsUnread(uint64_t postID) override;
            void refreshIcon() override;
            void removeIcon() override;

            std::string icon() const override;

            void updateProperties(const std::string& feedURL, std::optional<uint64_t> refreshIntervalInSeconds) override;
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FEEDREMOTE_H
