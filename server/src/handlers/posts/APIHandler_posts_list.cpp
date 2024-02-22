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

#include "API.h"
#include "APIHandlers.h"
#include "APIRequest.h"
#include "ZapFR/Flag.h"
#include "ZapFR/Global.h"
#include "ZapFR/base/Source.h"
#include "ZapFR/local/FeedLocal.h"
#include "ZapFR/local/FolderLocal.h"
#include "ZapFR/local/PostLocal.h"
#include "ZapFR/local/ScriptFolderLocal.h"

// ::API
//
//	Returns all the posts belonging to a feed, folder, source or script folder with various filters applied
//	/posts (GET)
//
//	Parameters:
//		parentType (REQD) - The type (source, folder, feed, scriptfolder) to retrieve posts for - apiRequest->parameter("parentType")
//		parentID - The ID of the parent type (feedID, folderID or scriptFolderID); n/a in case of 'source' - apiRequest->parameter("parentID")
//		perPage (REQD) - The amount of records per page to retrieve - apiRequest->parameter("perPage")
//		page (REQD) - The page number to retrieve - apiRequest->parameter("page")
//		showOnlyUnread - Whether to only retrieve unread posts - 'true' or 'false' - optional (default: false) - apiRequest->parameter("showOnlyUnread")
//		showUnreadPostsAtTop - Whether to show the unread posts first - 'true' or 'false' - optional (default: false) - apiRequest->parameter("showUnreadPostsAtTop")
//		searchFilter - An optional search filter to apply - apiRequest->parameter("searchFilter")
//		categoryFilter - An optional category filter to apply (the ID of the cat to match) - apiRequest->parameter("categoryFilter")
//		flagColor - The ID of a flag color to apply as a filter - apiRequest->parameter("flagColor")
//
//	Content-Type: application/json
//	JSON output: Object
//
// API::

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_posts_list([[maybe_unused]] APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{
    const auto parentType = apiRequest->parameter(ZapFR::Engine::HTTPParam::Post::ParentType);
    const auto parentIDStr = apiRequest->parameter(ZapFR::Engine::HTTPParam::Post::ParentID);
    const auto perPageStr = apiRequest->parameter(ZapFR::Engine::HTTPParam::Post::PerPage);
    const auto pageStr = apiRequest->parameter(ZapFR::Engine::HTTPParam::Post::Page);
    const auto showOnlyUnread = (apiRequest->parameter(ZapFR::Engine::HTTPParam::Post::ShowOnlyUnread) == ZapFR::Engine::HTTPParam::True);
    const auto showUnreadPostsAtTop = (apiRequest->parameter(ZapFR::Engine::HTTPParam::Post::ShowUnreadPostsAtTop) == ZapFR::Engine::HTTPParam::True);
    const auto searchFilter = apiRequest->parameter(ZapFR::Engine::HTTPParam::Post::SearchFilter);
    const auto categoryFilterStr = apiRequest->parameter(ZapFR::Engine::HTTPParam::Post::CategoryFilter);
    const auto flagColorStr = apiRequest->parameter(ZapFR::Engine::HTTPParam::Post::FlagColor);

    ZapFR::Engine::FlagColor flagFilter{ZapFR::Engine::FlagColor::Gray};
    if (!flagColorStr.empty())
    {
        flagFilter = ZapFR::Engine::Flag::flagColorForName(flagColorStr);
    }

    uint64_t perPage{1000};
    uint64_t page{1};
    uint64_t parentID{0};
    Poco::NumberParser::tryParseUnsigned64(perPageStr, perPage);
    Poco::NumberParser::tryParseUnsigned64(pageStr, page);
    Poco::NumberParser::tryParseUnsigned64(parentIDStr, parentID);

    uint64_t categoryFilterID{0};
    Poco::NumberParser::tryParseUnsigned64(categoryFilterStr, categoryFilterID);

    Poco::JSON::Object o;
    auto source = ZapFR::Engine::Source::getSource(1);
    if (source.has_value())
    {
        uint64_t postCount{0};
        std::vector<std::unique_ptr<ZapFR::Engine::Post>> posts;
        std::vector<ZapFR::Engine::ThumbnailData> thumbnailData;

        if (parentType == ZapFR::Engine::HTTPParam::Post::ParentTypeFeed)
        {
            auto feed = source.value()->getFeed(parentID, ZapFR::Engine::Source::FetchInfo::UnreadThumbnailData);
            if (feed.has_value())
            {
                auto t = feed.value()->getPosts(perPage, page, showOnlyUnread, showUnreadPostsAtTop, searchFilter, categoryFilterID, flagFilter);
                postCount = std::get<uint64_t>(t);
                posts = std::move(std::get<std::vector<std::unique_ptr<ZapFR::Engine::Post>>>(t));
                thumbnailData = feed.value()->thumbnailData();
            }
        }
        else if (parentType == ZapFR::Engine::HTTPParam::Post::ParentTypeSource)
        {
            auto t = source.value()->getPosts(perPage, page, showOnlyUnread, showUnreadPostsAtTop, searchFilter, categoryFilterID, flagFilter);
            postCount = std::get<uint64_t>(t);
            posts = std::move(std::get<std::vector<std::unique_ptr<ZapFR::Engine::Post>>>(t));

            source.value()->fetchThumbnailData();
            thumbnailData = source.value()->thumbnailData();
        }
        else if (parentType == ZapFR::Engine::HTTPParam::Post::ParentTypeFolder)
        {
            auto folder = source.value()->getFolder(parentID, ZapFR::Engine::Source::FetchInfo::UnreadThumbnailData);
            if (folder.has_value())
            {
                auto t = folder.value()->getPosts(perPage, page, showOnlyUnread, showUnreadPostsAtTop, searchFilter, categoryFilterID, flagFilter);
                postCount = std::get<uint64_t>(t);
                posts = std::move(std::get<std::vector<std::unique_ptr<ZapFR::Engine::Post>>>(t));
                thumbnailData = folder.value()->thumbnailData();
            }
        }
        else if (parentType == ZapFR::Engine::HTTPParam::Post::ParentTypeScriptFolder)
        {
            auto scriptFolder = source.value()->getScriptFolder(parentID, ZapFR::Engine::Source::FetchInfo::UnreadThumbnailData);
            if (scriptFolder.has_value())
            {
                auto t = scriptFolder.value()->getPosts(perPage, page, showOnlyUnread, showUnreadPostsAtTop, searchFilter, categoryFilterID, flagFilter);
                postCount = std::get<uint64_t>(t);
                posts = std::move(std::get<std::vector<std::unique_ptr<ZapFR::Engine::Post>>>(t));
                thumbnailData = scriptFolder.value()->thumbnailData();
            }
        }
        else
        {
            throw std::runtime_error("Invalid parent type requested");
        }

        Poco::JSON::Array postsArr;
        for (const auto& post : posts)
        {
            postsArr.add(post->toJSON());
        }
        o.set(ZapFR::Engine::JSON::Post::Posts, postsArr);
        o.set(ZapFR::Engine::JSON::Post::Count, postCount);

        Poco::JSON::Array tdArr;
        for (const auto& td : thumbnailData)
        {
            Poco::JSON::Object tdObj;
            tdObj.set(ZapFR::Engine::JSON::ThumbnailData::FeedID, td.feedID);
            tdObj.set(ZapFR::Engine::JSON::ThumbnailData::FeedTitle, td.feedTitle);
            tdObj.set(ZapFR::Engine::JSON::ThumbnailData::FeedLink, td.feedLink);

            Poco::JSON::Array tdpArr;
            for (const auto& post : td.posts)
            {
                Poco::JSON::Object pObj;
                pObj.set(ZapFR::Engine::JSON::ThumbnailData::PostLink, post.link);
                pObj.set(ZapFR::Engine::JSON::ThumbnailData::PostID, post.postID);
                pObj.set(ZapFR::Engine::JSON::ThumbnailData::PostThumbnail, post.thumbnail);
                pObj.set(ZapFR::Engine::JSON::ThumbnailData::PostTitle, post.title);
                tdpArr.add(pObj);
            }
            tdObj.set(ZapFR::Engine::JSON::ThumbnailData::Posts, tdpArr);
            tdObj.set(ZapFR::Engine::JSON::ThumbnailData::TotalPostCount, td.totalPostCount);
            tdArr.add(tdObj);
        }
        o.set(ZapFR::Engine::JSON::Post::ThumbnailData, tdArr);
    }

    Poco::JSON::Stringifier::stringify(o, response.send());

    return Poco::Net::HTTPResponse::HTTP_OK;
}