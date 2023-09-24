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

#include "ZapFR/dummy/FeedDummy.h"

ZapFR::Engine::FeedDummy::FeedDummy(uint64_t id, Source* parentSource) : Feed(id, parentSource)
{
}

std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Post>>> ZapFR::Engine::FeedDummy::getPosts(uint64_t /*perPage*/, uint64_t /*page*/, bool /*showOnlyUnread*/,
                                                                                                           const std::string& /*searchFilter*/, FlagColor /*flagColor*/)
{
    throw std::runtime_error("Not implemented");
}

std::optional<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::FeedDummy::getPost(uint64_t /*postID*/)
{
    throw std::runtime_error("Not implemented");
}

void ZapFR::Engine::FeedDummy::refresh()
{
    throw std::runtime_error("Not implemented");
}

void ZapFR::Engine::FeedDummy::markAsRead()
{
    throw std::runtime_error("Not implemented");
}

std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Log>>> ZapFR::Engine::FeedDummy::getLogs(uint64_t /*perPage*/, uint64_t /*page*/)
{
    throw std::runtime_error("Not implemented");
}

void ZapFR::Engine::FeedDummy::clearLogs()
{
    throw std::runtime_error("Not implemented");
}

void ZapFR::Engine::FeedDummy::updateProperties(const std::string& /*feedURL*/, std::optional<uint64_t> /*refreshIntervalInSeconds*/)
{
    throw std::runtime_error("Not implemented");
}
