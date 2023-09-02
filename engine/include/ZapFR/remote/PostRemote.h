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

#ifndef ZAPFR_ENGINE_POSTREMOTE_H
#define ZAPFR_ENGINE_POSTREMOTE_H

#include "../Post.h"

namespace ZapFR
{
    namespace Engine
    {
        class PostRemote : public Post
        {
          public:
            explicit PostRemote(uint64_t id);
            ~PostRemote() = default;

            void markFlagged(FlagColor flagColor) override;
            void markUnflagged(FlagColor flagColor) override;

            void markAsRead() override;
            void markAsUnread() override;

            void assignToScriptFolder(uint64_t scriptFolderID) override;
            void unassignFromScriptFolder(uint64_t scriptFolderID) override;

            static std::unique_ptr<Post> fromJSON(const Poco::JSON::Object::Ptr o);
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_POSTREMOTE_H
