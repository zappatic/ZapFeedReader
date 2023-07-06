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

#include "FeedParser.h"

ZapFR::Engine::FeedParser::FeedParser(Poco::XML::Document* xmlDoc, const std::string& url) : mXMLDoc(xmlDoc), mURL(url)
{
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

Poco::XML::Node* ZapFR::Engine::FeedParser::fetchNode(Poco::XML::Node* parent, const std::string& nodeName) const
{
    return parent->getNodeByPath(nodeName);
}