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
#include "ZapFR/remote/ScriptFolderRemote.h"
#include "ZapFR/remote/ScriptRemote.h"

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

            auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {});
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
        params["parentType"] = "source";
        params["perPage"] = std::to_string(perPage);
        params["page"] = std::to_string(page);
        params["showOnlyUnread"] = showOnlyUnread ? "true" : "false";
        params["searchFilter"] = searchFilter;
        params["flagColor"] = Flag::nameForFlagColor(flagColor);

        auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
        if (!rootObj.isNull())
        {
            postCount = rootObj->getValue<uint64_t>("count");
            auto postArr = rootObj->getArray("posts");
            if (!postArr.isNull())
            {
                for (size_t i = 0; i < postArr->size(); ++i)
                {
                    auto postObj = postArr->getObject(static_cast<uint32_t>(i));
                    posts.emplace_back(std::move(PostRemote::fromJSON(postObj)));
                }
            }
        }
    }
    return std::make_tuple(postCount, std::move(posts));
}

void ZapFR::Engine::SourceRemote::markAllAsRead()
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/mark-as-read");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, {});
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
            o.set("feedID", feedID);
            o.set("postID", postID);
            arr.add(o);
        }
        std::stringstream ss;
        Poco::JSON::Stringifier::stringify(arr, ss);

        std::map<std::string, std::string> params;
        params["feedsAndPostIDs"] = ss.str();
        params["markAsRead"] = markAsRead ? "true" : "false";

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
            o.set("feedID", feedID);
            o.set("postID", postID);
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
        params["feedsAndPostIDs"] = idSS.str();
        params["flagColors"] = colorSS.str();
        params["markFlagged"] = markFlagged ? "true" : "false";

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
    }
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
        params["parentType"] = "source";
        params["perPage"] = std::to_string(perPage);
        params["page"] = std::to_string(page);

        auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, params);
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
        if (!rootObj.isNull())
        {
            logCount = rootObj->getValue<uint64_t>("count");
            auto logArr = rootObj->getArray("logs");
            if (!logArr.isNull())
            {
                for (size_t i = 0; i < logArr->size(); ++i)
                {
                    auto logObj = logArr->getObject(static_cast<uint32_t>(i));
                    logs.emplace_back(std::move(Log::fromJSON(logObj)));
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

        auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {});
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto rootArr = root.extract<Poco::JSON::Array::Ptr>();
        if (!rootArr.isNull())
        {
            for (size_t i = 0; i < rootArr->size(); ++i)
            {
                try
                {
                    flagColors.insert(Flag::flagColorForName(rootArr->getElement<std::string>(static_cast<int32_t>(i))));
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

        auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {});
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto scriptFolderArr = root.extract<Poco::JSON::Array::Ptr>();
        if (!scriptFolderArr.isNull())
        {
            for (size_t i = 0; i < scriptFolderArr->size(); ++i)
            {
                auto scriptFolderObj = scriptFolderArr->getObject(static_cast<uint32_t>(i));
                scriptFolders.emplace_back(std::move(ScriptFolderRemote::fromJSON(this, scriptFolderObj)));
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

            auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {});
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

void ZapFR::Engine::SourceRemote::addScriptFolder(const std::string& title)
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/scriptfolder");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        params["title"] = title;

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

        auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {});
        auto parser = Poco::JSON::Parser();
        auto root = parser.parse(json);
        auto scriptArr = root.extract<Poco::JSON::Array::Ptr>();
        if (!scriptArr.isNull())
        {
            for (size_t i = 0; i < scriptArr->size(); ++i)
            {
                auto scriptObj = scriptArr->getObject(static_cast<uint32_t>(i));
                scripts.emplace_back(std::move(ScriptRemote::fromJSON(this, scriptObj)));
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

            auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {});
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

void ZapFR::Engine::SourceRemote::addScript(Script::Type /*type*/, const std::string& filename, bool enabled, const std::unordered_set<Script::Event>& events,
                                            const std::optional<std::unordered_set<uint64_t>>& feedIDs)
{
    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/script");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        std::map<std::string, std::string> params;
        params["type"] = Script::msTypeLuaIdentifier; // forced to lua
        params["filename"] = filename;
        params["isEnabled"] = enabled ? "true" : "false";
        params["runOnEvents"] = Script::runOnEventsString(events);
        params["runOnFeedIDs"] = Script::runOnFeedIDsString(feedIDs);

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_POST, creds, params);
    }
}

/* ************************** SOURCE STUFF ************************** */
void ZapFR::Engine::SourceRemote::fetchStatistics()
{
    mStatistics.clear();

    auto uri = remoteURL();
    if (mRemoteURLIsValid)
    {
        uri.setPath("/statistics");
        auto creds = Poco::Net::HTTPCredentials(mRemoteLogin, mRemotePassword);

        auto json = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {});
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
