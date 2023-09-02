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

#include "ZapFR/remote/FolderRemote.h"
#include "ZapFR/Log.h"
#include "ZapFR/Post.h"

ZapFR::Engine::FolderRemote::FolderRemote(uint64_t id, uint64_t parent) : Folder(id, parent)
{
}

std::vector<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::FolderRemote::getPosts(uint64_t /*perPage*/, uint64_t /*page*/, bool /*showOnlyUnread*/,
                                                                                        const std::string& /*searchFilter*/, FlagColor /*flagColor*/)
{
    return {};
}

uint64_t ZapFR::Engine::FolderRemote::getTotalPostCount(bool /*showOnlyUnread*/, const std::string& /*searchFilter*/, FlagColor /*flagColor*/)
{
    return 0;
}

std::unordered_set<uint64_t> ZapFR::Engine::FolderRemote::markAllAsRead()
{
    return {};
}

std::vector<uint64_t> ZapFR::Engine::FolderRemote::folderAndSubfolderIDs() const
{
    return {};
}

std::vector<uint64_t> ZapFR::Engine::FolderRemote::feedIDsInFoldersAndSubfolders() const
{
    return {};
}

std::vector<std::unique_ptr<ZapFR::Engine::Log>> ZapFR::Engine::FolderRemote::getLogs(uint64_t /*perPage*/, uint64_t /*page*/)
{
    return {};
}

uint64_t ZapFR::Engine::FolderRemote::getTotalLogCount()
{
    return 0;
}
