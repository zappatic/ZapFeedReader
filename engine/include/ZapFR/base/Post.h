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

#ifndef ZAPFR_ENGINE_POST_H
#define ZAPFR_ENGINE_POST_H

#include <unordered_set>

#include <Poco/JSON/Object.h>

#include "ZapFR/Flag.h"

namespace ZapFR
{
    namespace Engine
    {
        class Post
        {
          public:
            explicit Post(uint64_t id);
            virtual ~Post() = default;

            struct Enclosure
            {
                Enclosure() = default;
                Enclosure(const std::string& enclosureURL, const std::string& enclosureMimetype, uint64_t enclosureSize)
                    : url(enclosureURL), mimeType(enclosureMimetype), size(enclosureSize)
                {
                }
                std::string url{""};
                std::string mimeType{""};
                uint64_t size{0};
            };

            struct Category
            {
                Category() = default;
                Category(uint64_t catID, const std::string& catTitle) : id(catID), title(catTitle) {}

                uint64_t id;
                std::string title;
            };

            const uint64_t& id() const noexcept { return mID; }
            const uint64_t& feedID() const noexcept { return mFeedID; }
            const std::string& feedTitle() const noexcept { return mFeedTitle; }
            const std::string& feedLink() const noexcept { return mFeedLink; }
            bool isRead() const noexcept { return mIsRead; }
            const std::string& title() const noexcept { return mTitle; }
            const std::string& link() const noexcept { return mLink; }
            const std::string& content() const noexcept { return mContent; }
            const std::string& author() const noexcept { return mAuthor; }
            const std::string& commentsURL() const noexcept { return mCommentsURL; }
            const std::string& guid() const noexcept { return mGuid; }
            const std::string& datePublished() const noexcept { return mDatePublished; }
            const std::string& thumbnail() const noexcept { return mThumbnail; }
            const std::unordered_set<FlagColor>& flagColors() { return mFlagColors; }
            const std::vector<Enclosure>& enclosures() { return mEnclosures; }
            const std::vector<Category>& categories() { return mCategories; }

            void setIsRead(bool b) { mIsRead = b; }
            void setFeedID(uint64_t feedID) { mFeedID = feedID; }
            void setFeedTitle(const std::string& title) { mFeedTitle = title; }
            void setFeedLink(const std::string& link) { mFeedLink = link; }
            void setTitle(const std::string& title) { mTitle = title; }
            void setLink(const std::string& link) { mLink = link; }
            void setContent(const std::string& content) { mContent = content; }
            void setAuthor(const std::string& author) { mAuthor = author; }
            void setCommentsURL(const std::string& commentsURL) { mCommentsURL = commentsURL; }
            void setGuid(const std::string& guid) { mGuid = guid; }
            void setDatePublished(const std::string& datePublished) { mDatePublished = datePublished; }
            void setThumbnail(const std::string& thumbnail) { mThumbnail = thumbnail; }
            void setFlagColors(const std::unordered_set<FlagColor>& flagColors) { mFlagColors = flagColors; }
            void addEnclosure(const Enclosure& enclosure) { mEnclosures.emplace_back(enclosure); }
            void addEnclosure(const std::string& url, const std::string& mimeType, uint64_t size) { mEnclosures.emplace_back(url, mimeType, size); }
            void updateEnclosure(uint64_t index, const std::string& url, const std::string& mimeType, uint64_t size);
            void removeEnclosure(uint64_t index); // TODO: removing by index is not good, needs to be on unique id, because if you loop over enclosures it messes up
            void addCategory(const Category& cat) { mCategories.emplace_back(cat); }
            bool hasCategory(const std::string& title) const;

            virtual Poco::JSON::Object toJSON();
            virtual void fromJSON(const Poco::JSON::Object::Ptr o);

          protected:
            uint64_t mID{0};
            bool mIsRead{false};
            uint64_t mFeedID{0};
            std::string mFeedTitle{""};
            std::string mFeedLink{""};
            std::string mTitle{""};
            std::string mLink{""};
            std::string mContent{""};
            std::string mAuthor{""};
            std::string mCommentsURL{""};
            std::string mGuid{""};
            std::string mDatePublished{""};
            std::string mThumbnail{""};
            std::unordered_set<FlagColor> mFlagColors{};
            std::vector<Enclosure> mEnclosures{};
            std::vector<Category> mCategories{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_POST_H