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

#include "ZapFR/base/Post.h"
#include "ZapFR/Helpers.h"

ZapFR::Engine::Post::Post(uint64_t id) : mID(id)
{
}

Poco::JSON::Object ZapFR::Engine::Post::toJSON()
{
    Poco::JSON::Object o;
    o.set(JSONIdentifierPostID, mID);
    o.set(JSONIdentifierPostIsRead, mIsRead);
    o.set(JSONIdentifierPostFeedID, mFeedID);
    o.set(JSONIdentifierPostFeedTitle, mFeedTitle);
    o.set(JSONIdentifierPostFeedLink, mFeedLink);
    o.set(JSONIdentifierPostTitle, mTitle);
    o.set(JSONIdentifierPostLink, mLink);
    o.set(JSONIdentifierPostContent, mContent);
    o.set(JSONIdentifierPostAuthor, mAuthor);
    o.set(JSONIdentifierPostCommentsURL, mCommentsURL);
    o.set(JSONIdentifierPostGuid, mGuid);
    o.set(JSONIdentifierPostDatePublished, mDatePublished);

    std::vector<std::string> flagColors;
    for (const auto& flagColor : mFlagColors)
    {
        flagColors.emplace_back(Flag::nameForFlagColor(flagColor));
    }
    o.set(Post::JSONIdentifierPostFlagColors, Helpers::joinString(flagColors, ","));

    Poco::JSON::Array enclosuresArr;
    for (const auto& e : mEnclosures)
    {
        Poco::JSON::Object enclosureObj;
        enclosureObj.set(JSONIdentifierPostEnclosureURL, e.url);
        enclosureObj.set(JSONIdentifierPostEnclosureMimeType, e.mimeType);
        enclosureObj.set(JSONIdentifierPostEnclosureSize, e.size);
        enclosuresArr.add(enclosureObj);
    }
    o.set(Post::JSONIdentifierPostEnclosures, enclosuresArr);
    return o;
}
