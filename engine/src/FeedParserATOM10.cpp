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

#include "FeedParserAtom10.h"

ZapFR::Engine::FeedParserAtom10::FeedParserAtom10(Poco::XML::Document* xmlDoc, const std::string& url) : FeedParser(xmlDoc, url)
{
}

std::string ZapFR::Engine::FeedParserAtom10::guid() const
{
    return fetchNodeValue("/id");
}

std::string ZapFR::Engine::FeedParserAtom10::title() const
{
    return fetchNodeValue("/title");
}

std::string ZapFR::Engine::FeedParserAtom10::subtitle() const
{
    return fetchNodeValue("/subtitle");
}

std::string ZapFR::Engine::FeedParserAtom10::link() const
{
    std::string link;
    auto linkNodes = mXMLDoc->documentElement()->getElementsByTagName("link");
    for (size_t i = 0; i < linkNodes->length(); ++i)
    {
        auto linkEl = dynamic_cast<Poco::XML::Element*>(linkNodes->item(i));
        if (linkEl->hasAttribute("rel") && linkEl->getAttribute("rel") == "alternate")
        {
            link = linkEl->innerText();
            break;
        }
    }
    linkNodes->release();
    return link;
}

std::string ZapFR::Engine::FeedParserAtom10::description() const
{
    return "";
}

std::string ZapFR::Engine::FeedParserAtom10::iconURL() const
{
    return "";
}

std::string ZapFR::Engine::FeedParserAtom10::language() const
{
    auto docEl = mXMLDoc->documentElement();
    if (docEl->hasAttributeNS("xml", "lang"))
    {
        return docEl->getAttributeNS("xml", "lang");
    }
    return "";
}

std::string ZapFR::Engine::FeedParserAtom10::copyright() const
{
    return fetchNodeValue("/rights");
}

std::vector<ZapFR::Engine::FeedParser::Item> ZapFR::Engine::FeedParserAtom10::items() const
{
    std::vector<Item> items;

    auto entryList = mXMLDoc->getElementsByTagName("entry");
    for (size_t i = 0; i < entryList->length(); ++i)
    {
        auto entryNode = entryList->item(i);

        Item item;
        item.title = fetchNodeValue(entryNode, "title");
        // item.link = fetchNodeValue(entryNode, "link");
        item.description = fetchNodeValue(entryNode, "summary");
        // todo: content
        // item.author = fetchNodeValue(entryNode, "author");

        // std::stringstream categories;
        // auto catList = dynamic_cast<Poco::XML::Element*>(entryNode)->getElementsByTagName("category");
        // for (size_t j = 0; j < catList->length(); ++j)
        // {
        //     auto catNode = catList->item(j);
        //     categories << catNode->innerText() << ", ";
        // }
        // item.category = categories.str();
        // Poco::trimInPlace(item.category);
        // catList->release();

        // item.commentsURL = fetchNodeValue(entryNode, "comments");

        // auto enclosureNode = fetchNode(entryNode, "enclosure");
        // if (enclosureNode != nullptr)
        // {
        //     auto enclosureEl = dynamic_cast<Poco::XML::Element*>(enclosureNode);
        //     item.enclosureURL = enclosureEl->hasAttribute("url") ? enclosureEl->getAttribute("url") : "";
        //     item.enclosureLength = enclosureEl->hasAttribute("length") ? enclosureEl->getAttribute("length") : "";
        //     item.enclosureMimeType = enclosureEl->hasAttribute("type") ? enclosureEl->getAttribute("type") : "";
        // }

        item.guid = fetchNodeValue(entryNode, "id");
        item.guidIsPermalink = true;
        // auto guidNode = fetchNode(entryNode, "guid");
        // if (guidNode != nullptr)
        // {
        //     auto guidEl = dynamic_cast<Poco::XML::Element*>(guidNode);
        //     item.guid = guidNode->innerText();
        //     if (guidEl->hasAttribute("isPermaLink"))
        //     {
        //         item.guidIsPermalink = guidEl->getAttribute("isPermaLink") == "true";
        //     }
        // }

        item.datePublished = fetchNodeValue(entryNode, "updated");
        int tzDiff;
        auto parsedDate = Poco::DateTimeParser::parse(Poco::DateTimeFormat::ISO8601_FORMAT, item.datePublished, tzDiff); // actually RFC 3339
        parsedDate.makeUTC(tzDiff);
        item.datePublished = Poco::DateTimeFormatter::format(parsedDate, Poco::DateTimeFormat::ISO8601_FORMAT);

        // auto sourceNode = fetchNode(entryNode, "source");
        // if (sourceNode != nullptr)
        // {
        //     auto sourceEl = dynamic_cast<Poco::XML::Element*>(sourceNode);
        //     item.sourceURL = sourceEl->hasAttribute("url") ? sourceEl->getAttribute("url") : "";
        //     item.sourceTitle = sourceNode->innerText();
        // }

        items.emplace_back(item);
    }
    entryList->release();

    return items;
}