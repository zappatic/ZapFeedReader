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

#include <Poco/DOM/DOMWriter.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/XML/XMLWriter.h>

#include "ZapFR/FeedParser.h"

ZapFR::Engine::FeedParser::FeedParser(Poco::XML::Document* xmlDoc, const std::string& url) : mXMLDoc(xmlDoc), mURL(url)
{
    mURI = Poco::URI(url);
}

std::string ZapFR::Engine::FeedParser::url() const noexcept
{
    return mURL;
}

std::string ZapFR::Engine::FeedParser::fetchNodeValue(const std::string& nodeName) const
{
    auto node = mXMLDoc->documentElement()->getNodeByPath(nodeName);
    if (node != nullptr)
    {
        return node->innerText();
    }
    return "";
}

std::string ZapFR::Engine::FeedParser::fetchNodeValue(Poco::XML::Node* parent, const std::string& nodeName) const
{
    auto node = parent->getNodeByPath(nodeName);
    if (node != nullptr)
    {
        return node->innerText();
    }
    return "";
}

std::string ZapFR::Engine::FeedParser::fetchNodeValueNS(Poco::XML::Node* parent, const std::string& nodeName, const Poco::XML::Node::NSMap& nsMap) const
{
    auto node = parent->getNodeByPathNS(nodeName, nsMap);
    if (node != nullptr)
    {
        return node->innerText();
    }
    return "";
}

std::string ZapFR::Engine::FeedParser::fetchNodeValueInnerXML(Poco::XML::Node* parent, const std::string& nodeName) const
{
    auto node = parent->getNodeByPath(nodeName);
    if (node != nullptr)
    {
        std::stringstream ss;
        auto writer = Poco::XML::DOMWriter();
        writer.setOptions(Poco::XML::XMLWriter::CANONICAL_XML);

        auto nodeChildren = node->childNodes();
        for (size_t i = 0; i < nodeChildren->length(); ++i)
        {
            auto child = nodeChildren->item(i);
            auto childType = child->nodeType();
            if (childType == Poco::XML::Node::ELEMENT_NODE)
            {
                writer.writeNode(ss, child);
            }
            else if (childType == Poco::XML::Node::CDATA_SECTION_NODE || childType == Poco::XML::Node::TEXT_NODE)
            {
                ss << child->innerText();
            }
        }
        nodeChildren->release();

        return ss.str();
    }
    return "";
}

Poco::XML::Node* ZapFR::Engine::FeedParser::fetchNode(Poco::XML::Node* parent, const std::string& nodeName) const
{
    return parent->getNodeByPath(nodeName);
}
