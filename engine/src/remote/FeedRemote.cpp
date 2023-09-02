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

#include "ZapFR/remote/FeedRemote.h"
#include "ZapFR/Helpers.h"
#include "ZapFR/Log.h"
#include "ZapFR/Post.h"

ZapFR::Engine::FeedRemote::FeedRemote(uint64_t id) : Feed(id)
{
}

std::vector<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::FeedRemote::getPosts(uint64_t /*perPage*/, uint64_t /*page*/, bool /*showOnlyUnread*/,
                                                                                      const std::string& /*searchFilter*/, FlagColor /*flagColor*/)
{
    return {};
}

uint64_t ZapFR::Engine::FeedRemote::getTotalPostCount(bool /*showOnlyUnread*/, const std::string& /*searchFilter*/, FlagColor /*flagColor*/)
{
    return 0;
}

std::optional<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::FeedRemote::getPost(uint64_t /*postID*/)
{
    return {};
}

bool ZapFR::Engine::FeedRemote::refresh(const std::optional<std::string>& /*feedXML*/)
{
    return true;
}

void ZapFR::Engine::FeedRemote::markAllAsRead()
{
}

void ZapFR::Engine::FeedRemote::markAsRead(uint64_t /*postID*/)
{
}

void ZapFR::Engine::FeedRemote::markAsUnread(uint64_t /*postID*/)
{
}

void ZapFR::Engine::FeedRemote::refreshIcon()
{
}

void ZapFR::Engine::FeedRemote::removeIcon()
{
}

std::string ZapFR::Engine::FeedRemote::icon() const
{
    return "";
}

std::vector<std::unique_ptr<ZapFR::Engine::Log>> ZapFR::Engine::FeedRemote::getLogs(uint64_t /*perPage*/, uint64_t /*page*/)
{
    return {};
}

uint64_t ZapFR::Engine::FeedRemote::getTotalLogCount()
{
    return 0;
}

bool ZapFR::Engine::FeedRemote::fetchData()
{
    return true;
}

void ZapFR::Engine::FeedRemote::fetchStatistics()
{
}

void ZapFR::Engine::FeedRemote::updateProperties(const std::string& /*feedURL*/, std::optional<uint64_t> /*refreshIntervalInSeconds*/)
{
}
