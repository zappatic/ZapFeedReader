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

#include "FeedFetcher.h"
#include "FeedLocal.h"
#include "FeedParserATOM10.h"
#include "FeedParserRSS20.h"
#include "Helpers.h"

std::unique_ptr<ZapFR::Engine::FeedParser> ZapFR::Engine::FeedFetcher::parseURL(const std::string& url, uint64_t associatedFeedID)
{
    auto xml = Helpers::performHTTPRequest(url, "GET", associatedFeedID);
    return parseString(xml, url);
}

std::unique_ptr<ZapFR::Engine::FeedParser> ZapFR::Engine::FeedFetcher::parseString(const std::string& xml, const std::string& originalURL)
{
    mXML = xml;

    Poco::XML::DOMParser parser;
    auto xmlDoc = parser.parseString(xml);

    auto docEl = xmlDoc->documentElement();
    if (docEl->nodeName() == "rss")
    {
        if (docEl->hasAttribute("version") && docEl->getAttribute("version") == "2.0")
        {
            return std::make_unique<FeedParserRSS20>(xmlDoc, originalURL);
        }
        // TODO: rss 1.0
    }
    else if (docEl->nodeName() == "feed")
    {
        return std::make_unique<FeedParserATOM10>(xmlDoc, originalURL);
    }
    else
    {
        throw std::runtime_error("Unkown feed type");
    }
    return nullptr;
}

std::string ZapFR::Engine::FeedFetcher::xml() const noexcept
{
    return mXML;
}
