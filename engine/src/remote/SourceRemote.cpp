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

#define FMT_HEADER_ONLY
#include <fmt/core.h>

#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPRequest.h>

#include "ZapFR/Helpers.h"
#include "ZapFR/Log.h"
#include "ZapFR/base/Feed.h"
#include "ZapFR/base/Folder.h"
#include "ZapFR/base/Post.h"
#include "ZapFR/base/ScriptFolder.h"
#include "ZapFR/remote/FeedRemote.h"
#include "ZapFR/remote/FolderRemote.h"
#include "ZapFR/remote/PostRemote.h"
#include "ZapFR/remote/ScriptFolderRemote.h"
#include "ZapFR/remote/ScriptRemote.h"
#include "ZapFR/remote/SourceRemote.h"

ZapFR::Engine::SourceRemote::SourceRemote(uint64_t id) : Source(id)
{
}

Poco::URI ZapFR::Engine::SourceRemote::remoteURL() const
{
    if (mRemoteURL.empty())
    {
        try
        {
            Poco::JSON::Parser parser;
            auto root = parser.parse(mConfigData);
            auto obj = root.extract<Poco::JSON::Object::Ptr>();
            auto host = obj->getValue<std::string>(JSON::RemoteConfigData::Host);
            auto port = obj->getValue<uint16_t>(JSON::RemoteConfigData::Port);
            auto useHTTPS = obj->getValue<bool>(JSON::RemoteConfigData::UseHTTPS);
            mRemoteLogin = obj->getValue<std::string>(JSON::RemoteConfigData::Login);
            mRemotePassword = obj->getValue<std::string>(JSON::RemoteConfigData::Password);

            mRemoteURL.setScheme(useHTTPS ? "https" : "http");
            mRemoteURL.setHost(host);
            mRemoteURL.setPort(port);
            mRemoteURLIsValid = true;
        }
        catch (...)
        {
            Log::log(LogLevel::Error, "Invalid config data for source");
        }
    }
    return mRemoteURL;
}

