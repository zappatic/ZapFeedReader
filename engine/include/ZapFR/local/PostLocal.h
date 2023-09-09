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

#ifndef ZAPFR_ENGINE_POSTLOCAL_H
#define ZAPFR_ENGINE_POSTLOCAL_H

#include "ZapFR/base/Post.h"

namespace ZapFR
{
    namespace Engine
    {
        class PostLocal : public Post
        {
          public:
            explicit PostLocal(uint64_t id);
            ~PostLocal() = default;

            void markFlagged(FlagColor flagColor);
            void markUnflagged(FlagColor flagColor);

            void markAsRead();
            void markAsUnread();

            void assignToScriptFolder(uint64_t scriptFolderID);
            void unassignFromScriptFolder(uint64_t scriptFolderID);

            void update(const std::string& title, const std::string& link, const std::string& description, const std::string& author, const std::string& commentsURL,
                        const std::string& guid, const std::string& datePublished, const std::vector<Enclosure>& enclosures);

            static std::vector<std::unique_ptr<Post>> queryMultiple(const std::vector<std::string>& whereClause, const std::string& orderClause,
                                                                    const std::string& limitClause, const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings);
            static std::optional<std::unique_ptr<Post>> querySingle(const std::vector<std::string>& whereClause,
                                                                    const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings);
            static uint64_t queryCount(const std::vector<std::string>& whereClause, const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings);

            static void updateIsRead(bool isRead, const std::vector<std::string>& whereClause, const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings);

            static std::unique_ptr<Post> create(uint64_t feedID, const std::string& feedTitle, const std::string& title, const std::string& link,
                                                const std::string& description, const std::string& author, const std::string& commentsURL, const std::string& guid,
                                                const std::string& datePublished, const std::vector<Enclosure>& enclosures);

          private:
            static std::mutex msCreatePostMutex;

            static void replaceEnclosures(uint64_t postID, const std::vector<Enclosure>& enclosures);
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_POSTLOCAL_H
