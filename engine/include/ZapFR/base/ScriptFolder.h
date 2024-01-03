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

#ifndef ZAPFR_ENGINE_SCRIPTFOLDER_H
#define ZAPFR_ENGINE_SCRIPTFOLDER_H

#include <Poco/JSON/Object.h>

#include "ZapFR/Flag.h"
#include "ZapFR/Global.h"
#include "ZapFR/base/Category.h"

namespace ZapFR
{
    namespace Engine
    {
        class Post;
        class Source;

        class ScriptFolder
        {
          public:
            ScriptFolder(uint64_t id, Source* parentSource);
            virtual ~ScriptFolder() = default;
            ScriptFolder(const ScriptFolder& e) = delete;
            ScriptFolder& operator=(const ScriptFolder&) = delete;
            ScriptFolder(ScriptFolder&&) = delete;
            ScriptFolder& operator=(ScriptFolder&&) = delete;

            const uint64_t& id() const noexcept { return mID; }
            const std::string& title() const noexcept { return mTitle; }
            bool showTotal() const noexcept { return mShowTotal; }
            bool showUnread() const noexcept { return mShowUnread; }
            const uint64_t& totalPostCount() const noexcept { return mTotalPostCount; }
            const uint64_t& totalUnreadCount() const noexcept { return mTotalUnreadCount; }
            const std::vector<ThumbnailData>& thumbnailData() { return mThumbnailData; }

            void setTitle(const std::string& title) { mTitle = title; }
            void setShowTotal(bool b) { mShowTotal = b; }
            void setShowUnread(bool b) { mShowUnread = b; }
            void setTotalPostCount(uint64_t p) { mTotalPostCount = p; }
            void setTotalUnreadCount(uint64_t p) { mTotalUnreadCount = p; }

            virtual std::tuple<uint64_t, std::vector<std::unique_ptr<Post>>> getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, bool showUnreadPostsAtTop,
                                                                                      const std::string& searchFilter, uint64_t categoryFilterID, FlagColor flagColor) = 0;

            virtual std::vector<std::unique_ptr<ZapFR::Engine::Category>> getCategories() = 0;

            virtual std::vector<uint64_t> markAsRead(uint64_t maxPostID) = 0;
            virtual void update(const std::string& title, bool showTotal, bool showUnread) = 0;

            virtual Poco::JSON::Object toJSON();

          protected:
            uint64_t mID{0};
            Source* mParentSource{nullptr};
            std::string mTitle{""};
            bool mShowTotal{false};
            bool mShowUnread{false};
            uint64_t mTotalPostCount{0};
            uint64_t mTotalUnreadCount{0};
            std::vector<ThumbnailData> mThumbnailData{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_SCRIPTFOLDER_H