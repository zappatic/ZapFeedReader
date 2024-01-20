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

#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/RegularExpression.h>
#include <Poco/SAX/Attributes.h>
#include <Poco/SAX/SAXParser.h>
#include <Poco/URI.h>

#include "ZapFR/Helpers.h"
#include "ZapFR/feed_handling/FavIconParser.h"

ZapFR::Engine::FavIconParser::FavIconParser(const std::string& url, uint64_t associatedFeedID) : mURL(url), mFavIcon("")
{
    if (url.empty())
    {
        return;
    }

    Poco::Net::HTTPCredentials creds; // TODO
    auto uri = Poco::URI(url);
    const auto& [html, cgi] = Helpers::performHTTPRequest(uri, Poco::Net::HTTPRequest::HTTP_GET, creds, {}, associatedFeedID);

    // exception for YouTube: extract the channel image from the ytInitialData variable
    if (Poco::endsWith(uri.getHost(), std::string("youtube.com")))
    {
        static Poco::RegularExpression ytInitialDataRegex("var ytInitialData = ({.*?});");
        Poco::RegularExpression::MatchVec matches;
        ytInitialDataRegex.match(html, 0, matches);
        if (matches.size() == 2)
        {
            try
            {
                auto match = matches.at(1);

                Poco::JSON::Parser parser;
                auto root = parser.parse(html.substr(match.offset, match.length));
                auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
                if (rootObj->has("metadata") &&
                    rootObj->isObject("metadata")) // converting to Poco::DynamicStruct has syntax advantages but performance disadvantages (needs to copy)
                {
                    auto metadataObj = rootObj->getObject("metadata");
                    if (metadataObj->has("channelMetadataRenderer") && metadataObj->isObject("channelMetadataRenderer"))
                    {
                        auto cmdrObj = metadataObj->getObject("channelMetadataRenderer");
                        if (cmdrObj->has("avatar") && cmdrObj->isObject("avatar"))
                        {
                            auto avatarObj = cmdrObj->getObject("avatar");
                            if (avatarObj->has("thumbnails") && avatarObj->isArray("thumbnails"))
                            {
                                auto thumbnailsArr = avatarObj->getArray("thumbnails");
                                if (thumbnailsArr->size() > 0 && thumbnailsArr->isObject(0))
                                {
                                    auto thumbnail = thumbnailsArr->getObject(0);
                                    if (thumbnail->has("url"))
                                    {
                                        mFavIcon = thumbnail->getValue<std::string>("url");
                                        return;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch (...)
            {
                // failed to parse the channel icon out of the json dump; default to YouTube favicon
            }
        }
    }

    // try to locate <link rel="icon" href="..."> with a sax parser
    FavIconSaxParser handler;
    FavIconSaxErrorHandler errorHandler;
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

    mFavIcon = handler.favIconURL();
    if (!mFavIcon.empty() || saxParsingSuccessful)
    {
        return;
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

    mFavIcon = handler.favIconURL();
    if (!mFavIcon.empty() || saxParsingSuccessful)
    {
        return;
    }

    // point to <site>/favicon.ico as a last resort
    auto lastResortURI = Poco::URI(url);
    lastResortURI.setPath("/favicon.ico");
    mFavIcon = lastResortURI.toString();
}

std::string ZapFR::Engine::FavIconParser::favIcon() const noexcept
{
    // make sure the link is absolute
    if (!mFavIcon.starts_with("http"))
    {
        auto uri = Poco::URI(mURL);
        uri.setPath(mFavIcon);
        return uri.toString();
    }
    return mFavIcon;
}

void ZapFR::Engine::FavIconSaxParser::startElement(const Poco::XML::XMLString& /*uri*/, const Poco::XML::XMLString& localName, const Poco::XML::XMLString& /*qname*/,
                                                   const Poco::XML::Attributes& attrList)
{
    if (localName == "link")
    {
        auto relIndex = attrList.getIndex("", "rel");
        auto hrefIndex = attrList.getIndex("", "href");
        if (relIndex > -1 && hrefIndex > -1)
        {
            auto relValue = attrList.getValue(relIndex);
            if (relValue.find("icon") != std::string::npos)
            {
                mFavIconURL = attrList.getValue(hrefIndex);
            }
        }
    }
}
