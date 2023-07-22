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
        class Database;
        class FeedParser;

        class FeedLocal : public Feed
        {
          public:
            explicit FeedLocal(uint64_t id);
            virtual ~FeedLocal() = default;

            std::vector<std::unique_ptr<Post>> getPosts(uint64_t perPage, uint64_t page) override;
            std::optional<std::unique_ptr<Post>> getPost(uint64_t postID) override;
            bool fetchData() override;
            void refresh() override;
            void markAllAsRead() override;
            void markAsRead(uint64_t postID) override;
            void refreshIcon() override;
            void removeIcon() override;

            std::string icon() const override;

            void processItems(FeedParser* parsedFeed);

            static void setIconDir(const std::string& iconDir);

          private:
            static std::string msIconDir;

            Poco::File iconFile() const;
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FEEDLOCAL_H