/* ************************** FEED STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceRemote::getFeeds(uint32_t fetchInfo)
{
    std::vector<std::unique_ptr<ZapFR::Engine::Feed>> feeds;

    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/feeds");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        if ((fetchInfo & FetchInfo::Icon) == FetchInfo::Icon)
        {
            params[HTTPParam::Feed::GetIcons] = HTTPParam::True;
        }

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto feedArr = root.extract<Poco::JSON::Array::Ptr>();
        if (!feedArr.isNull())
        {
            for (size_t i = 0; i < feedArr->size(); ++i)
            {
                auto feedObj = feedArr->getObject(static_cast<uint32_t>(i));
                feeds.emplace_back(FeedRemote::fromJSON(this, feedObj));
            }
        }
    }
    return feeds;
}

std::optional<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceRemote::getFeed(uint64_t feedID, uint32_t fetchInfo)
{
    if ((fetchInfo & FetchInfo::Data) == FetchInfo::Data || (fetchInfo & FetchInfo::Statistics) == FetchInfo::Statistics ||
        (fetchInfo & FetchInfo::FeedUnreadCount) == FetchInfo::FeedUnreadCount)
    {
        auto uri = remoteURL();
        if (mRemoteURLIsValid)
        {
            uri.setPath(fmt::format("/feed/{}", feedID));
            auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

            std::map<std::string, std::string> params;
            if ((fetchInfo & FetchInfo::Data) == FetchInfo::Data)
            {
                params[HTTPParam::Feed::GetData] = HTTPParam::True;
            }
            if ((fetchInfo & FetchInfo::Statistics) == FetchInfo::Statistics)
            {
                params[HTTPParam::Feed::GetStatistics] = HTTPParam::True;
            }
            if ((fetchInfo & FetchInfo::FeedUnreadCount) == FetchInfo::FeedUnreadCount)
            {
                params[HTTPParam::Feed::GetUnreadCount] = HTTPParam::True;
            }

            const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, params);
            auto parser = Poco::JSON::Parser();
            auto root = parser.parse(json);
            auto feedObj = root.extract<Poco::JSON::Object::Ptr>();
            if (!feedObj.isNull())
            {
                return FeedRemote::fromJSON(this, feedObj);
            }
        }
    }
    else
    {
        return std::make_unique<FeedRemote>(feedID, this);
    }
    return {};
}

std::optional<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceRemote::addFeed(const std::string& url, uint64_t folder)
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/feed");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        params[HTTPParam::Feed::URL] = url;
        params[HTTPParam::Feed::ParentFolderID] = std::to_string(folder);

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto o = root.extract<Poco::JSON::Object::Ptr>();
        if (!o.isNull())
        {
            return FeedRemote::fromJSON(this, o);
        }
    }
    return {};
}

std::unordered_map<uint64_t, uint64_t> ZapFR::Engine::SourceRemote::moveFeed(uint64_t feedID, uint64_t newFolder, uint64_t newSortOrder)
{
    std::unordered_map<uint64_t, uint64_t> affectedFeedIDs;

    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath(fmt::format("/feed/{}/move", feedID));
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        params[HTTPParam::Feed::ParentFolderID] = std::to_string(newFolder);
        params[HTTPParam::Feed::SortOrder] = std::to_string(newSortOrder);

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto arr = root.extract<Poco::JSON::Array::Ptr>();
        if (!arr.isNull())
        {
            for (size_t i = 0; i < arr->size(); ++i)
            {
                auto o = arr->getObject(static_cast<uint32_t>(i));
                if (!o.isNull())
                {
                    auto id = o->getValue<uint64_t>(JSON::Feed::ID);
                    auto sortOrder = o->getValue<uint64_t>(JSON::Feed::SortOrder);
                    affectedFeedIDs[id] = sortOrder;
                }
            }
        }
    }

    return affectedFeedIDs;
}

void ZapFR::Engine::SourceRemote::removeFeed(uint64_t feedID)
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath(fmt::format("/feed/{}", feedID));
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);
        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_DELETE, creds, {});
    }
}

/* ************************** FOLDER STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Folder>> ZapFR::Engine::SourceRemote::getFolders(uint64_t parent, uint32_t fetchInfo)
{
    std::vector<std::unique_ptr<ZapFR::Engine::Folder>> folders;

    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/folders");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        params[HTTPParam::Folder::ParentFolderID] = std::to_string(parent);
        if ((fetchInfo & FetchInfo::Subfolders) == FetchInfo::Subfolders)
        {
            params[HTTPParam::Folder::GetSubfolders] = HTTPParam::True;
        }

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto folderArr = root.extract<Poco::JSON::Array::Ptr>();
        if (!folderArr.isNull())
        {
            for (size_t i = 0; i < folderArr->size(); ++i)
            {
                auto folderObj = folderArr->getObject(static_cast<uint32_t>(i));
                folders.emplace_back(FolderRemote::fromJSON(this, folderObj));
            }
        }
    }
    return folders;
}

std::optional<std::unique_ptr<ZapFR::Engine::Folder>> ZapFR::Engine::SourceRemote::getFolder(uint64_t folderID, uint32_t fetchInfo)
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath(fmt::format("/folder/{}", folderID));
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        if ((fetchInfo & Source::FetchInfo::FolderFeedIDs) == Source::FetchInfo::FolderFeedIDs)
        {
            params[HTTPParam::Folder::GetFeedIDs] = HTTPParam::True;
        }
        if ((fetchInfo & Source::FetchInfo::Statistics) == Source::FetchInfo::Statistics)
        {
            params[HTTPParam::Folder::GetStatistics] = HTTPParam::True;
        }

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto folderObj = root.extract<Poco::JSON::Object::Ptr>();
        if (!folderObj.isNull())
        {
            return FolderRemote::fromJSON(this, folderObj);
        }
    }
    return {};
}

std::tuple<uint64_t, uint64_t> ZapFR::Engine::SourceRemote::addFolder(const std::string& title, uint64_t parentID)
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/folder");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        params[HTTPParam::Folder::Title] = title;
        params[HTTPParam::Folder::ParentFolderID] = std::to_string(parentID);

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto o = root.extract<Poco::JSON::Object::Ptr>();
        if (!o.isNull())
        {
            return std::make_tuple(o->getValue<uint64_t>(JSON::Folder::ID), o->getValue<uint64_t>(JSON::Folder::SortOrder));
        }
    }
    return std::make_tuple(0, 0);
}

std::unordered_map<uint64_t, uint64_t> ZapFR::Engine::SourceRemote::moveFolder(uint64_t folderID, uint64_t newParent, uint64_t newSortOrder)
{
    std::unordered_map<uint64_t, uint64_t> affectedFolderIDs;

    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath(fmt::format("/folder/{}/move", folderID));
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        params[HTTPParam::Folder::ParentFolderID] = std::to_string(newParent);
        params[HTTPParam::Folder::SortOrder] = std::to_string(newSortOrder);

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto arr = root.extract<Poco::JSON::Array::Ptr>();
        if (!arr.isNull())
        {
            for (size_t i = 0; i < arr->size(); ++i)
            {
                auto o = arr->getObject(static_cast<uint32_t>(i));
                if (!o.isNull())
                {
                    auto id = o->getValue<uint64_t>(JSON::Folder::ID);
                    auto sortOrder = o->getValue<uint64_t>(JSON::Folder::SortOrder);
                    affectedFolderIDs[id] = sortOrder;
                }
            }
        }
    }

    return affectedFolderIDs;
}

void ZapFR::Engine::SourceRemote::removeFolder(uint64_t folderID)
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath(fmt::format("/folder/{}", folderID));
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);
        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_DELETE, creds, {});
    }
}

/* ************************** POST STUFF ************************** */
std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Post>>> ZapFR::Engine::SourceRemote::getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                                                                                              const std::string& searchFilter, FlagColor flagColor)
{
    std::vector<std::unique_ptr<ZapFR::Engine::Post>> posts;
    uint64_t postCount{0};

    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/posts");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        params[HTTPParam::Post::ParentType] = HTTPParam::Post::ParentTypeSource;
        params[HTTPParam::Post::PerPage] = std::to_string(perPage);
        params[HTTPParam::Post::Page] = std::to_string(page);
        params[HTTPParam::Post::ShowOnlyUnread] = showOnlyUnread ? HTTPParam::True : HTTPParam::False;
        params[HTTPParam::Post::SearchFilter] = searchFilter;
        params[HTTPParam::Post::FlagColor] = Flag::nameForFlagColor(flagColor);

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
        if (!rootObj.isNull())
        {
            postCount = rootObj->getValue<uint64_t>(JSON::Post::Count);
            auto postArr = rootObj->getArray(JSON::Post::Posts);
            if (!postArr.isNull())
            {
                for (size_t i = 0; i < postArr->size(); ++i)
                {
                    auto postObj = postArr->getObject(static_cast<uint32_t>(i));
                    posts.emplace_back(PostRemote::createFromJSON(postObj));
                }
            }
            unserializeThumbnailData(mThumbnailData, rootObj->getArray(JSON::Post::ThumbnailData));
        }
    }
    return std::make_tuple(postCount, std::move(posts));
}

