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

#include "ZapFR/remote/SourceRemote.h"
#include "ZapFR/Feed.h"
#include "ZapFR/Folder.h"
#include "ZapFR/Helpers.h"
#include "ZapFR/Log.h"
#include "ZapFR/Post.h"
#include "ZapFR/ScriptFolder.h"
#include "ZapFR/remote/FeedRemote.h"
#include "ZapFR/remote/FolderRemote.h"
#include "ZapFR/remote/PostRemote.h"

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
            auto host = obj->getValue<std::string>("host");
            auto port = obj->getValue<uint16_t>("port");
            auto useHTTPS = obj->getValue<bool>("useHTTPS");
            mRemoteLogin = obj->getValue<std::string>("login");
            mRemotePassword = obj->getValue<std::string>("password");

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
std::vector<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceRemote::getFeeds()
{
    std::vector<std::unique_ptr<ZapFR::Engine::Feed>> feeds;

    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/feeds");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        try
        {
            auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {});
            auto parser = Poco::JSON::Parser();
            auto root = parser.parse(json);
            auto feedArr = root.extract<Poco::JSON::Array::Ptr>();
            if (!feedArr.isNull())
            {
                for (size_t i = 0; i < feedArr->size(); ++i)
                {
                    auto feedObj = feedArr->getObject(static_cast<uint32_t>(i));
                    feeds.emplace_back(std::move(FeedRemote::fromJSON(this, feedObj)));
                }
            }
        }
        catch (...)
        {
            return feeds;
        }
    }
    return feeds;
}

std::optional<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceRemote::getFeed(uint64_t feedID, uint32_t fetchInfo)
{
    // FetchInfo::Statistics is implied
    if ((fetchInfo & FetchInfo::Data) == FetchInfo::Data)
    {
        auto uri = remoteURL();
        if (mRemoteURLIsValid)
        {
            uri.setPath(fmt::format("/feed/{}", feedID));
            auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

            try
            {
                auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {});
                auto parser = Poco::JSON::Parser();
                auto root = parser.parse(json);
                auto feedObj = root.extract<Poco::JSON::Object::Ptr>();
                if (!feedObj.isNull())
                {
                    return FeedRemote::fromJSON(this, feedObj);
                }
            }
            catch (...)
            {
                return {};
            }
        }
    }
    else
    {
        return std::make_unique<FeedRemote>(feedID, this);
    }
    return {};
}

uint64_t ZapFR::Engine::SourceRemote::addFeed(const std::string& url, uint64_t folder)
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/feed");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        params["url"] = url;
        params["folder"] = std::to_string(folder);

        auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto o = root.extract<Poco::JSON::Object::Ptr>();
        if (!o.isNull())
        {
            return o->getValue<uint64_t>(Feed::JSONIdentifierFeedID);
        }
    }
    return 0;
}

void ZapFR::Engine::SourceRemote::moveFeed(uint64_t feedID, uint64_t newFolder, uint64_t newSortOrder)
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath(fmt::format("/feed/{}/move", feedID));
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        params["parentFolderID"] = std::to_string(newFolder);
        params["sortOrder"] = std::to_string(newSortOrder);

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
    }
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
std::vector<std::unique_ptr<ZapFR::Engine::Folder>> ZapFR::Engine::SourceRemote::getFolders(uint64_t parent)
{
    std::vector<std::unique_ptr<ZapFR::Engine::Folder>> folders;

    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/folders");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        params["parentFolderID"] = std::to_string(parent);

        try
        {
            auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, params);
            auto parser = Poco::JSON::Parser();
            auto root = parser.parse(json);
            auto folderArr = root.extract<Poco::JSON::Array::Ptr>();
            if (!folderArr.isNull())
            {
                for (size_t i = 0; i < folderArr->size(); ++i)
                {
                    auto folderObj = folderArr->getObject(static_cast<uint32_t>(i));
                    folders.emplace_back(std::move(FolderRemote::fromJSON(this, folderObj)));
                }
            }
        }
        catch (...)
        {
            return folders;
        }
    }
    return folders;
}

std::optional<std::unique_ptr<ZapFR::Engine::Folder>> ZapFR::Engine::SourceRemote::getFolder(uint64_t folderID, uint32_t /*folderFetchInfo*/)
{
    // FolderFetchInfo::Statistics is implied
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath(fmt::format("/folder/{}", folderID));
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        try
        {
            auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {});
            auto parser = Poco::JSON::Parser();
            auto root = parser.parse(json);
            auto folderObj = root.extract<Poco::JSON::Object::Ptr>();
            if (!folderObj.isNull())
            {
                return FolderRemote::fromJSON(this, folderObj);
            }
        }
        catch (...)
        {
            return {};
        }
    }
    return {};
}

