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
#include "FeedParserAtom10.h"
#include "FeedParserRSS20.h"

ZapFR::Engine::FeedFetcher::FeedFetcher(Database* db) : mDatabase(db)
{
    mSSLContext = new Poco::Net::Context(Poco::Net::Context::TLS_CLIENT_USE, "", Poco::Net::Context::VERIFY_NONE);
}

void ZapFR::Engine::FeedFetcher::subscribeToFeed(const std::string& url)
{
    auto xml = performHTTPRequest(url, "GET");

    Poco::XML::DOMParser parser;
    auto xmlDoc = parser.parseString(xml);

    auto docEl = xmlDoc->documentElement();
    if (docEl->nodeName() == "rss")
    {
        if (docEl->hasAttribute("version") && docEl->getAttribute("version") == "2.0")
        {
            auto feed = FeedParserRSS20(xmlDoc, url);
            mDatabase->subscribeToFeed(feed);
        }
    }
    else if (docEl->nodeName() == "feed")
    {
        auto feed = FeedParserAtom10(xmlDoc, url);
        mDatabase->subscribeToFeed(feed);
    }
    else
    {
        throw std::runtime_error("Unkown feed type");
    }
}

std::string ZapFR::Engine::FeedFetcher::performHTTPRequest(const std::string& url, const std::string& method)
{
    Poco::URI uri(url);

    auto session = std::make_unique<Poco::Net::HTTPSClientSession>(uri.getHost(), uri.getPort(), mSSLContext);
    session->setTimeout(Poco::Timespan(10, 0));
    Poco::Net::HTTPRequest request(method, uri.getPathAndQuery(), Poco::Net::HTTPMessage::HTTP_1_1);
    request.setKeepAlive(false);

    session->sendRequest(request);

    Poco::Net::HTTPResponse response;
    std::istream& responseStream = session->receiveResponse(response);

    auto status = response.getStatus();

    if (status == 301)
    {
        auto newURL = response.get("Location");
        std::cout << "Redirect to " << newURL << "\n";
        // TODO: limit amount of redirects
        return performHTTPRequest(newURL, method);
    }

    if (status < 200 || status > 299)
    {
        throw std::runtime_error(fmt::format("Fetching feed, status {} received", static_cast<uint32_t>(response.getStatus())));
    }

    std::string resultStr;
    Poco::StreamCopier::copyToString(responseStream, resultStr);

    return resultStr;
}
