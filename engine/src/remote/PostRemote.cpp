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

#include "ZapFR/remote/PostRemote.h"
#include "ZapFR/Helpers.h"

ZapFR::Engine::PostRemote::PostRemote(uint64_t id) : Post(id)
{
}

std::unique_ptr<ZapFR::Engine::Post> ZapFR::Engine::PostRemote::createFromJSON(const Poco::JSON::Object::Ptr o)
{
    auto postID = o->getValue<uint64_t>(Post::JSONIdentifierPostID);
    auto post = std::make_unique<PostRemote>(postID);
    post->fromJSON(o);
    return post;
}
