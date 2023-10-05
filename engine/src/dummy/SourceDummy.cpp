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

#include "ZapFR/dummy/SourceDummy.h"

ZapFR::Engine::SourceDummy::SourceDummy(uint64_t id) : Source(id)
{
}

/* ************************** FEED STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceDummy::getFeeds(uint32_t /*fetchInfo*/)
{
    throw std::runtime_error("Not implemented");
}

std::optional<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceDummy::getFeed(uint64_t /*feedID*/, uint32_t /*fetchInfo*/)
{
    throw std::runtime_error("Not implemented");
}

std::optional<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceDummy::addFeed(const std::string& /*url*/, uint64_t /*folder*/)
{
    throw std::runtime_error("Not implemented");
}

std::unordered_map<uint64_t, uint64_t> ZapFR::Engine::SourceDummy::moveFeed(uint64_t /*feedID*/, uint64_t /*newFolder*/, uint64_t /*newSortOrder*/)
{
    throw std::runtime_error("Not implemented");
}

void ZapFR::Engine::SourceDummy::removeFeed(uint64_t /*feedID*/)
{
    throw std::runtime_error("Not implemented");
}

/* ************************** FOLDER STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Folder>> ZapFR::Engine::SourceDummy::getFolders(uint64_t /*parent*/, uint32_t /*fetchInfo*/)
{
    throw std::runtime_error("Not implemented");
}

std::optional<std::unique_ptr<ZapFR::Engine::Folder>> ZapFR::Engine::SourceDummy::getFolder(uint64_t /*folderID*/, uint32_t /*fetchInfo*/)
{
    throw std::runtime_error("Not implemented");
}

std::tuple<uint64_t, uint64_t> ZapFR::Engine::SourceDummy::addFolder(const std::string& /*title*/, uint64_t /*parentID*/)
{
    throw std::runtime_error("Not implemented");
}

std::unordered_map<uint64_t, uint64_t> ZapFR::Engine::SourceDummy::moveFolder(uint64_t /*folderID*/, uint64_t /*newParent*/, uint64_t /*newSortOrder*/)
{
    throw std::runtime_error("Not implemented");
}

void ZapFR::Engine::SourceDummy::removeFolder(uint64_t /*folderID*/)
{
    throw std::runtime_error("Not implemented");
}

/* ************************** POST STUFF ************************** */
std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Post>>> ZapFR::Engine::SourceDummy::getPosts(uint64_t /*perPage*/, uint64_t /*page*/, bool /*showOnlyUnread*/,
                                                                                                             const std::string& /*searchFilter*/, FlagColor /*flagColor*/)
{
    throw std::runtime_error("Not implemented");
}

void ZapFR::Engine::SourceDummy::markAsRead(uint64_t /*maxPostID*/)
{
    throw std::runtime_error("Not implemented");
}

void ZapFR::Engine::SourceDummy::setPostsReadStatus(bool markAsRead, const std::vector<std::tuple<uint64_t, uint64_t>>& /*feedsAndPostIDs*/)
{
    if (markAsRead)
    {
        mAssociatedPost->markAsRead();
    }
    else
    {
        mAssociatedPost->markAsUnread();
    }
}

void ZapFR::Engine::SourceDummy::setPostsFlagStatus(bool markFlagged, const std::unordered_set<FlagColor>& flagColors,
                                                    const std::vector<std::tuple<uint64_t, uint64_t>>& /*feedsAndPostIDs*/)
{
    if (markFlagged)
    {
        for (const auto& fc : flagColors)
        {
            mAssociatedPost->markFlagged(fc);
        }
    }
    else
    {
        for (const auto& fc : flagColors)
        {
            mAssociatedPost->markUnflagged(fc);
        }
    }
}

void ZapFR::Engine::SourceDummy::assignPostsToScriptFolder(uint64_t scriptFolderID, bool assign, const std::vector<std::tuple<uint64_t, uint64_t>>& /*feedsAndPostIDs*/)
{
    if (assign)
    {
        mAssociatedPost->assignToScriptFolder(scriptFolderID);
    }
    else
    {
        mAssociatedPost->unassignFromScriptFolder(scriptFolderID);
    }
}

Poco::JSON::Object ZapFR::Engine::SourceDummy::getStatus()
{
    throw std::runtime_error("Not implemented");
}

/* ************************** LOGS STUFF ************************** */
std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Log>>> ZapFR::Engine::SourceDummy::getLogs(uint64_t /*perPage*/, uint64_t /*page*/)
{
    throw std::runtime_error("Not implemented");
}

/* ************************** FLAG STUFF ************************** */
std::unordered_set<ZapFR::Engine::FlagColor> ZapFR::Engine::SourceDummy::getUsedFlagColors()
{
    throw std::runtime_error("Not implemented");
}

/* ************************** SCRIPT FOLDER STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::ScriptFolder>> ZapFR::Engine::SourceDummy::getScriptFolders()
{
    throw std::runtime_error("Not implemented");
}

std::optional<std::unique_ptr<ZapFR::Engine::ScriptFolder>> ZapFR::Engine::SourceDummy::getScriptFolder(uint64_t /*id*/, uint32_t /*fetchInfo*/)
{
    throw std::runtime_error("Not implemented");
}

void ZapFR::Engine::SourceDummy::addScriptFolder(const std::string& /*title*/, bool /*showTotal*/, bool /*showUnread*/)
{
    throw std::runtime_error("Not implemented");
}

void ZapFR::Engine::SourceDummy::removeScriptFolder(uint64_t /*scriptFolderID*/)
{
    throw std::runtime_error("Not implemented");
}

/* ************************** SCRIPT STUFF ************************** */
std::vector<std::unique_ptr<ZapFR::Engine::Script>> ZapFR::Engine::SourceDummy::getScripts()
{
    throw std::runtime_error("Not implemented");
}

std::optional<std::unique_ptr<ZapFR::Engine::Script>> ZapFR::Engine::SourceDummy::getScript(uint64_t /*scriptID*/, uint32_t /*fetchInfo*/)
{
    throw std::runtime_error("Not implemented");
}

void ZapFR::Engine::SourceDummy::removeScript(uint64_t /*scriptID*/)
{
    throw std::runtime_error("Not implemented");
}

void ZapFR::Engine::SourceDummy::addScript(Script::Type /*type*/, const std::string& /*title*/, bool /*enabled*/, const std::unordered_set<Script::Event>& /*events*/,
                                           const std::optional<std::unordered_set<uint64_t>>& /*feedIDs*/, const std::string& /*script*/)
{
    throw std::runtime_error("Not implemented");
}

/* ************************** SOURCE STUFF ************************** */
void ZapFR::Engine::SourceDummy::fetchStatistics()
{
    throw std::runtime_error("Not implemented");
}

void ZapFR::Engine::SourceDummy::fetchThumbnailData()
{
    throw std::runtime_error("Not implemented");
}

std::unordered_set<uint64_t> ZapFR::Engine::SourceDummy::importOPML(const std::string& /*opml*/, uint64_t /*parentFolderID*/)
{
    throw std::runtime_error("Not implemented");
}

void ZapFR::Engine::SourceDummy::clearLogs()
{
    throw std::runtime_error("Not implemented");
}
