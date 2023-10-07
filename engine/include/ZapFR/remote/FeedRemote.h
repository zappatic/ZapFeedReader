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

#include "ZapFR/base/Feed.h"

namespace ZapFR
{
    namespace Engine
    {
        class FeedRemote : public Feed
        {
          public:
            FeedRemote(uint64_t id, Source* parentSource);
            virtual ~FeedRemote() = default;

            std::tuple<uint64_t, std::vector<std::unique_ptr<Post>>> getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter,
                                                                              uint64_t categoryFilterID, FlagColor flagColor) override;
            std::optional<std::unique_ptr<Post>> getPost(uint64_t postID) override;

            std::tuple<uint64_t, std::vector<std::unique_ptr<Log>>> getLogs(uint64_t perPage, uint64_t page) override;
            void clearLogs() override;

            std::vector<std::unique_ptr<ZapFR::Engine::Category>> getCategories() override;

            void refresh() override;
            void markAsRead(uint64_t maxPostID) override;

            void updateProperties(const std::string& feedURL, std::optional<uint64_t> refreshIntervalInSeconds) override;
            void fromJSON(const Poco::JSON::Object::Ptr o);

            static std::unique_ptr<Feed> fromJSON(Source* parentSource, const Poco::JSON::Object::Ptr o);
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FEEDREMOTE_H
