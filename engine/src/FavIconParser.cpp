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

#include "ZapFR/FavIconParser.h"
#include "ZapFR/Helpers.h"

ZapFR::Engine::FavIconParser::FavIconParser(const std::string& url, uint64_t associatedFeedID) : mURL(url), mFavIcon("")
{
    if (url.empty())
    {
        return;
    }

    Poco::Net::HTTPCredentials creds; // TODO
    auto html = Helpers::performHTTPRequest(Poco::URI(url), Poco::Net::HTTPRequest::HTTP_GET, creds, {}, associatedFeedID);

    // try to locate <link rel="icon" href="..."> with a sax parser
    FavIconSaxParser handler;
    FavIconSaxErrorHandler errorHandler;
    bool saxParsingSuccessful{true};
    try
    {
        Poco::XML::SAXParser parser;
        parser.setContentHandler(&handler);
        parser.setErrorHandler(&errorHandler);
        parser.parseString(html);
    }
    catch (...)
    {
        // todo: log std::cout << "parsing failed in whole html doc\n";
        saxParsingSuccessful = false;
    }

    mFavIcon = handler.favIconURL();
    if (!mFavIcon.empty() || saxParsingSuccessful)
    {
        // todo: log std::cout << "found favicon url : " << mFavIcon << "\n";
        return;
    }

    // try to extract all <link ...> tags from the original html, and parse that, to avoid xml errors elsewhere in the doc
    static auto linkRegex = Poco::RegularExpression("(<link.*?>)");
    Poco::RegularExpression::MatchVec matches;
    linkRegex.match(html, 0, matches);
    if (matches.size() > 0)
    {
        std::stringstream fakeXML;
        fakeXML << R"(<xml version="1.0">\n<links>)";
        for (const auto& match : matches)
        {
            fakeXML << html.substr(match.offset, match.length) << "\n";
        }
        fakeXML << "</links>\n";

        saxParsingSuccessful = true;
        try
        {
            Poco::XML::SAXParser parser;
            parser.setContentHandler(&handler);
            parser.setErrorHandler(&errorHandler);
            parser.parseString(fakeXML.str());
        }
        catch (...)
        {
            // todo: log std::cout << "parsing failed in fake html doc\n";
            saxParsingSuccessful = false;
        }

        mFavIcon = handler.favIconURL();
        if (!mFavIcon.empty() || saxParsingSuccessful)
        {
            // todo: log std::cout << "found favicon url : " << mFavIcon << "\n";
            return;
        }
    }

    // point to <site>/favicon.ico as a last resort
    auto uri = Poco::URI(url);
    uri.setPath("/favicon.ico");
    // todo: log std::cout << "fallback to " << uri.toString() << "\n";
    mFavIcon = uri.toString();
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
