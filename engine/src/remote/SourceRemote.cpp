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
#include "ZapFR/Log.h"
#include "ZapFR/Post.h"
#include "ZapFR/ScriptFolder.h"

ZapFR::Engine::SourceRemote::SourceRemote(uint64_t id) : Source(id)
{
}

/* ************************** FEED STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceRemote::getFeeds()
{
    return {};
}

std::optional<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceRemote::getFeed(uint64_t /*feedID*/, bool /*fetchData*/)
{
    return {};
}

uint64_t ZapFR::Engine::SourceRemote::addFeed(const std::string& /*url*/, uint64_t /*folder*/)
{
    return 0;
}

void ZapFR::Engine::SourceRemote::moveFeed(uint64_t /*feedID*/, uint64_t /*newFolder*/, uint64_t /*newSortOrder*/)
{
}

void ZapFR::Engine::SourceRemote::removeFeed(uint64_t /*feedID*/)
{
}

/* ************************** FOLDER STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Folder>> ZapFR::Engine::SourceRemote::getFolders(uint64_t /*parent*/)
{
    return {};
}

std::optional<std::unique_ptr<ZapFR::Engine::Folder>> ZapFR::Engine::SourceRemote::getFolder(uint64_t /*folderID*/)
{
    return {};
}

uint64_t ZapFR::Engine::SourceRemote::addFolder(const std::string& /*title*/, uint64_t /*parentID*/)
{
    return 0;
}

void ZapFR::Engine::SourceRemote::moveFolder(uint64_t /*folderID*/, uint64_t /*newParent*/, uint64_t /*newSortOrder*/)
{
}

void ZapFR::Engine::SourceRemote::removeFolder(uint64_t /*folder*/)
{
}

uint64_t ZapFR::Engine::SourceRemote::createFolderHierarchy(uint64_t /*parentID*/, const std::vector<std::string>& /*folderHierarchy*/)
{
    return 0;
}

/* ************************** POST STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::SourceRemote::getPosts(uint64_t /*perPage*/, uint64_t /*page*/, bool /*showOnlyUnread*/,
                                                                                        const std::string& /*searchFilter*/, FlagColor /*flagColor*/)
{
    return {};
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
