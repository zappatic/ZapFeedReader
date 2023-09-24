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

#include "ZapFR/Flag.h"
#include "ZapFR/Global.h"

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
            const std::unordered_set<FlagColor>& flagColors() { return mFlagColors; }
            const std::vector<Enclosure>& enclosures() { return mEnclosures; }

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
            void setFlagColors(const std::unordered_set<FlagColor>& flagColors) { mFlagColors = flagColors; }
            void addEnclosure(const Enclosure& enclosure) { mEnclosures.emplace_back(enclosure); }
            void addEnclosure(const std::string& url, const std::string& mimeType, uint64_t size) { mEnclosures.emplace_back(url, mimeType, size); }
            void updateEnclosure(uint64_t index, const std::string& url, const std::string& mimeType, uint64_t size);
            void removeEnclosure(uint64_t index);

            virtual Poco::JSON::Object toJSON();
            virtual void fromJSON(const Poco::JSON::Object::Ptr o);
            static constexpr const char* JSONIdentifierPostID{"id"};
            static constexpr const char* JSONIdentifierPostIsRead{"isRead"};
            static constexpr const char* JSONIdentifierPostFeedID{"feedID"};
            static constexpr const char* JSONIdentifierPostFeedTitle{"feedTitle"};
            static constexpr const char* JSONIdentifierPostFeedLink{"feedLink"};
            static constexpr const char* JSONIdentifierPostTitle{"title"};
            static constexpr const char* JSONIdentifierPostLink{"link"};
            static constexpr const char* JSONIdentifierPostContent{"content"};
            static constexpr const char* JSONIdentifierPostAuthor{"author"};
            static constexpr const char* JSONIdentifierPostCommentsURL{"commentsURL"};
            static constexpr const char* JSONIdentifierPostGuid{"guid"};
            static constexpr const char* JSONIdentifierPostDatePublished{"datePublished"};
            static constexpr const char* JSONIdentifierPostFlagColors{"flagColors"};
            static constexpr const char* JSONIdentifierPostEnclosures{"enclosures"};
            static constexpr const char* JSONIdentifierPostEnclosureURL{"url"};
            static constexpr const char* JSONIdentifierPostEnclosureMimeType{"mimeType"};
            static constexpr const char* JSONIdentifierPostEnclosureSize{"size"};

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
            std::unordered_set<FlagColor> mFlagColors{};
            std::vector<Enclosure> mEnclosures{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_POST_H