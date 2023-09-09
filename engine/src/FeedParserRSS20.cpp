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

#include "ZapFR/FeedParserRSS20.h"

ZapFR::Engine::FeedParserRSS20::FeedParserRSS20(Poco::XML::Document* xmlDoc, const std::string& url) : FeedParser(xmlDoc, url)
{
}

std::string ZapFR::Engine::FeedParserRSS20::guid() const
{
    return "";
}

std::string ZapFR::Engine::FeedParserRSS20::title() const
{
    return fetchNodeValue("/channel/title");
}

std::string ZapFR::Engine::FeedParserRSS20::subtitle() const
{
    return "";
}

std::string ZapFR::Engine::FeedParserRSS20::link() const
{
    return fetchNodeValue("/channel/link");
}

std::string ZapFR::Engine::FeedParserRSS20::description() const
{
    return fetchNodeValue("/channel/description");
}

std::string ZapFR::Engine::FeedParserRSS20::language() const
{
    return fetchNodeValue("/channel/language");
}

std::string ZapFR::Engine::FeedParserRSS20::copyright() const
{
    return fetchNodeValue("/channel/copyright");
}

std::string ZapFR::Engine::FeedParserRSS20::iconURL() const
{
    return fetchNodeValue("/channel/image/url");
}

std::vector<ZapFR::Engine::FeedParser::Item> ZapFR::Engine::FeedParserRSS20::items() const
{
    std::vector<Item> items;

    auto itemList = mXMLDoc->getElementsByTagName("item");
    for (size_t i = 0; i < itemList->length(); ++i)
    {
        auto itemNode = itemList->item(i);

        Item item;
        item.title = fetchNodeValue(itemNode, "title");
        item.link = fetchNodeValue(itemNode, "link");
        item.description = fetchNodeValue(itemNode, "description");
        item.author = fetchNodeValue(itemNode, "author");

        std::stringstream categories;
        auto catList = dynamic_cast<Poco::XML::Element*>(itemNode)->getElementsByTagName("category");
        for (size_t j = 0; j < catList->length(); ++j)
        {
            auto catNode = catList->item(j);
            categories << catNode->innerText() << ", ";
        }
        item.category = categories.str();
        Poco::trimInPlace(item.category);
        catList->release();

        item.commentsURL = fetchNodeValue(itemNode, "comments");

        // auto enclosureNode = fetchNode(itemNode, "enclosure");
        // if (enclosureNode != nullptr)
        // {
        //     auto enclosureEl = dynamic_cast<Poco::XML::Element*>(enclosureNode);
        //     item.enclosureURL = enclosureEl->hasAttribute("url") ? enclosureEl->getAttribute("url") : "";
        //     item.enclosureLength = enclosureEl->hasAttribute("length") ? enclosureEl->getAttribute("length") : "";
        //     item.enclosureMimeType = enclosureEl->hasAttribute("type") ? enclosureEl->getAttribute("type") : "";
        // }

        auto guidNode = fetchNode(itemNode, "guid");
        if (guidNode != nullptr)
        {
            item.guid = guidNode->innerText();
        }
        else
        {
            // create a guid out of either title or description (all are optional, but either title or description must be present)
            auto guidSrc = item.title;
            if (guidSrc.empty())
            {
                guidSrc = item.description;
            }
            if (guidSrc.empty()) // shouldn't happen, but just in case, use a random uuid
            {
                guidSrc = Poco::UUIDGenerator::defaultGenerator().createRandom().toString();
            }

            Poco::MD5Engine md5;
            Poco::DigestOutputStream ds(md5);
            ds << guidSrc;
            ds.close();
            item.guid = Poco::DigestEngine::digestToHex(md5.digest());
        }

        item.datePublished = fetchNodeValue(itemNode, "pubDate");
        int tzDiff;
        auto parsedDate = Poco::DateTimeParser::parse(Poco::DateTimeFormat::RFC1123_FORMAT, item.datePublished, tzDiff);
        parsedDate.makeUTC(tzDiff);
        item.datePublished = Poco::DateTimeFormatter::format(parsedDate, Poco::DateTimeFormat::ISO8601_FORMAT);

        items.emplace_back(item);
    }
    itemList->release();

    return items;
}
