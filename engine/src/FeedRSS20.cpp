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

#include "FeedRSS20.h"

ZapFR::Engine::FeedRSS20::FeedRSS20(Poco::XML::Document* xmlDoc, const std::string& url) : Feed(xmlDoc, url)
{
}

std::string ZapFR::Engine::FeedRSS20::guid() const
{
    return "";
}

std::string ZapFR::Engine::FeedRSS20::title() const
{
    return fetchNodeValue("/channel/title");
}

std::string ZapFR::Engine::FeedRSS20::subtitle() const
{
    return "";
}

std::string ZapFR::Engine::FeedRSS20::link() const
{
    return fetchNodeValue("/channel/link");
}

std::string ZapFR::Engine::FeedRSS20::description() const
{
    return fetchNodeValue("/channel/description");
}

std::string ZapFR::Engine::FeedRSS20::language() const
{
    return fetchNodeValue("/channel/language");
}

std::string ZapFR::Engine::FeedRSS20::copyright() const
{
    return fetchNodeValue("/channel/copyright");
}

std::vector<ZapFR::Engine::Feed::Item> ZapFR::Engine::FeedRSS20::items() const
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

        auto enclosureNode = fetchNode(itemNode, "enclosure");
        if (enclosureNode != nullptr)
        {
            auto enclosureEl = dynamic_cast<Poco::XML::Element*>(enclosureNode);
            item.enclosureURL = enclosureEl->hasAttribute("url") ? enclosureEl->getAttribute("url") : "";
            item.enclosureLength = enclosureEl->hasAttribute("length") ? enclosureEl->getAttribute("length") : "";
            item.enclosureMimeType = enclosureEl->hasAttribute("type") ? enclosureEl->getAttribute("type") : "";
        }

        auto guidNode = fetchNode(itemNode, "guid");
        if (guidNode != nullptr)
        {
            auto guidEl = dynamic_cast<Poco::XML::Element*>(guidNode);
            item.guid = guidNode->innerText();
            if (guidEl->hasAttribute("isPermaLink"))
            {
                item.guidIsPermalink = guidEl->getAttribute("isPermaLink") == "true";
            }
        }

        item.datePublished = fetchNodeValue(itemNode, "pubDate");
        int tzDiff;
        auto parsedDate = Poco::DateTimeParser::parse(Poco::DateTimeFormat::RFC822_FORMAT, item.datePublished, tzDiff);
        parsedDate.makeUTC(tzDiff);
        item.datePublished = Poco::DateTimeFormatter::format(parsedDate, Poco::DateTimeFormat::ISO8601_FORMAT);

        auto sourceNode = fetchNode(itemNode, "source");
        if (sourceNode != nullptr)
        {
            auto sourceEl = dynamic_cast<Poco::XML::Element*>(sourceNode);
            item.sourceURL = sourceEl->hasAttribute("url") ? sourceEl->getAttribute("url") : "";
            item.sourceTitle = sourceNode->innerText();
        }

        items.emplace_back(item);
    }
    itemList->release();

    return items;
}
