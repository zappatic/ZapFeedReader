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
#include "ZapFR/Global.h"
#include "ZapFR/Helpers.h"

ZapFR::Engine::Post::Post(uint64_t id) : mID(id)
{
}

Poco::JSON::Object ZapFR::Engine::Post::toJSON()
{
    Poco::JSON::Object o;
    o.set(JSON::Post::ID, mID);
    o.set(JSON::Post::IsRead, mIsRead);
    o.set(JSON::Post::FeedID, mFeedID);
    o.set(JSON::Post::FeedTitle, mFeedTitle);
    o.set(JSON::Post::FeedLink, mFeedLink);
    o.set(JSON::Post::Title, mTitle);
    o.set(JSON::Post::Link, mLink);
    o.set(JSON::Post::Content, mContent);
    o.set(JSON::Post::Author, mAuthor);
    o.set(JSON::Post::CommentsURL, mCommentsURL);
    o.set(JSON::Post::Guid, mGuid);
    o.set(JSON::Post::DatePublished, mDatePublished);
    o.set(JSON::Post::Thumbnail, mThumbnail);

    std::vector<std::string> flagColors;
    for (const auto& flagColor : mFlagColors)
    {
        flagColors.emplace_back(Flag::nameForFlagColor(flagColor));
    }
    o.set(JSON::Post::FlagColors, Helpers::joinString(flagColors, ","));

    Poco::JSON::Array enclosuresArr;
    for (const auto& e : mEnclosures)
    {
        Poco::JSON::Object enclosureObj;
        enclosureObj.set(JSON::Post::EnclosureURL, e.url);
        enclosureObj.set(JSON::Post::EnclosureMimeType, e.mimeType);
        enclosureObj.set(JSON::Post::EnclosureSize, e.size);
        enclosuresArr.add(enclosureObj);
    }
    o.set(JSON::Post::Enclosures, enclosuresArr);

    Poco::JSON::Array categoryArr;
    for (const auto& cat : mCategories)
    {
        Poco::JSON::Object categoryObj;
        categoryObj.set(JSON::Post::CategoryID, cat.id);
        categoryObj.set(JSON::Post::CategoryTitle, cat.title);
        categoryArr.add(categoryObj);
    }
    o.set(JSON::Post::Categories, categoryArr);

    return o;
}

void ZapFR::Engine::Post::fromJSON(const Poco::JSON::Object::Ptr o)
{
    setIsRead(o->getValue<bool>(JSON::Post::IsRead));
    setFeedID(o->getValue<uint64_t>(JSON::Post::FeedID));
    setFeedTitle(o->getValue<std::string>(JSON::Post::FeedTitle));
    setFeedLink(o->getValue<std::string>(JSON::Post::FeedLink));
    setTitle(o->getValue<std::string>(JSON::Post::Title));
    setLink(o->getValue<std::string>(JSON::Post::Link));
    setContent(o->getValue<std::string>(JSON::Post::Content));
    setAuthor(o->getValue<std::string>(JSON::Post::Author));
    setCommentsURL(o->getValue<std::string>(JSON::Post::CommentsURL));
    setGuid(o->getValue<std::string>(JSON::Post::Guid));
    setDatePublished(o->getValue<std::string>(JSON::Post::DatePublished));
    setThumbnail(o->getValue<std::string>(JSON::Post::Thumbnail));

    std::unordered_set<FlagColor> flagColors;
    std::vector<std::string> flagColorNames;
    Helpers::splitString(o->getValue<std::string>(JSON::Post::FlagColors), ',', flagColorNames);
    for (auto& name : flagColorNames)
    {
        Poco::trimInPlace(name);
        flagColors.insert(Flag::flagColorForName(name));
    }
    setFlagColors(flagColors);

    auto enclosuresArr = o->getArray(JSON::Post::Enclosures);
    if (enclosuresArr != nullptr)
    {
        for (size_t i = 0; i < enclosuresArr->size(); ++i)
        {
            auto enclosureObj = enclosuresArr->getObject(static_cast<uint32_t>(i));
            addEnclosure(enclosureObj->getValue<std::string>(JSON::Post::EnclosureURL), enclosureObj->getValue<std::string>(JSON::Post::EnclosureMimeType),
                         enclosureObj->getValue<uint64_t>(JSON::Post::EnclosureSize));
        }
    }
    auto categoryArr = o->getArray(JSON::Post::Categories);
    if (categoryArr != nullptr)
    {
        for (size_t i = 0; i < categoryArr->size(); ++i)
        {
            auto catObj = categoryArr->getObject(static_cast<uint32_t>(i));
            mCategories.emplace_back(catObj->getValue<uint64_t>(JSON::Post::CategoryID), catObj->getValue<std::string>(JSON::Post::CategoryTitle));
        }
    }
}

void ZapFR::Engine::Post::removeEnclosure(uint64_t index)
{
    if (index < mEnclosures.size())
    {
        mEnclosures.erase(mEnclosures.begin() + static_cast<int64_t>(index));
    }
}

void ZapFR::Engine::Post::updateEnclosure(uint64_t index, const std::string& url, const std::string& mimeType, uint64_t size)
{
    if (index < mEnclosures.size())
    {
        auto& enclosure = mEnclosures.at(index);
        enclosure.url = url;
        enclosure.mimeType = mimeType;
        enclosure.size = size;
    }
}

bool ZapFR::Engine::Post::hasCategory(const std::string& title) const
{
    for (const auto& cat : mCategories)
    {
        if (Poco::icompare(cat.title, title) == 0)
        {
            return true;
        }
    }
    return false;
}