uint64_t ZapFR::Engine::SourceRemote::addFolder(const std::string& title, uint64_t parentID)
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/folder");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        params["title"] = title;
        params["parentFolderID"] = std::to_string(parentID);

        auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto o = root.extract<Poco::JSON::Object::Ptr>();
        if (!o.isNull())
        {
            return o->getValue<uint64_t>(Folder::JSONIdentifierFolderID);
        }
    }
    return 0;
}

void ZapFR::Engine::SourceRemote::moveFolder(uint64_t folderID, uint64_t newParent, uint64_t newSortOrder)
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath(fmt::format("/folder/{}/move", folderID));
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        params["parentFolderID"] = std::to_string(newParent);
        params["sortOrder"] = std::to_string(newSortOrder);

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
    }
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

uint64_t ZapFR::Engine::SourceRemote::createFolderHierarchy(uint64_t /*parentID*/, const std::vector<std::string>& /*folderHierarchy*/)
{
    return 0; // TODO (OPML import)
}

/* ************************** POST STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::SourceRemote::getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter,
                                                                                        FlagColor flagColor)
{
    std::vector<std::unique_ptr<ZapFR::Engine::Post>> posts;

    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/posts");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        params["parentType"] = "source";
        params["perPage"] = std::to_string(perPage);
        params["page"] = std::to_string(page);
        params["showOnlyUnread"] = showOnlyUnread ? "true" : "false";
        params["searchFilter"] = searchFilter;
        params["flagColor"] = Flag::nameForFlagColor(flagColor);

        try
        {
            auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, params);
            auto parser = Poco::JSON::Parser();
            auto root = parser.parse(json);
            auto postArr = root.extract<Poco::JSON::Array::Ptr>();
            if (!postArr.isNull())
            {
                for (size_t i = 0; i < postArr->size(); ++i)
                {
                    auto postObj = postArr->getObject(static_cast<uint32_t>(i));
                    posts.emplace_back(std::move(PostRemote::fromJSON(postObj)));
                }
            }
        }
        catch (...)
        {
            return posts;
        }
    }

    return posts;
}

uint64_t ZapFR::Engine::SourceRemote::getTotalPostCount(bool /*showOnlyUnread*/, const std::string& /*searchFilter*/, FlagColor /*flagColor*/)
{
    return 0;
}

void ZapFR::Engine::SourceRemote::markAllAsRead()
{
}

/* ************************** LOGS STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Log>> ZapFR::Engine::SourceRemote::getLogs(uint64_t /*perPage*/, uint64_t /*page*/)
{
    return {};
}

uint64_t ZapFR::Engine::SourceRemote::getTotalLogCount()
{
    return 0;
}

/* ************************** FLAG STUFF ************************** */
std::unordered_set<ZapFR::Engine::FlagColor> ZapFR::Engine::SourceRemote::getUsedFlagColors()
{
    return {};
}

/* ************************** SCRIPT FOLDER STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::ScriptFolder>> ZapFR::Engine::SourceRemote::getScriptFolders()
{
    return {};
}

std::optional<std::unique_ptr<ZapFR::Engine::ScriptFolder>> ZapFR::Engine::SourceRemote::getScriptFolder(uint64_t /*id*/)
{
    return {};
}

void ZapFR::Engine::SourceRemote::addScriptFolder(const std::string& /*title*/)
{
}

void ZapFR::Engine::SourceRemote::removeScriptFolder(uint64_t /*scriptFolderID*/)
{
}

/* ************************** SCRIPT STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Script>> ZapFR::Engine::SourceRemote::getScripts()
{
    return {};
}

std::optional<std::unique_ptr<ZapFR::Engine::Script>> ZapFR::Engine::SourceRemote::getScript(uint64_t /*scriptID*/)
{
    return {};
}

void ZapFR::Engine::SourceRemote::removeScript(uint64_t /*scriptID*/)
{
}

void ZapFR::Engine::SourceRemote::addScript(Script::Type /*type*/, const std::string& /*filename*/, bool /*enabled*/, const std::unordered_set<Script::Event>& /*events*/,
                                            const std::optional<std::unordered_set<uint64_t>>& /*feedIDs*/)
{
}

/* ************************** SOURCE STUFF ************************** */
void ZapFR::Engine::SourceRemote::fetchStatistics()
{
    mStatistics.clear();

    mStatistics[Statistic::FeedCount] = "0";
    mStatistics[Statistic::PostCount] = "0";
    mStatistics[Statistic::FlaggedPostCount] = "0";
    mStatistics[Statistic::OldestPost] = "";
    mStatistics[Statistic::NewestPost] = "";
}
