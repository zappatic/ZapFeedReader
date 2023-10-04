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

#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/RegularExpression.h>
#include <Poco/SAX/Attributes.h>
#include <Poco/SAX/SAXParser.h>

#include "ZapFR/Helpers.h"
#include "ZapFR/feed_handling/FeedDiscovery.h"

ZapFR::Engine::FeedDiscovery::FeedDiscovery(const std::string& url) : mURL(url)
{
    mDiscoveredFeeds.clear();
    if (url.empty())
    {
        return;
    }

    Poco::Net::HTTPCredentials creds; // TODO
    Poco::URI uri;
    if (!url.starts_with("http"))
    {
        uri = Poco::URI("https://" + url);
    }
    else
    {
        uri = Poco::URI(url);
    }

    std::string html;
    try
    {
        std::string cgi;
        std::tie(html, cgi) = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {});
    }
    catch (...)
    {
        return;
    }

    if (interpretAsYoutubeSource(uri, html))
    {
        return;
    }

    if (interpretAsDirectFeedLink(uri, html))
    {
        return;
    }

    if (interpretAsHTMLWithRelAlternateLinks(uri, html))
    {
        postProcessFeeds();
        return;
    }
}

bool ZapFR::Engine::FeedDiscovery::interpretAsYoutubeSource(const Poco::URI& uri, const std::string& html)
{
    if (Poco::endsWith(uri.getHost(), std::string("youtube.com")))
    {
        static Poco::RegularExpression titleRegex(R"(<title>(.*?)</title>)");
        static Poco::RegularExpression canonicalURLRegex(R"#(<link rel="canonical" href="(.*?)")#");
        static Poco::RegularExpression channelURLRegex(R"(^https://.*?youtube.com/channel/(.*?)$)");
        Poco::RegularExpression::MatchVec linkMatches;
        if (canonicalURLRegex.match(html, 0, linkMatches) > 0)
        {
            auto canonicalURL = html.substr(linkMatches.at(1).offset, linkMatches.at(1).length);
            Poco::RegularExpression::MatchVec urlMatches;
            if (channelURLRegex.match(canonicalURL, 0, urlMatches) > 0)
            {
                auto channelID = canonicalURL.substr(urlMatches.at(1).offset, urlMatches.at(1).length);

                std::string channelTitle = channelID;
                Poco::RegularExpression::MatchVec titleMatches;
                if (titleRegex.match(html, 0, titleMatches) > 0)
                {
                    channelTitle = html.substr(titleMatches.at(1).offset, titleMatches.at(1).length);
                }

                mDiscoveredFeeds.emplace_back(channelTitle, fmt::format("https://www.youtube.com/feeds/videos.xml?channel_id={}", channelID), DiscoveredFeed::Type::Atom);
                return true;
            }
        }
    }
    return false;
}

bool ZapFR::Engine::FeedDiscovery::interpretAsDirectFeedLink([[maybe_unused]] const Poco::URI& uri, const std::string& data)
{
    if (data.at(0) == '<')
    {
        DocumentElementExtractorSaxParser handler;
        FeedDiscoverySaxErrorHandler errorHandler;
        try
        {
            Poco::XML::SAXParser parser;
            parser.setContentHandler(&handler);
            parser.setErrorHandler(&errorHandler);
            parser.parseString(data);

            const auto& documentElementTitle = handler.documentElementTitle();
            if (Poco::icompare(documentElementTitle, "rss") == 0 || Poco::icompare(documentElementTitle, "rdf") == 0)
            {
                mDiscoveredFeeds.emplace_back("RSS Feed", uri.toString(), DiscoveredFeed::Type::RSS);
                return true;
            }
            else if (Poco::icompare(documentElementTitle, "feed") == 0)
            {
                mDiscoveredFeeds.emplace_back("Atom Feed", uri.toString(), DiscoveredFeed::Type::Atom);
                return true;
            }
        }
        catch (...)
        {
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
            if (Poco::icompare(version, "https://jsonfeed.org/version/1.1") == 0 || Poco::icompare(version, "https://jsonfeed.org/version/1") == 0)
            {
                std::string feedTitle = "JSON feed";
                if (rootObj->has("title"))
                {
                    feedTitle = rootObj->getValue<std::string>("title");
                }
                mDiscoveredFeeds.emplace_back(feedTitle, uri.toString(), DiscoveredFeed::Type::JSON);
                return true;
            }
        }
    }
    return false;
}