void ZapFR::Engine::SourceRemote::unserializeThumbnailData(std::vector<ThumbnailData>& destination, Poco::JSON::Array::Ptr source)
{
    destination.clear();
    for (size_t i = 0; i < source->size(); ++i)
    {
        auto tdObj = source->getObject(static_cast<uint32_t>(i));
        ThumbnailData td;
        td.feedID = tdObj->getValue<uint64_t>(JSON::ThumbnailData::FeedID);
        td.feedTitle = tdObj->getValue<std::string>(JSON::ThumbnailData::FeedTitle);

        auto tdPostsArr = tdObj->getArray(JSON::ThumbnailData::Posts);
        for (size_t j = 0; j < tdPostsArr->size(); ++j)
        {
            auto tdpObj = tdPostsArr->getObject(static_cast<uint32_t>(j));
            ThumbnailDataPost tdp;
            tdp.link = tdpObj->getValue<std::string>(JSON::ThumbnailData::PostLink);
            tdp.postID = tdpObj->getValue<uint64_t>(JSON::ThumbnailData::PostID);
            tdp.thumbnail = tdpObj->getValue<std::string>(JSON::ThumbnailData::PostThumbnail);
            tdp.timestamp = tdpObj->getValue<time_t>(JSON::ThumbnailData::PostTimestamp);
            tdp.title = tdpObj->getValue<std::string>(JSON::ThumbnailData::PostTitle);
            td.posts.emplace_back(tdp);
        }

        destination.emplace_back(td);
    }
}

