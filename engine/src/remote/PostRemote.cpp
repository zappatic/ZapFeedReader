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

void ZapFR::Engine::PostRemote::markFlagged(FlagColor /*flagColor*/)
{
}

void ZapFR::Engine::PostRemote::markUnflagged(FlagColor /*flagColor*/)
{
}

void ZapFR::Engine::PostRemote::markAsRead()
{
}

void ZapFR::Engine::PostRemote::markAsUnread()
{
}

void ZapFR::Engine::PostRemote::assignToScriptFolder(uint64_t /*scriptFolderID*/)
{
}

void ZapFR::Engine::PostRemote::unassignFromScriptFolder(uint64_t /*scriptFolderID*/)
{
}

std::unique_ptr<ZapFR::Engine::Post> ZapFR::Engine::PostRemote::fromJSON(const Poco::JSON::Object::Ptr o)
{
    auto postID = o->getValue<uint64_t>(Post::JSONIdentifierPostID);

    auto post = std::make_unique<PostRemote>(postID);
    post->setIsRead(o->getValue<bool>(Post::JSONIdentifierPostIsRead));
    post->setFeedID(o->getValue<uint64_t>(Post::JSONIdentifierPostFeedID));
    post->setFeedTitle(o->getValue<std::string>(Post::JSONIdentifierPostFeedTitle));
    post->setTitle(o->getValue<std::string>(Post::JSONIdentifierPostTitle));
    post->setDescription(o->getValue<std::string>(Post::JSONIdentifierPostDescription));
    post->setAuthor(o->getValue<std::string>(Post::JSONIdentifierPostAuthor));
    post->setCommentsURL(o->getValue<std::string>(Post::JSONIdentifierPostCommentsURL));
    post->setEnclosureURL(o->getValue<std::string>(Post::JSONIdentifierPostEnclosureURL));
    post->setEnclosureLength(o->getValue<std::string>(Post::JSONIdentifierPostEnclosureLength));
    post->setEnclosureMimeType(o->getValue<std::string>(Post::JSONIdentifierPostEnclosureMimetype));
    post->setGuid(o->getValue<std::string>(Post::JSONIdentifierPostGuid));
    post->setGuidIsPermalink(o->getValue<bool>(Post::JSONIdentifierPostGuidIsPermalink));
    post->setDatePublished(o->getValue<std::string>(Post::JSONIdentifierPostDatePublished));
    post->setSourceURL(o->getValue<std::string>(Post::JSONIdentifierPostSourceURL));
    post->setSourceTitle(o->getValue<std::string>(Post::JSONIdentifierPostSourceTitle));

    std::unordered_set<FlagColor> flagColors;
    std::vector<std::string> flagColorNames;
    Helpers::splitString(o->getValue<std::string>(JSONIdentifierPostFlagColors), ',', flagColorNames);
    for (auto& name : flagColorNames)
    {
        Poco::trimInPlace(name);
        flagColors.insert(Flag::flagColorForName(name));
    }
    post->setFlagColors(flagColors);

    return post;
}