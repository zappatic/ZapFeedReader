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
#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPRequest.h>

#include "ZapFR/Helpers.h"
#include "ZapFR/feed_handling/FeedFetcher.h"
#include "ZapFR/feed_handling/FeedParserATOM10.h"
#include "ZapFR/feed_handling/FeedParserJSON11.h"
#include "ZapFR/feed_handling/FeedParserRSS10.h"
#include "ZapFR/feed_handling/FeedParserRSS20.h"

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

    if (data.empty())
    {
        return nullptr;
    }
    if (data.at(0) == '<')
    {
        Poco::XML::DOMParser parser;
        Poco::AutoPtr<Poco::XML::Document> xmlDoc = parser.parseString(data);

        auto docEl = xmlDoc->documentElement();
        if (docEl->nodeName() == "rss")
        {
            if (docEl->hasAttribute("version") && docEl->getAttribute("version") == "2.0")
            {
                auto feed = std::make_unique<FeedParserRSS20>(originalURL);
                feed->setXMLDoc(xmlDoc);
                return feed;
            }
        }
        else if (docEl->nodeName() == "feed")
        {
            auto feed = std::make_unique<FeedParserATOM10>(originalURL);
            feed->setXMLDoc(xmlDoc);
            return feed;
        }
        else if (docEl->nodeName() == "rdf:RDF")
        {
            auto feed = std::make_unique<FeedParserRSS10>(originalURL);
            feed->setXMLDoc(xmlDoc);
            return feed;
        }
        else
        {
            throw std::runtime_error("Unkown feed type");
        }
    }
    else if (data.at(0) == '{')
    {
        Poco::JSON::Parser parser;
        auto root = parser.parse(data);
        auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
        if (!rootObj.isNull())
        {
            auto version = rootObj->getValue<std::string>("version");
            // both v1 and v1.1 can be parsed by the 1.1 parser, as it checks for both 'authors' and 'author' entries
            if (Poco::icompare(version, "https://jsonfeed.org/version/1.1") == 0 || Poco::icompare(version, "https://jsonfeed.org/version/1") == 0)
            {
                auto feed = std::make_unique<FeedParserJSON11>(originalURL);
                feed->setRootObj(rootObj);
                return feed;
            }
        }
    }

    return nullptr;
}