void ZapFR::Engine::SourceRemote::markAsRead(uint64_t maxPostID)
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/mark-as-read");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        params[HTTPParam::Post::MaxPostID] = std::to_string(maxPostID);

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
    }
}

void ZapFR::Engine::SourceRemote::setPostsReadStatus(bool markAsRead, const std::vector<std::tuple<uint64_t, uint64_t>>& feedsAndPostIDs)
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/set-posts-read-status");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        Poco::JSON::Array arr;
        for (const auto& [feedID, postID] : feedsAndPostIDs)
        {
            Poco::JSON::Object o;
            o.set(HTTPParam::Source::FeedID, feedID);
            o.set(HTTPParam::Source::PostID, postID);
            arr.add(o);
        }
        std::stringstream ss;
        Poco::JSON::Stringifier::stringify(arr, ss);

        std::map<std::string, std::string> params;
        params[HTTPParam::Source::FeedsAndPostIDs] = ss.str();
        params[HTTPParam::Source::MarkAsRead] = markAsRead ? HTTPParam::True : HTTPParam::False;

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
    }
}

void ZapFR::Engine::SourceRemote::setPostsFlagStatus(bool markFlagged, const std::unordered_set<FlagColor>& flagColors,
                                                     const std::vector<std::tuple<uint64_t, uint64_t>>& feedsAndPostIDs)
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/set-posts-flag-status");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        Poco::JSON::Array idArr;
        for (const auto& [feedID, postID] : feedsAndPostIDs)
        {
            Poco::JSON::Object o;
            o.set(HTTPParam::Source::FeedID, feedID);
            o.set(HTTPParam::Source::PostID, postID);
            idArr.add(o);
        }
        std::stringstream idSS;
        Poco::JSON::Stringifier::stringify(idArr, idSS);

        Poco::JSON::Array colorArr;
        for (const auto& fc : flagColors)
        {
            colorArr.add(Flag::nameForFlagColor(fc));
        }
        std::stringstream colorSS;
        Poco::JSON::Stringifier::stringify(colorArr, colorSS);

        std::map<std::string, std::string> params;
        params[HTTPParam::Source::FeedsAndPostIDs] = idSS.str();
        params[HTTPParam::Source::FlagColors] = colorSS.str();
        params[HTTPParam::Source::MarkFlagged] = markFlagged ? HTTPParam::True : HTTPParam::False;

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
    }
}

void ZapFR::Engine::SourceRemote::assignPostsToScriptFolder(uint64_t scriptFolderID, bool assign, const std::vector<std::tuple<uint64_t, uint64_t>>& feedsAndPostIDs)
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath(fmt::format("/scriptfolder/{}/assign-posts", scriptFolderID));
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        Poco::JSON::Array idArr;
        for (const auto& [feedID, postID] : feedsAndPostIDs)
        {
            Poco::JSON::Object o;
            o.set(HTTPParam::ScriptFolder::FeedID, feedID);
            o.set(HTTPParam::ScriptFolder::PostID, postID);
            idArr.add(o);
        }
        std::stringstream idSS;
        Poco::JSON::Stringifier::stringify(idArr, idSS);

        std::map<std::string, std::string> params;
        params[HTTPParam::ScriptFolder::FeedsAndPostIDs] = idSS.str();
        params[HTTPParam::ScriptFolder::Assign] = assign ? HTTPParam::True : HTTPParam::False;

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
    }
}

Poco::JSON::Object ZapFR::Engine::SourceRemote::getStatus()
{
    Poco::JSON::Object statusObj;

    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/status");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {});
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
        if (!rootObj.isNull())
        {
            statusObj = *rootObj;
        }
    }

    return statusObj;
}

