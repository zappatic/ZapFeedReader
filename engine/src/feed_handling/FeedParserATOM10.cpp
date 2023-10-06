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

#define FMT_HEADER_ONLY
#include <fmt/core.h>

#include <Poco/DOM/NodeList.h>

#include "ZapFR/feed_handling/FeedParserATOM10.h"

ZapFR::Engine::FeedParserATOM10::FeedParserATOM10(const std::string& url) : FeedParserXML(url)
{
}

std::string ZapFR::Engine::FeedParserATOM10::guid() const
{
    return fetchNodeValue("/id");
}

std::string ZapFR::Engine::FeedParserATOM10::title() const
{
    return fetchNodeValue("/title");
}

std::string ZapFR::Engine::FeedParserATOM10::subtitle() const
{
    return fetchNodeValue("/subtitle");
}

std::string ZapFR::Engine::FeedParserATOM10::link() const
{
    std::string link;
    auto linkNodes = mXMLDoc->documentElement()->getElementsByTagName("link");
    for (size_t i = 0; i < linkNodes->length(); ++i)
    {
        auto linkNode = linkNodes->item(i);
        if (linkNode->nodeType() == Poco::XML::Node::ELEMENT_NODE)
        {
            auto linkEl = dynamic_cast<Poco::XML::Element*>(linkNode);
            if (linkEl->hasAttribute("rel") && linkEl->getAttribute("rel") == "alternate" && linkEl->hasAttribute("href"))
            {
                link = linkEl->getAttribute("href");
                break;
            }
        }
    }
    linkNodes->release();
    return link;
}

std::string ZapFR::Engine::FeedParserATOM10::description() const
{
    return "";
}

std::string ZapFR::Engine::FeedParserATOM10::iconURL() const
{
    return "";
}

std::string ZapFR::Engine::FeedParserATOM10::language() const
{
    auto docEl = mXMLDoc->documentElement();
    if (docEl->hasAttributeNS("xml", "lang"))
    {
        return docEl->getAttributeNS("xml", "lang");
    }
    return "";
}

std::string ZapFR::Engine::FeedParserATOM10::copyright() const
{
    return fetchNodeValue("/rights");
}

