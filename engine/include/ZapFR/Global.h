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

#ifndef ZAPFR_ENGINE_GLOBAL_H
#define ZAPFR_ENGINE_GLOBAL_H

#include <memory>
#include <vector>

namespace ZapFR
{
    namespace Engine
    {
        enum class ApplicationType
        {
            Server,
            Client
        };

        enum class SortMethod
        {
            AlphabeticallyAscending,
        };

        struct ThumbnailDataPost
        {
            ThumbnailDataPost() = default;
            ThumbnailDataPost(uint64_t tdpPostID, const std::string& tdpTitle, const std::string& tdpThumbnail, const std::string& tdpLink, time_t tdpTimestamp)
                : postID(tdpPostID), title(tdpTitle), thumbnail(tdpThumbnail), link(tdpLink), timestamp(tdpTimestamp)
            {
            }
            uint64_t postID{0};
            std::string title{""};
            std::string thumbnail{""};
            std::string link{""};
            time_t timestamp{0};
        };

        struct ThumbnailData
        {
            uint64_t feedID{0};
            std::string feedTitle{""};
            std::vector<ThumbnailDataPost> posts{};
        };

        static const uint64_t DBVersion{2};
        static const uint64_t APIVersion{1};
        static const uint64_t DefaultFeedAutoRefreshInterval{15 * 60};
        static const uint16_t DefaultServerPort{16016};

        namespace ServerIdentifier
        {
            static const char* Local{"zapfeedreader.local"};
            static const char* Remote{"zapfeedreader.remote"};
        }; // namespace ServerIdentifier

        namespace JSON
        {
            namespace RemoteConfigData
            {
                static const char* Host{"host"};
                static const char* Port{"port"};
                static const char* Login{"login"};
                static const char* Password{"password"};
                static const char* UseHTTPS{"useHTTPS"};
            }; // namespace RemoteConfigData

            namespace ThumbnailData
            {
                static constexpr const char* FeedID{"feedID"};
                static constexpr const char* FeedTitle{"feedTitle"};
                static constexpr const char* PostLink{"link"};
                static constexpr const char* PostID{"postID"};
                static constexpr const char* PostThumbnail{"thumbnail"};
                static constexpr const char* PostTimestamp{"timestamp"};
                static constexpr const char* PostTitle{"title"};
                static constexpr const char* Posts{"posts"};
            }; // namespace ThumbnailData

            namespace Feed
            {
                static constexpr const char* ID{"id"};
                static constexpr const char* URL{"url"};
                static constexpr const char* Folder{"folder"};
                static constexpr const char* GUID{"guid"};
                static constexpr const char* Icon{"icon"};
                static constexpr const char* IconHash{"iconHash"};
                static constexpr const char* Title{"title"};
                static constexpr const char* Subtitle{"subtitle"};
                static constexpr const char* Link{"link"};
                static constexpr const char* Description{"description"};
                static constexpr const char* Language{"language"};
                static constexpr const char* Copyright{"copyright"};
                static constexpr const char* LastRefreshError{"lastRefreshError"};
                static constexpr const char* RefreshInterval{"refreshInterval"};
                static constexpr const char* LastChecked{"lastChecked"};
                static constexpr const char* SortOrder{"sortOrder"};
                static constexpr const char* UnreadCount{"unreadCount"};
                static constexpr const char* Statistics{"statistics"};
            }; // namespace Feed

            namespace Folder
            {
                static constexpr const char* ID{"id"};
                static constexpr const char* Title{"title"};
                static constexpr const char* Parent{"parent"};
                static constexpr const char* SortOrder{"sortOrder"};
                static constexpr const char* Subfolders{"subfolders"};
                static constexpr const char* Statistics{"statistics"};
                static constexpr const char* FeedIDs{"feedIDs"};
            }; // namespace Folder

            namespace Post
            {
                static constexpr const char* ID{"id"};
                static constexpr const char* IsRead{"isRead"};
                static constexpr const char* FeedID{"feedID"};
                static constexpr const char* FeedTitle{"feedTitle"};
                static constexpr const char* FeedLink{"feedLink"};
                static constexpr const char* Title{"title"};
                static constexpr const char* Link{"link"};
                static constexpr const char* Content{"content"};
                static constexpr const char* Author{"author"};
                static constexpr const char* CommentsURL{"commentsURL"};
                static constexpr const char* Guid{"guid"};
                static constexpr const char* DatePublished{"datePublished"};
                static constexpr const char* Thumbnail{"thumbnail"};
                static constexpr const char* FlagColors{"flagColors"};
                static constexpr const char* Enclosures{"enclosures"};
                static constexpr const char* EnclosureURL{"url"};
                static constexpr const char* EnclosureMimeType{"mimeType"};
                static constexpr const char* EnclosureSize{"size"};
            }; // namespace Post

            namespace ScriptFolder
            {
                static constexpr const char* ID{"id"};
                static constexpr const char* Title{"title"};
                static constexpr const char* ShowTotal{"showTotal"};
                static constexpr const char* ShowUnread{"showUnread"};
                static constexpr const char* TotalPostCount{"totalPostCount"};
                static constexpr const char* TotalUnreadCount{"totalUnreadCount"};
            }; // namespace ScriptFolder

            namespace Log
            {
                static constexpr const char* ID{"id"};
                static constexpr const char* Timestamp{"timestamp"};
                static constexpr const char* Level{"level"};
                static constexpr const char* Message{"message"};
                static constexpr const char* FeedID{"feedID"};
                static constexpr const char* FeedTitle{"feedTitle"};
            }; // namespace Log

            namespace Script
            {
                static constexpr const char* ID{"id"};
                static constexpr const char* Title{"title"};
                static constexpr const char* Type{"type"};
                static constexpr const char* IsEnabled{"isEnabled"};
                static constexpr const char* RunOnEvents{"runOnEvents"};
                static constexpr const char* RunOnFeedIDs{"runOnFeedIDs"};
                static constexpr const char* Script{"script"};
            }; // namespace Script

            namespace Statistic
            {
                static constexpr const char* FeedCount{"feedCount"};
                static constexpr const char* PostCount{"postCount"};
                static constexpr const char* FlaggedPostCount{"flaggedPostCount"};
                static constexpr const char* OldestPost{"oldestPost"};
                static constexpr const char* NewestPost{"newestPost"};
            }; // namespace Statistic

        }; // namespace JSON

    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_GLOBAL_H