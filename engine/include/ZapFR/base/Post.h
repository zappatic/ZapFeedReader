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

            uint64_t id() const noexcept { return mID; }
            uint64_t feedID() const noexcept { return mFeedID; }
            std::string feedTitle() const noexcept { return mFeedTitle; }
            bool isRead() const noexcept { return mIsRead; }
            std::string title() const noexcept { return mTitle; }
            std::string link() const noexcept { return mLink; }
            std::string description() const noexcept { return mDescription; }
            std::string author() const noexcept { return mAuthor; }
            std::string commentsURL() const noexcept { return mCommentsURL; }
            std::string enclosureURL() const noexcept { return mEnclosureURL; }
            std::string enclosureLength() const noexcept { return mEnclosureLength; }
            std::string enclosureMimeType() const noexcept { return mEnclosureMimeType; }
            std::string guid() const noexcept { return mGuid; }
            bool guidIsPermalink() const noexcept { return mGuidIsPermalink; }
            std::string datePublished() const noexcept { return mDatePublished; }
            std::string sourceURL() const noexcept { return mSourceURL; }
            std::string sourceTitle() const noexcept { return mSourceTitle; }
            std::unordered_set<FlagColor> flagColors() { return mFlagColors; }

            void setIsRead(bool b) { mIsRead = b; }
            void setFeedID(uint64_t feedID) { mFeedID = feedID; }
            void setFeedTitle(const std::string& title) { mFeedTitle = title; }
            void setTitle(const std::string& title) { mTitle = title; }
            void setLink(const std::string& link) { mLink = link; }
            void setDescription(const std::string& description) { mDescription = description; }
            void setAuthor(const std::string& author) { mAuthor = author; }
            void setCommentsURL(const std::string& commentsURL) { mCommentsURL = commentsURL; }
            void setEnclosureURL(const std::string& enclosureURL) { mEnclosureURL = enclosureURL; }
            void setEnclosureLength(const std::string& enclosureLength) { mEnclosureLength = enclosureLength; }
            void setEnclosureMimeType(const std::string& enclosureMimeType) { mEnclosureMimeType = enclosureMimeType; }
            void setGuid(const std::string& guid) { mGuid = guid; }
            void setGuidIsPermalink(bool b) { mGuidIsPermalink = b; }
            void setDatePublished(const std::string& datePublished) { mDatePublished = datePublished; }
            void setSourceURL(const std::string& sourceURL) { mSourceURL = sourceURL; }
            void setSourceTitle(const std::string& sourceTitle) { mSourceTitle = sourceTitle; }
            void setFlagColors(const std::unordered_set<FlagColor>& flagColors) { mFlagColors = flagColors; }

            virtual Poco::JSON::Object toJSON();
            static constexpr const char* JSONIdentifierPostID{"id"};
            static constexpr const char* JSONIdentifierPostIsRead{"isRead"};
            static constexpr const char* JSONIdentifierPostFeedID{"feedID"};
            static constexpr const char* JSONIdentifierPostFeedTitle{"feedTitle"};
            static constexpr const char* JSONIdentifierPostTitle{"title"};
            static constexpr const char* JSONIdentifierPostLink{"link"};
            static constexpr const char* JSONIdentifierPostDescription{"description"};
            static constexpr const char* JSONIdentifierPostAuthor{"author"};
            static constexpr const char* JSONIdentifierPostCommentsURL{"commentsURL"};
            static constexpr const char* JSONIdentifierPostEnclosureURL{"enclosureURL"};
            static constexpr const char* JSONIdentifierPostEnclosureLength{"enclosureLength"};
            static constexpr const char* JSONIdentifierPostEnclosureMimetype{"enclosureMimetype"};
            static constexpr const char* JSONIdentifierPostGuid{"guid"};
            static constexpr const char* JSONIdentifierPostGuidIsPermalink{"guidIsPermalink"};
            static constexpr const char* JSONIdentifierPostDatePublished{"datePublished"};
            static constexpr const char* JSONIdentifierPostSourceURL{"sourceURL"};
            static constexpr const char* JSONIdentifierPostSourceTitle{"sourceTitle"};
            static constexpr const char* JSONIdentifierPostFlagColors{"flagColors"};

          protected:
            uint64_t mID{0};
            bool mIsRead{false};
            uint64_t mFeedID{0};
            std::string mFeedTitle{""};
            std::string mTitle{""};
            std::string mLink{""};
            std::string mDescription{""};
            std::string mAuthor{""};
            std::string mCommentsURL{""};
            std::string mEnclosureURL{""};
            std::string mEnclosureLength{""};
            std::string mEnclosureMimeType{""};
            std::string mGuid{""};
            bool mGuidIsPermalink{false};
            std::string mDatePublished{""};
            std::string mSourceURL{""};
            std::string mSourceTitle{""};
            std::unordered_set<FlagColor> mFlagColors{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_POST_H