/*
    ZapFeedReader - RSS/Atom feed reader
    Copyright (C) 1023-present  Kasper Nauwelaerts (zapfr at zappatic dot net)

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

#include "ZapFR/FeedParserRSS10.h"

ZapFR::Engine::FeedParserRSS10::FeedParserRSS10(Poco::XML::Document* xmlDoc, const std::string& url) : FeedParser(xmlDoc, url)
{
}

std::string ZapFR::Engine::FeedParserRSS10::guid() const
{
    return "";
}

std::string ZapFR::Engine::FeedParserRSS10::title() const
{
    return fetchNodeValue("/channel/title");
}

std::string ZapFR::Engine::FeedParserRSS10::subtitle() const
{
    return "";
}

std::string ZapFR::Engine::FeedParserRSS10::link() const
{
    return fetchNodeValue("/channel/link");
}

std::string ZapFR::Engine::FeedParserRSS10::description() const
{
    return fetchNodeValue("/channel/description");
}

std::string ZapFR::Engine::FeedParserRSS10::language() const
{
    return "";
}

std::string ZapFR::Engine::FeedParserRSS10::copyright() const
{
    return "";
}

std::string ZapFR::Engine::FeedParserRSS10::iconURL() const
{
    auto imageNode = mXMLDoc->documentElement()->getNodeByPath("/image");
    if (imageNode != nullptr)
    {
        auto imageEl = dynamic_cast<Poco::XML::Element*>(imageNode);
        if (imageEl->hasAttributeNS("rdf", "about"))
        {
            return imageEl->getAttributeNS("rdf", "about");
        }
    }
    return "";
}

std::vector<ZapFR::Engine::FeedParser::Item> ZapFR::Engine::FeedParserRSS10::items() const
{
    std::vector<Item> items;

    auto itemList = mXMLDoc->getElementsByTagName("item");
    for (size_t i = 0; i < itemList->length(); ++i)
    {
        auto itemNode = itemList->item(i);

        Poco::XML::Element::NSMap nsMap;
        nsMap.declarePrefix("dc", "http://purl.org/dc/elements/1.1/");
        nsMap.declarePrefix("content", "http://purl.org/rss/1.0/modules/content/");

        Item item;
        item.title = fetchNodeValue(itemNode, "title");
        item.link = fetchNodeValue(itemNode, "link");
        item.description = fetchNodeValueInnerXML(itemNode, "description");

        // see if there's a content:encoded with more information present, if so, replace the body of the text with that
        auto contentEncodedNode = itemNode->getNodeByPathNS("content:encoded", nsMap);
        if (contentEncodedNode != nullptr)
        {
            item.description = contentEncodedNode->innerText();
        }

        // create a guid out of the link if present, or either title or description (all are optional, but either title or description must be present)
        auto guidSrc = item.link;
        if (guidSrc.empty())
        {
            guidSrc = item.title;
        }
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

        auto creatorNode = itemNode->getNodeByPathNS("dc:creator", nsMap);
        if (creatorNode != nullptr)
        {
            item.author = creatorNode->innerText();
        }

        auto dateNode = itemNode->getNodeByPathNS("dc:date", nsMap);
        if (dateNode != nullptr)
        {
            item.datePublished = dateNode->innerText();
            int tzDiff;
            auto parsedDate = Poco::DateTimeParser::parse(Poco::DateTimeFormat::ISO8601_FORMAT, item.datePublished, tzDiff);
            parsedDate.makeUTC(tzDiff);
            item.datePublished = Poco::DateTimeFormatter::format(parsedDate, Poco::DateTimeFormat::ISO8601_FORMAT);
        }
        items.emplace_back(item);
    }
    itemList->release();

    return items;
}
