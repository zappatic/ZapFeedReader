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

ZapFR::Engine::FeedDiscovery::FeedDiscovery(const std::string& url)
{
    if (url.empty())
    {
        return;
    }

    Poco::Net::HTTPCredentials creds; // TODO
    if (!url.starts_with("http"))
    {
        mURI = Poco::URI("https://" + url);
    }
    else
    {
        mURI = Poco::URI(url);
    }

    try
    {
        std::string cgi;
        std::tie(mData, cgi) = Helpers::performHTTPRequest(mURI, Poco::Net::HTTPRequest::HTTP_GET, creds, {});
    }
    catch (...)
    {
        return;
    }
}

ZapFR::Engine::FeedDiscovery::FeedDiscovery(const std::string& url, const std::string& data) : mURI(Poco::URI(url)), mData(data)
{
}

void ZapFR::Engine::FeedDiscovery::discover()
{
    mDiscoveredFeeds.clear();
    if (mData.empty())
    {
        return;
    }

    if (interpretAsYoutubeSource())
    {
        return;
    }

    if (interpretAsDirectFeedLink())
    {
        return;
    }

    if (interpretAsHTMLWithRelAlternateLinks())
    {
        postProcessFeeds();
        return;
    }
}

bool ZapFR::Engine::FeedDiscovery::interpretAsYoutubeSource()
{
    if (Poco::endsWith(mURI.getHost(), std::string("youtube.com")))
    {
        static Poco::RegularExpression titleRegex(R"(<title>(.*?)</title>)");
        static Poco::RegularExpression canonicalURLRegex(R"#(<link rel="canonical" href="(.*?)")#");
        static Poco::RegularExpression channelURLRegex(R"(^https://.*?youtube.com/channel/(.*?)$)");
        Poco::RegularExpression::MatchVec linkMatches;
        if (canonicalURLRegex.match(mData, 0, linkMatches) > 0)
        {
            auto canonicalURL = mData.substr(linkMatches.at(1).offset, linkMatches.at(1).length);
            Poco::RegularExpression::MatchVec urlMatches;
            if (channelURLRegex.match(canonicalURL, 0, urlMatches) > 0)
            {
                auto channelID = canonicalURL.substr(urlMatches.at(1).offset, urlMatches.at(1).length);

                std::string channelTitle = channelID;
                Poco::RegularExpression::MatchVec titleMatches;
                if (titleRegex.match(mData, 0, titleMatches) > 0)
                {
                    channelTitle = mData.substr(titleMatches.at(1).offset, titleMatches.at(1).length);
                }

                mDiscoveredFeeds.emplace_back(channelTitle, fmt::format("https://www.youtube.com/feeds/videos.xml?channel_id={}", channelID), Feed::Type::Atom);
                return true;
            }
        }
    }
    return false;
}

bool ZapFR::Engine::FeedDiscovery::interpretAsDirectFeedLink()
{
    if (mData.at(0) == '<')
    {
        DocumentElementExtractorSaxParser handler;
        FeedDiscoverySaxErrorHandler errorHandler;
        try
        {
            Poco::XML::SAXParser parser;
            parser.setContentHandler(&handler);
            parser.setErrorHandler(&errorHandler);
            parser.parseString(mData);

            const auto& documentElementTitle = handler.documentElementTitle();
            if (Poco::icompare(documentElementTitle, "rss") == 0 || Poco::icompare(documentElementTitle, "rdf") == 0)
            {
                mDiscoveredFeeds.emplace_back("RSS Feed", mURI.toString(), Feed::Type::RSS);
                return true;
            }
            else if (Poco::icompare(documentElementTitle, "feed") == 0)
            {
                mDiscoveredFeeds.emplace_back("Atom Feed", mURI.toString(), Feed::Type::Atom);
                return true;
            }
        }
        catch (...)
        {
        }
    }
    else if (mData.at(0) == '{')
    {
        Poco::JSON::Parser parser;
        auto root = parser.parse(mData);
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
                mDiscoveredFeeds.emplace_back(feedTitle, mURI.toString(), Feed::Type::JSON);
                return true;
            }
        }
    }
    return false;
}

bool ZapFR::Engine::FeedDiscovery::interpretAsHTMLWithRelAlternateLinks()
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
        parser.parseString(mData);
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
    // the link tags might be unclosed, the attrib values might not have quotes around it, so try to fix that first
    static auto linkRegex = Poco::RegularExpression("(<link.*?>)");
    Poco::RegularExpression::MatchVec matches;
    size_t offset{0};
    std::stringstream fakeXML;
    fakeXML << R"(<?xml version="1.0" encoding="UTF-8"?><links>)";
    while (linkRegex.match(mData, offset, matches) > 0)
    {
        auto link = mData.substr(matches.at(1).offset, matches.at(1).length);

        if (!link.ends_with("/>"))
        {
            static Poco::RegularExpression trailingSlashRegex("(>)$");
            trailingSlashRegex.subst(link, " />");
        }

        if (!link.ends_with(" />"))
        {
            static Poco::RegularExpression trailingSlashAndSpaceRegex(R"((\/>)$)");
            trailingSlashAndSpaceRegex.subst(link, " />");
        }

        if (link.find(R"(")") == std::string::npos)
        {
            static Poco::RegularExpression attribRegex(R"(\s?(.*?)=(.*?)\s)");
            attribRegex.subst(link, R"( $1="$2" )", Poco::RegularExpression::RE_GLOBAL);
        }

        fakeXML << link << "\n";
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
            auto uri = mURI;
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
                Feed::Type t;
                auto typeValue = attrList.getValue(typeIndex);
                if (Poco::icompare(typeValue, "application/rss+xml") == 0)
                {
                    t = Feed::Type::RSS;
                }
                else if (Poco::icompare(typeValue, "application/atom+xml") == 0)
                {
                    t = Feed::Type::Atom;
                }
                else if (Poco::icompare(typeValue, "application/json") == 0)
                {
                    t = Feed::Type::JSON;
                }
                else if (Poco::icompare(typeValue, "application/feed+json") == 0)
                {
                    t = Feed::Type::JSON;
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