/* ************************** LOGS STUFF ************************** */
std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Log>>> ZapFR::Engine::SourceRemote::getLogs(uint64_t perPage, uint64_t page)
{
    std::vector<std::unique_ptr<ZapFR::Engine::Log>> logs;
    uint64_t logCount{0};

    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/logs");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        params[HTTPParam::Log::ParentType] = HTTPParam::Log::ParentTypeSource;
        params[HTTPParam::Log::PerPage] = std::to_string(perPage);
        params[HTTPParam::Log::Page] = std::to_string(page);

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
        if (!rootObj.isNull())
        {
            logCount = rootObj->getValue<uint64_t>(JSON::Log::Count);
            auto logArr = rootObj->getArray(JSON::Log::Logs);
            if (!logArr.isNull())
            {
                for (size_t i = 0; i < logArr->size(); ++i)
                {
                    auto logObj = logArr->getObject(static_cast<uint32_t>(i));
                    logs.emplace_back(Log::fromJSON(logObj));
                }
            }
        }
    }
    return std::make_tuple(logCount, std::move(logs));
}

/* ************************** FLAG STUFF ************************** */
std::unordered_set<ZapFR::Engine::FlagColor> ZapFR::Engine::SourceRemote::getUsedFlagColors()
{
    std::unordered_set<ZapFR::Engine::FlagColor> flagColors;

    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/used-flag-colors");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {});
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto rootArr = root.extract<Poco::JSON::Array::Ptr>();
        if (!rootArr.isNull())
        {
            for (size_t i = 0; i < rootArr->size(); ++i)
            {
                try
                {
                    flagColors.insert(Flag::flagColorForName(rootArr->getElement<std::string>(static_cast<uint32_t>(i))));
                }
                catch (...)
                {
                    // skip unknown flag names
                }
            }
        }
    }
    return flagColors;
}

