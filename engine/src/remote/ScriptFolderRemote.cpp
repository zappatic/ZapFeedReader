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

#include "ZapFR/remote/ScriptFolderRemote.h"
#include "ZapFR/Helpers.h"
#include "ZapFR/Post.h"
#include "ZapFR/remote/PostRemote.h"
#include "ZapFR/remote/SourceRemote.h"

ZapFR::Engine::ScriptFolderRemote::ScriptFolderRemote(uint64_t id, Source* parentSource) : ScriptFolder(id, parentSource)
{
}

std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Post>>> ZapFR::Engine::ScriptFolderRemote::getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                                                                                                    const std::string& searchFilter, FlagColor flagColor)
{
    std::vector<std::unique_ptr<ZapFR::Engine::Post>> posts;
    uint64_t postCount{0};

    auto remoteSource = dynamic_cast<SourceRemote*>(mParentSource);
    auto uri = remoteSource->remoteURL();
    if (remoteSource->remoteURLIsValid())
    {
        uri.setPath("/posts");
        auto creds = Poco::Net::HTTPCredentials(remoteSource->remoteLogin(), remoteSource->remotePassword());

        std::map<std::string, std::string> params;
        params["parentType"] = "scriptfolder";
        params["parentID"] = std::to_string(mID);
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

void ZapFR::Engine::ScriptFolderRemote::update(const std::string& title)
{
    auto remoteSource = dynamic_cast<SourceRemote*>(mParentSource);
    auto uri = remoteSource->remoteURL();
    if (remoteSource->remoteURLIsValid())
    {
        uri.setPath(fmt::format("/scriptfolder/{}", mID));
        auto creds = Poco::Net::HTTPCredentials(remoteSource->remoteLogin(), remoteSource->remotePassword());

        std::map<std::string, std::string> params;
        params["title"] = title;

        Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_PATCH, creds, params);
    }
}

std::unique_ptr<ZapFR::Engine::ScriptFolder> ZapFR::Engine::ScriptFolderRemote::fromJSON(Source* parentSource, const Poco::JSON::Object::Ptr o)
{
    auto scriptFolderID = o->getValue<uint64_t>(ScriptFolder::JSONIdentifierScriptFolderID);

    auto scriptFolder = std::make_unique<ScriptFolderRemote>(scriptFolderID, parentSource);
    scriptFolder->setTitle(o->getValue<std::string>(ScriptFolder::JSONIdentifierScriptFolderTitle));

    return scriptFolder;
}