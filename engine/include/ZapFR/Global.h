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
            std::string feedLink{""};
            std::vector<ThumbnailDataPost> posts{};
        };

        constexpr uint64_t DBVersion{7};
        constexpr uint64_t APIVersion{1};
        constexpr uint64_t DefaultFeedAutoRefreshInterval{15 * 60};
        constexpr uint16_t DefaultServerPort{16016};

        namespace ServerIdentifier
        {
            constexpr const char Local[]{"zapfeedreader.local"};
            constexpr const char Remote[]{"zapfeedreader.remote"};
        }; // namespace ServerIdentifier

        namespace JSON
        {
            namespace RemoteConfigData
            {
                constexpr const char Host[]{"host"};
                constexpr const char Port[]{"port"};
                constexpr const char Login[]{"login"};
                constexpr const char Password[]{"password"};
                constexpr const char UseHTTPS[]{"useHTTPS"};
            }; // namespace RemoteConfigData

            namespace ThumbnailData
            {
                constexpr const char FeedID[]{"feedID"};
                constexpr const char FeedTitle[]{"feedTitle"};
                constexpr const char FeedLink[]{"feedLink"};
                constexpr const char PostLink[]{"link"};
                constexpr const char PostID[]{"postID"};
                constexpr const char PostThumbnail[]{"thumbnail"};
                constexpr const char PostTimestamp[]{"timestamp"};
                constexpr const char PostTitle[]{"title"};
                constexpr const char Posts[]{"posts"};
            }; // namespace ThumbnailData

            namespace Feed
            {
                constexpr const char ID[]{"id"};
                constexpr const char URL[]{"url"};
                constexpr const char Folder[]{"folder"};
                constexpr const char GUID[]{"guid"};
                constexpr const char Icon[]{"icon"};
                constexpr const char IconHash[]{"iconHash"};
                constexpr const char Title[]{"title"};
                constexpr const char Subtitle[]{"subtitle"};
                constexpr const char Link[]{"link"};
                constexpr const char Description[]{"description"};
                constexpr const char Language[]{"language"};
                constexpr const char Copyright[]{"copyright"};
                constexpr const char LastRefreshError[]{"lastRefreshError"};
                constexpr const char RefreshInterval[]{"refreshInterval"};
                constexpr const char LastChecked[]{"lastChecked"};
                constexpr const char SortOrder[]{"sortOrder"};
                constexpr const char UnreadCount[]{"unreadCount"};
                constexpr const char Statistics[]{"statistics"};
            }; // namespace Feed

            namespace Folder
            {
                constexpr const char ID[]{"id"};
                constexpr const char Title[]{"title"};
                constexpr const char Parent[]{"parent"};
                constexpr const char SortOrder[]{"sortOrder"};
                constexpr const char Subfolders[]{"subfolders"};
                constexpr const char Statistics[]{"statistics"};
                constexpr const char FeedIDs[]{"feedIDs"};
                constexpr const char FolderSortOrders[]{"folderSortOrders"};
                constexpr const char FeedSortOrders[]{"feedSortOrders"};
            }; // namespace Folder

            namespace Post
            {
                constexpr const char ID[]{"id"};
                constexpr const char IsRead[]{"isRead"};
                constexpr const char FeedID[]{"feedID"};
                constexpr const char FeedTitle[]{"feedTitle"};
                constexpr const char FeedLink[]{"feedLink"};
                constexpr const char Title[]{"title"};
                constexpr const char Link[]{"link"};
                constexpr const char Content[]{"content"};
                constexpr const char Author[]{"author"};
                constexpr const char CommentsURL[]{"commentsURL"};
                constexpr const char Guid[]{"guid"};
                constexpr const char DatePublished[]{"datePublished"};
                constexpr const char Thumbnail[]{"thumbnail"};
                constexpr const char FlagColors[]{"flagColors"};
                constexpr const char Enclosures[]{"enclosures"};
                constexpr const char EnclosureURL[]{"url"};
                constexpr const char EnclosureMimeType[]{"mimeType"};
                constexpr const char EnclosureSize[]{"size"};
                constexpr const char Posts[]{"posts"};
                constexpr const char Count[]{"count"};
                constexpr const char ThumbnailData[]{"thumbnailData"};
                constexpr const char Categories[]{"categories"};
                constexpr const char CategoryID[]{"id"};
                constexpr const char CategoryTitle[]{"title"};
            }; // namespace Post

            namespace ScriptFolder
            {
                constexpr const char ID[]{"id"};
                constexpr const char Title[]{"title"};
                constexpr const char ShowTotal[]{"showTotal"};
                constexpr const char ShowUnread[]{"showUnread"};
                constexpr const char TotalPostCount[]{"totalPostCount"};
                constexpr const char TotalUnreadCount[]{"totalUnreadCount"};
            }; // namespace ScriptFolder

            namespace Log
            {
                constexpr const char ID[]{"id"};
                constexpr const char Timestamp[]{"timestamp"};
                constexpr const char Level[]{"level"};
                constexpr const char Message[]{"message"};
                constexpr const char FeedID[]{"feedID"};
                constexpr const char FeedTitle[]{"feedTitle"};
                constexpr const char Logs[]{"logs"};
                constexpr const char Count[]{"count"};
            }; // namespace Log

            namespace Script
            {
                constexpr const char ID[]{"id"};
                constexpr const char Title[]{"title"};
                constexpr const char Type[]{"type"};
                constexpr const char IsEnabled[]{"isEnabled"};
                constexpr const char RunOnEvents[]{"runOnEvents"};
                constexpr const char RunOnFeedIDs[]{"runOnFeedIDs"};
                constexpr const char Script[]{"script"};
            }; // namespace Script

            namespace Category
            {
                constexpr const char ID[]{"id"};
                constexpr const char Title[]{"title"};
            }; // namespace Category

            namespace Statistic
            {
                constexpr const char FeedCount[]{"feedCount"};
                constexpr const char PostCount[]{"postCount"};
                constexpr const char FlaggedPostCount[]{"flaggedPostCount"};
                constexpr const char OldestPost[]{"oldestPost"};
                constexpr const char NewestPost[]{"newestPost"};
            }; // namespace Statistic

            namespace SourceStatus
            {
                constexpr const char FeedID[]{"feedID"};
                constexpr const char UnreadCount[]{"unreadCount"};
                constexpr const char UnreadCounts[]{"unreadCounts"};
                constexpr const char FeedError[]{"error"};
                constexpr const char FeedErrors[]{"feedErrors"};
                constexpr const char HighestPostID[]{"highestPostID"};
            }; // namespace SourceStatus

            namespace About
            {
                constexpr const char Name[]{"name"};
                constexpr const char Version[]{"version"};
            }; // namespace About

        }; // namespace JSON

        namespace HTTPParam
        {
            constexpr const char True[]{"true"};
            constexpr const char False[]{"false"};

            namespace Feed
            {
                constexpr const char URL[]{"url"};
                constexpr const char GetData[]{"getData"};
                constexpr const char GetStatistics[]{"getStatistics"};
                constexpr const char GetUnreadCount[]{"getUnreadCount"};
                constexpr const char SortOrder[]{"sortOrder"};
                constexpr const char ParentFolderID[]{"parentFolderID"};
                constexpr const char RefreshInterval[]{"refreshInterval"};
                constexpr const char GetIcons[]{"getIcons"};

            }; // namespace Feed

            namespace Folder
            {
                constexpr const char Title[]{"title"};
                constexpr const char ParentFolderID[]{"parentFolderID"};
                constexpr const char FolderID[]{"folderID"};
                constexpr const char GetStatistics[]{"getStatistics"};
                constexpr const char GetFeedIDs[]{"getFeedIDs"};
                constexpr const char SortOrder[]{"sortOrder"};
                constexpr const char SortMethod[]{"sortMethod"};
                constexpr const char SortMethodAlphabeticallyAscending[]{"alphaAsc"};
                constexpr const char NewTitle[]{"newTitle"};
                constexpr const char GetSubfolders[]{"getSubfolders"};
            }; // namespace Folder

            namespace Log
            {
                constexpr const char ParentType[]{"parentType"};
                constexpr const char ParentTypeSource[]{"source"};
                constexpr const char ParentTypeFeed[]{"feed"};
                constexpr const char ParentTypeFolder[]{"folder"};
                constexpr const char ParentID[]{"parentID"};
                constexpr const char PerPage[]{"perPage"};
                constexpr const char Page[]{"page"};
            }; // namespace Log

            namespace Post
            {
                constexpr const char FeedID[]{"feedID"};
                constexpr const char MaxPostID[]{"maxPostID"};
                constexpr const char ParentType[]{"parentType"};
                constexpr const char ParentTypeSource[]{"source"};
                constexpr const char ParentTypeFeed[]{"feed"};
                constexpr const char ParentTypeFolder[]{"folder"};
                constexpr const char ParentTypeScriptFolder[]{"scriptfolder"};
                constexpr const char ParentID[]{"parentID"};
                constexpr const char PerPage[]{"perPage"};
                constexpr const char Page[]{"page"};
                constexpr const char ShowOnlyUnread[]{"showOnlyUnread"};
                constexpr const char ShowUnreadPostsAtTop[]{"showUnreadPostsAtTop"};
                constexpr const char SearchFilter[]{"searchFilter"};
                constexpr const char CategoryFilter[]{"categoryFilter"};
                constexpr const char FlagColor[]{"flagColor"};
            }; // namespace Post

            namespace ScriptFolder
            {
                constexpr const char Title[]{"title"};
                constexpr const char ShowTotal[]{"showTotal"};
                constexpr const char ShowUnread[]{"showUnread"};
                constexpr const char FeedsAndPostIDs[]{"feedsAndPostIDs"};
                constexpr const char FeedID[]{"feedID"};
                constexpr const char PostID[]{"postID"};
                constexpr const char Assign[]{"assign"};

            }; // namespace ScriptFolder

            namespace Script
            {
                constexpr const char Type[]{"type"};
                constexpr const char Title[]{"title"};
                constexpr const char IsEnabled[]{"isEnabled"};
                constexpr const char RunOnEvents[]{"runOnEvents"};
                constexpr const char RunOnFeedIDs[]{"runOnFeedIDs"};
                constexpr const char Script[]{"script"};
            }; // namespace Script

            namespace Source
            {
                constexpr const char OPML[]{"opml"};
                constexpr const char ParentFolderID[]{"parentFolderID"};
                constexpr const char MarkFlagged[]{"markFlagged"};
                constexpr const char FlagColors[]{"flagColors"};
                constexpr const char FeedsAndPostIDs[]{"feedsAndPostIDs"};
                constexpr const char FeedID[]{"feedID"};
                constexpr const char PostID[]{"postID"};
                constexpr const char MarkAsRead[]{"markAsRead"};
            }; // namespace Source

            namespace Category
            {
                constexpr const char ParentType[]{"parentType"};
                constexpr const char ParentTypeSource[]{"source"};
                constexpr const char ParentTypeFeed[]{"feed"};
                constexpr const char ParentTypeFolder[]{"folder"};
                constexpr const char ParentTypeScriptFolder[]{"scriptfolder"};
                constexpr const char ParentID[]{"parentID"};
            } // namespace Category

        }; // namespace HTTPParam

    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_GLOBAL_H