/* ************************** SCRIPT FOLDER STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::ScriptFolder>> ZapFR::Engine::SourceRemote::getScriptFolders()
{
    std::vector<std::unique_ptr<ZapFR::Engine::ScriptFolder>> scriptFolders;

    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/scriptfolders");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {});
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto scriptFolderArr = root.extract<Poco::JSON::Array::Ptr>();
        if (!scriptFolderArr.isNull())
        {
            for (size_t i = 0; i < scriptFolderArr->size(); ++i)
            {
                auto scriptFolderObj = scriptFolderArr->getObject(static_cast<uint32_t>(i));
                scriptFolders.emplace_back(ScriptFolderRemote::fromJSON(this, scriptFolderObj));
            }
        }
    }
    return scriptFolders;
}

std::optional<std::unique_ptr<ZapFR::Engine::ScriptFolder>> ZapFR::Engine::SourceRemote::getScriptFolder(uint64_t id, uint32_t fetchInfo)
{
    if ((fetchInfo & FetchInfo::Data) == FetchInfo::Data)
    {
        auto uri = remoteURL();
        if (mRemoteURLIsValid)
        {
            uri.setPath(fmt::format("/scriptfolder/{}", id));
            auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

            const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {});
            auto parser = Poco::JSON::Parser();
            auto root = parser.parse(json);
            auto scriptFolderObj = root.extract<Poco::JSON::Object::Ptr>();
            if (!scriptFolderObj.isNull())
            {
                return ScriptFolderRemote::fromJSON(this, scriptFolderObj);
            }
        }
    }
    else
    {
        return std::make_unique<ScriptFolderRemote>(id, this);
    }
    return {};
}

void ZapFR::Engine::SourceRemote::addScriptFolder(const std::string& title, bool showTotal, bool showUnread)
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/scriptfolder");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        params[HTTPParam::ScriptFolder::Title] = title;
        params[HTTPParam::ScriptFolder::ShowTotal] = showTotal ? HTTPParam::True : HTTPParam::False;
        params[HTTPParam::ScriptFolder::ShowUnread] = showUnread ? HTTPParam::True : HTTPParam::False;

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
    }
}

void ZapFR::Engine::SourceRemote::removeScriptFolder(uint64_t scriptFolderID)
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath(fmt::format("/scriptfolder/{}", scriptFolderID));
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_DELETE, creds, {});
    }
}

/* ************************** SCRIPT STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Script>> ZapFR::Engine::SourceRemote::getScripts()
{
    std::vector<std::unique_ptr<ZapFR::Engine::Script>> scripts;

    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/scripts");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {});
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto scriptArr = root.extract<Poco::JSON::Array::Ptr>();
        if (!scriptArr.isNull())
        {
            for (size_t i = 0; i < scriptArr->size(); ++i)
            {
                auto scriptObj = scriptArr->getObject(static_cast<uint32_t>(i));
                scripts.emplace_back(ScriptRemote::fromJSON(this, scriptObj));
            }
        }
    }
    return scripts;
}

std::optional<std::unique_ptr<ZapFR::Engine::Script>> ZapFR::Engine::SourceRemote::getScript(uint64_t scriptID, uint32_t fetchInfo)
{
    if ((fetchInfo & FetchInfo::Data) == FetchInfo::Data)
    {
        auto uri = remoteURL();
        if (mRemoteURLIsValid)
        {
            uri.setPath(fmt::format("/script/{}", scriptID));
            auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

            const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {});
            auto parser = Poco::JSON::Parser();
            auto root = parser.parse(json);
            auto scriptObj = root.extract<Poco::JSON::Object::Ptr>();
            if (!scriptObj.isNull())
            {
                return ScriptRemote::fromJSON(this, scriptObj);
            }
        }
    }
    else
    {
        return std::make_unique<ScriptRemote>(scriptID, this);
    }
    return {};
}

void ZapFR::Engine::SourceRemote::removeScript(uint64_t scriptID)
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath(fmt::format("/script/{}", scriptID));
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_DELETE, creds, {});
    }
}

void ZapFR::Engine::SourceRemote::addScript(Script::Type /*type*/, const std::string& title, bool enabled, const std::unordered_set<Script::Event>& events,
                                            const std::optional<std::unordered_set<uint64_t>>& feedIDs, const std::string& script)
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/script");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        params[HTTPParam::Script::Type] = Script::msTypeLuaIdentifier; // forced to lua
        params[HTTPParam::Script::Title] = title;
        params[HTTPParam::Script::IsEnabled] = enabled ? HTTPParam::True : HTTPParam::False;
        params[HTTPParam::Script::RunOnEvents] = Script::runOnEventsString(events);
        params[HTTPParam::Script::RunOnFeedIDs] = Script::runOnFeedIDsString(feedIDs);
        params[HTTPParam::Script::Script] = script;

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
    }
}

/* ************************** SOURCE STUFF ************************** */
std::unordered_set<uint64_t> ZapFR::Engine::SourceRemote::importOPML(const std::string& opml, uint64_t parentFolderID)
{
    std::unordered_set<uint64_t> feedIDs;

    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/import-opml");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        params[HTTPParam::Source::OPML] = opml;
        params[HTTPParam::Source::ParentFolderID] = std::to_string(parentFolderID);

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto rootArr = root.extract<Poco::JSON::Array::Ptr>();
        if (!rootArr.isNull())
        {
            for (size_t i = 0; i < rootArr->size(); ++i)
            {
                feedIDs.insert(rootArr->getElement<uint64_t>(static_cast<uint32_t>(i)));
            }
        }
    }

    return feedIDs;
}

void ZapFR::Engine::SourceRemote::fetchStatistics()
{
    mStatistics.clear();

    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/statistics");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        const auto& [json, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {});
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto statsObj = root.extract<Poco::JSON::Object::Ptr>();
        if (!statsObj.isNull())
        {
            for (const auto& [k, v] : Source::SourceStatisticJSONIdentifierMap)
            {
                if (statsObj->has(v))
                {
                    mStatistics[k] = statsObj->getValue<std::string>(v);
                }
            }
        }
    }
}

void ZapFR::Engine::SourceRemote::fetchThumbnailData()
{
    // nop, this data is received with the getPosts call
}

void ZapFR::Engine::SourceRemote::clearLogs()
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/logs");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);
        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_DELETE, creds, {});
    }
}