bool ZapFR::Engine::FeedDiscovery::interpretAsHTMLWithRelAlternateLinks([[maybe_unused]] const Poco::URI& uri, const std::string& html)
{
    // try to locate <link rel="alternate" type="<rss mimetype>" href="..."> with a sax parser
    HTMLRelAlternateFeedExtractorSaxParser handler;
    FeedDiscoverySaxErrorHandler errorHandler;
    handler.setOutputVector(&mDiscoveredFeeds);
    bool saxParsingSuccessful{false};
    try
    {
        Poco::XML::SAXParser parser;
        parser.setContentHandler(&handler);
        parser.setErrorHandler(&errorHandler);
        parser.parseString(html);
        saxParsingSuccessful = true;
    }
    catch (...)
    {
    }

    if (!mDiscoveredFeeds.empty() || saxParsingSuccessful)
    {
        return true;
    }

    // try to extract all <link ...> tags from the original html, and parse that, to avoid xml errors elsewhere in the doc
    static auto linkRegex = Poco::RegularExpression("(<link.*?>)");
    Poco::RegularExpression::MatchVec matches;
    size_t offset{0};
    std::stringstream fakeXML;
    fakeXML << R"(<?xml version="1.0" encoding="UTF-8"?><links>)";
    while (linkRegex.match(html, offset, matches) > 0)
    {
        fakeXML << html.substr(matches.at(1).offset, matches.at(1).length) << "\n";
        offset = matches.at(0).offset + 1;
    }
    fakeXML << "</links>\n";

    saxParsingSuccessful = false;
    try
    {
        Poco::XML::SAXParser parser;
        parser.setContentHandler(&handler);
        parser.setErrorHandler(&errorHandler);
        parser.parseString(fakeXML.str());
        saxParsingSuccessful = true;
    }
    catch (...)
    {
    }

    return saxParsingSuccessful;
}

void ZapFR::Engine::FeedDiscovery::postProcessFeeds()
{
    // convert relative urls to absolute urls
    for (auto& discoveredFeed : mDiscoveredFeeds)
    {
        if (!discoveredFeed.url.starts_with("http"))
        {
            auto uri = Poco::URI(mURL);
            uri.setPathEtc(discoveredFeed.url);
            discoveredFeed.url = uri.toString();
        }
    }
}

void ZapFR::Engine::HTMLRelAlternateFeedExtractorSaxParser::startElement(const Poco::XML::XMLString& /*uri*/, const Poco::XML::XMLString& localName,
                                                                         const Poco::XML::XMLString& /*qname*/, const Poco::XML::Attributes& attrList)
{
    if (localName == "link")
    {
        auto relIndex = attrList.getIndex("", "rel");
        auto hrefIndex = attrList.getIndex("", "href");
        auto typeIndex = attrList.getIndex("", "type");
        auto titleIndex = attrList.getIndex("", "title");
        if (relIndex > -1 && hrefIndex > -1 && typeIndex > -1)
        {
            auto relValue = attrList.getValue(relIndex);
            if (Poco::icompare(relValue, "alternate") == 0)
            {
                DiscoveredFeed::Type t;
                auto typeValue = attrList.getValue(typeIndex);
                if (Poco::icompare(typeValue, "application/rss+xml") == 0)
                {
                    t = DiscoveredFeed::Type::RSS;
                }
                else if (Poco::icompare(typeValue, "application/atom+xml") == 0)
                {
                    t = DiscoveredFeed::Type::Atom;
                }
                else if (Poco::icompare(typeValue, "application/json") == 0)
                {
                    t = DiscoveredFeed::Type::JSON;
                }
                else if (Poco::icompare(typeValue, "application/feed+json") == 0)
                {
                    t = DiscoveredFeed::Type::JSON;
                }
                else
                {
                    return;
                }

                std::string title;
                if (titleIndex > -1)
                {
                    title = attrList.getValue(titleIndex);
                }

                mOutputVector->emplace_back(title, attrList.getValue(hrefIndex), t);
            }
        }
    }
}

void ZapFR::Engine::DocumentElementExtractorSaxParser::startElement(const Poco::XML::XMLString& /*uri*/, const Poco::XML::XMLString& localName,
                                                                    const Poco::XML::XMLString& /*qname*/, const Poco::XML::Attributes& /*attrList*/)
{
    if (!mFirstElementParsed)
    {
        mDocumentElementTitle = localName;
        mFirstElementParsed = true;
    }
}
