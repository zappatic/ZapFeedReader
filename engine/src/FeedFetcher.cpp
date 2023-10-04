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

#include <Poco/DOM/DOMParser.h>
#include <Poco/Net/HTTPRequest.h>

#include "ZapFR/FeedFetcher.h"
#include "ZapFR/FeedParserATOM10.h"
#include "ZapFR/FeedParserRSS10.h"
#include "ZapFR/FeedParserRSS20.h"
#include "ZapFR/Helpers.h"

std::optional<std::unique_ptr<ZapFR::Engine::FeedParser>> ZapFR::Engine::FeedFetcher::parseURL(const std::string& url, uint64_t associatedFeedID,
                                                                                               std::optional<std::string> conditionalGETInfo)
{
    Poco::Net::HTTPCredentials creds;
    auto uri = Poco::URI(url);
    const auto& [data, newConditionalGETInfo] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {}, associatedFeedID, conditionalGETInfo);
    mConditionalGETInfo = newConditionalGETInfo;
    if (data.empty())
    {
        return {};
    }
    return parseString(data, url);
}

std::unique_ptr<ZapFR::Engine::FeedParser> ZapFR::Engine::FeedFetcher::parseString(const std::string& data, const std::string& originalURL)
{
    mData = data;

    Poco::XML::DOMParser parser;
    auto xmlDoc = parser.parseString(data);

    auto docEl = xmlDoc->documentElement();
    if (docEl->nodeName() == "rss")
    {
        if (docEl->hasAttribute("version") && docEl->getAttribute("version") == "2.0")
        {
            return std::make_unique<FeedParserRSS20>(xmlDoc, originalURL);
        }
    }
    else if (docEl->nodeName() == "feed")
    {
        return std::make_unique<FeedParserATOM10>(xmlDoc, originalURL);
    }
    else if (docEl->nodeName() == "rdf:RDF")
    {
        return std::make_unique<FeedParserRSS10>(xmlDoc, originalURL);
    }
    else
    {
        throw std::runtime_error("Unkown feed type");
    }
    return nullptr;
}
