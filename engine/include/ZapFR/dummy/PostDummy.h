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

#ifndef ZAPFR_ENGINE_POSTDUMMY_H
#define ZAPFR_ENGINE_POSTDUMMY_H

#include "ZapFR/base/Post.h"

namespace ZapFR
{
    namespace Engine
    {
        class PostDummy : public Post
        {
          public:
            explicit PostDummy(uint64_t id);
            ~PostDummy() = default;

            void setLogCallback(const std::function<void(const std::string&)>& cb) { mLogCallback = cb; }

            void markFlagged(FlagColor flagColor);
            void markUnflagged(FlagColor flagColor);

            void markAsRead();
            void markAsUnread();

            void assignToScriptFolder(uint64_t scriptFolderID);
            void unassignFromScriptFolder(uint64_t scriptFolderID);

            static std::unique_ptr<PostDummy> createFromJSON(const Poco::JSON::Object::Ptr o);

          private:
            std::optional<std::function<void(const std::string&)>> mLogCallback{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_POSTDUMMY_H
