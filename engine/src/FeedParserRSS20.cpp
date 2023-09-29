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

#include <Poco/DOM/NodeList.h>
#include <Poco/DigestStream.h>
#include <Poco/MD5Engine.h>
#include <Poco/UUIDGenerator.h>

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
        item.author = fetchNodeValue(itemNode, "author");
        item.content = fetchNodeValueInnerXML(itemNode, "description");

        Poco::XML::Element::NSMap nsMap;
        nsMap.declarePrefix("content", "http://purl.org/rss/1.0/modules/content/");
        nsMap.declarePrefix("torrent", "http://xmlns.ezrss.it/0.1/");

        // see if there's a content:encoded with more information present, if so, replace the body of the text with that
        auto contentEncodedNode = itemNode->getNodeByPathNS("content:encoded", nsMap);
        if (contentEncodedNode != nullptr)
        {
            item.content = contentEncodedNode->innerText();
        }

        auto enclosureNodes = dynamic_cast<Poco::XML::Element*>(itemNode)->getElementsByTagName("enclosure");
        for (size_t j = 0; j < enclosureNodes->length(); ++j)
        {
            auto enclosureNode = enclosureNodes->item(j);
            auto enclosureEl = dynamic_cast<Poco::XML::Element*>(enclosureNode);
            Post::Enclosure e;
            e.url = enclosureEl->hasAttribute("url") ? enclosureEl->getAttribute("url") : "";
            e.mimeType = enclosureEl->hasAttribute("type") ? enclosureEl->getAttribute("type") : "";
            if (enclosureEl->hasAttribute("length"))
            {
                auto sizeStr = enclosureEl->getAttribute("length");
                Poco::NumberParser::tryParseUnsigned64(sizeStr, e.size);
            }
            item.enclosures.emplace_back(e);
        }
        enclosureNodes->release();

        // see if there's a torrent:magnetURI and torrent:contentLength present, if so, add that as an enclosure
        auto magnetURI = fetchNodeValueNS(itemNode, "torrent:magnetURI", nsMap);
        if (!magnetURI.empty())
        {
            Post::Enclosure e;
            e.url = magnetURI;
            e.mimeType = "application/x-bittorrent";
            auto contentLength = fetchNodeValueNS(itemNode, "torrent:contentLength", nsMap);
            Poco::NumberParser::tryParseUnsigned64(contentLength, e.size);
            item.enclosures.emplace_back(e);
        }

        item.commentsURL = fetchNodeValue(itemNode, "comments");

        auto guidNode = fetchNode(itemNode, "guid");
        if (guidNode != nullptr)
        {
            item.guid = guidNode->innerText();
        }
        else
        {
            // create a guid out of the link if present, or either title or description (all are optional, but either title or description must be present)
            auto guidSrc = item.link;
            if (guidSrc.empty())
            {
                guidSrc = item.title;
            }
            if (guidSrc.empty())
            {
                guidSrc = item.content;
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

        if (item.link.empty() && item.guid.starts_with("http"))
        {
            item.link = item.guid;
        }

        item.datePublished = fetchNodeValue(itemNode, "pubDate");
        int tzDiff;

        Poco::DateTime parsedDate;
        auto dateParseSuccess = Poco::DateTimeParser::tryParse(Poco::DateTimeFormat::RFC1123_FORMAT, item.datePublished, parsedDate, tzDiff);
        if (!dateParseSuccess)
        {
            dateParseSuccess = Poco::DateTimeParser::tryParse(Poco::DateTimeFormat::RFC822_FORMAT, item.datePublished, parsedDate, tzDiff);
        }

        if (dateParseSuccess)
        {
            parsedDate.makeUTC(tzDiff);
            item.datePublished = Poco::DateTimeFormatter::format(parsedDate, Poco::DateTimeFormat::ISO8601_FORMAT);
        }
        else
        {
            item.datePublished = "";
        }

        items.emplace_back(item);
    }
    itemList->release();

    return items;
}