std::vector<ZapFR::Engine::FeedParser::Item> ZapFR::Engine::FeedParserATOM10::items() const
{
    std::vector<Item> items;

    auto entryList = mXMLDoc->getElementsByTagName("entry");
    for (size_t i = 0; i < entryList->length(); ++i)
    {
        auto entryNode = entryList->item(i);
        if (entryNode->nodeType() == Poco::XML::Node::ELEMENT_NODE)
        {
            auto entryEl = dynamic_cast<Poco::XML::Element*>(entryNode);

            Item item;
            item.title = fetchNodeValue(entryNode, "title");

            auto linkNodes = entryEl->getElementsByTagName("link");
            for (size_t j = 0; j < linkNodes->length(); ++j)
            {
                auto linkNode = linkNodes->item(j);
                if (linkNode->nodeType() == Poco::XML::Node::ELEMENT_NODE)
                {
                    auto linkEl = dynamic_cast<Poco::XML::Element*>(linkNode);
                    if (linkEl->hasAttribute("href"))
                    {
                        if (linkEl->hasAttribute("rel"))
                        {
                            auto rel = linkEl->getAttribute("rel");
                            if (rel == "enclosure")
                            {
                                Post::Enclosure e;
                                if (linkEl->hasAttribute("href"))
                                {
                                    e.url = linkEl->getAttribute("href");
                                }
                                if (linkEl->hasAttribute("type"))
                                {
                                    e.mimeType = linkEl->getAttribute("type");
                                }
                                if (linkEl->hasAttribute("length"))
                                {
                                    auto lengthStr = linkEl->getAttribute("length");
                                    Poco::NumberParser::tryParseUnsigned64(lengthStr, e.size);
                                }
                                if (!e.url.empty())
                                {
                                    item.enclosures.emplace_back(e);
                                }
                            }
                            else if (rel != "alternate")
                            {
                                continue;
                            }
                        }
                        item.link = linkEl->getAttribute("href");
                        break;
                    }
                }
            }
            linkNodes->release();

            // some feeds also put <enclosure> elements within items (diverges from spec, but allow anyway)
            // try url/href, length and type as attributes
            auto enclosureNodes = entryEl->getElementsByTagName("enclosure");
            for (size_t j = 0; j < enclosureNodes->length(); ++j)
            {
                auto enclosureNode = enclosureNodes->item(j);
                if (enclosureNode->nodeType() == Poco::XML::Node::ELEMENT_NODE)
                {
                    auto enclosureEl = dynamic_cast<Poco::XML::Element*>(enclosureNode);
                    Post::Enclosure e;
                    if (enclosureEl->hasAttribute("href"))
                    {
                        e.url = enclosureEl->getAttribute("href");
                    }
                    if (e.url.empty() && enclosureEl->hasAttribute("url"))
                    {
                        e.url = enclosureEl->getAttribute("url");
                    }
                    if (enclosureEl->hasAttribute("type"))
                    {
                        e.mimeType = enclosureEl->getAttribute("type");
                    }
                    if (enclosureEl->hasAttribute("length"))
                    {
                        auto lengthStr = enclosureEl->getAttribute("length");
                        Poco::NumberParser::tryParseUnsigned64(lengthStr, e.size);
                    }
                    if (!e.url.empty())
                    {
                        item.enclosures.emplace_back(e);
                    }
                }
            }
            enclosureNodes->release();

            item.content = fetchNodeValueInnerXML(entryNode, "summary");

            auto contentNode = fetchNode(entryNode, "content");
            if (contentNode != nullptr)
            {
                auto contentEl = dynamic_cast<Poco::XML::Element*>(contentNode);
                if (!contentEl->hasAttribute("src"))
                {
                    std::string contentType = "text";
                    if (contentEl->hasAttribute("type"))
                    {
                        contentType = contentEl->getAttribute("type");
                    }

                    if (contentType == "text")
                    {
                        item.content = fmt::format("<pre>{}</pre>", contentEl->innerText());
                    }
                    else
                    {
                        item.content = fetchNodeValueInnerXML(entryNode, "content");
                    }
                }
            }

            if (item.content.empty()) // see if there's a media:thumbnail/media:description present (for YouTube)
            {
                Poco::XML::Element::NSMap nsMap;
                nsMap.declarePrefix("media", "http://search.yahoo.com/mrss/");

                std::stringstream mediaContentStream;
                auto mediaGroupNode = entryNode->getNodeByPathNS("media:group", nsMap);
                if (mediaGroupNode != nullptr)
                {
                    auto thumbnailNode = mediaGroupNode->getNodeByPathNS("media:thumbnail", nsMap);
                    if (thumbnailNode != nullptr && thumbnailNode->nodeType() == Poco::XML::Node::ELEMENT_NODE)
                    {
                        auto thumbnailEl = dynamic_cast<Poco::XML::Element*>(thumbnailNode);
                        if (thumbnailEl->hasAttribute("url"))
                        {
                            item.thumbnail = thumbnailEl->getAttribute("url");
                            mediaContentStream << R"(<a href=")" << item.link << R"("><img src=")" << item.thumbnail << R"(" alt="" /></a>)";
                        }
                    }

                    auto descriptionNode = mediaGroupNode->getNodeByPathNS("media:description", nsMap);
                    if (descriptionNode != nullptr && descriptionNode->nodeType() == Poco::XML::Node::ELEMENT_NODE)
                    {
                        auto descriptionEl = dynamic_cast<Poco::XML::Element*>(descriptionNode);
                        if (descriptionEl->hasAttribute("type") && descriptionEl->getAttribute("type") == "html")
                        {
                            mediaContentStream << "<p>" << descriptionNode->innerText() << "</p>";
                        }
                        else
                        {
                            auto text = descriptionNode->innerText();
                            Poco::replaceInPlace(text, "\n", "<br />");
                            mediaContentStream << "<p>" << text << "</p>";
                        }
                    }

                    item.content = mediaContentStream.str();
                }
            }

            auto authorNode = fetchNode(entryNode, "author");
            if (authorNode != nullptr)
            {
                auto authorNameNode = fetchNode(authorNode, "name");
                if (authorNameNode != nullptr)
                {
                    item.author = authorNameNode->innerText();
                }
            }

            item.guid = fetchNodeValue(entryNode, "id");

            item.datePublished = fetchNodeValue(entryNode, "updated");
            int tzDiff;
            auto parsedDate = Poco::DateTimeParser::parse(Poco::DateTimeFormat::ISO8601_FORMAT, item.datePublished, tzDiff); // actually RFC 3339
            parsedDate.makeUTC(tzDiff);
            item.datePublished = Poco::DateTimeFormatter::format(parsedDate, Poco::DateTimeFormat::ISO8601_FORMAT);

            auto categoryNodes = entryEl->getElementsByTagName("category");
            for (size_t j = 0; j < categoryNodes->length(); ++j)
            {
                auto categoryNode = categoryNodes->item(j);
                if (categoryNode->nodeType() == Poco::XML::Node::ELEMENT_NODE)
                {
                    auto categoryEl = dynamic_cast<Poco::XML::Element*>(categoryNode);
                    if (categoryEl->hasAttribute("term"))
                    {
                        item.categories.emplace_back(categoryEl->getAttribute("term"));
                    }
                }
            }

            items.emplace_back(item);
        }
    }
    entryList->release();

    return items;
}
