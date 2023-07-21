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
#include "FeedParserAtom10.h"
#include "FeedParserRSS20.h"
#include "Helpers.h"

std::unique_ptr<ZapFR::Engine::FeedParser> ZapFR::Engine::FeedFetcher::parse(const std::string& url)
{
    auto xml = Helpers::performHTTPRequest(url, "GET");

    Poco::XML::DOMParser parser;
    auto xmlDoc = parser.parseString(xml);

    auto docEl = xmlDoc->documentElement();
    if (docEl->nodeName() == "rss")
    {
        if (docEl->hasAttribute("version") && docEl->getAttribute("version") == "2.0")
        {
            return std::make_unique<FeedParserRSS20>(xmlDoc, url);
        }
        // TODO: rss 1.0
    }
    else if (docEl->nodeName() == "feed")
    {
        return std::make_unique<FeedParserAtom10>(xmlDoc, url);
    }
    else
    {
        throw std::runtime_error("Unkown feed type");
    }
    return nullptr;
}
