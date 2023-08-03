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

#include "Helpers.h"

namespace
{
    static Poco::Net::Context::Ptr gsSSLContext{nullptr};
    static std::mutex gsSSLContextMutex{};
} // namespace

std::string ZapFR::Engine::Helpers::joinString(const std::vector<std::string>& sourceVector, const char* delimiter)
{
    switch (sourceVector.size())
    {
        case 0:
            return "";
        case 1:
            return sourceVector.at(0);
        default:
            std::ostringstream os;
            std::copy(sourceVector.begin(), sourceVector.end() - 1, std::ostream_iterator<std::string>(os, delimiter));
            os << *sourceVector.rbegin();
            return os.str();
    }
}

std::string ZapFR::Engine::Helpers::joinIDNumbers(const std::vector<uint64_t>& sourceVector, const char* delimiter)
{
    switch (sourceVector.size())
    {
        case 0:
            return "";
        case 1:
            return std::to_string(sourceVector.at(0));
        default:
            std::ostringstream os;
            std::copy(sourceVector.begin(), sourceVector.end() - 1, std::ostream_iterator<uint64_t>(os, delimiter));
            os << *sourceVector.rbegin();
            return os.str();
    }
}

std::string ZapFR::Engine::Helpers::performHTTPRequest(const std::string& url, const std::string& method)
{
    {
        std::lock_guard<std::mutex> lock(gsSSLContextMutex);
        if (gsSSLContext == nullptr)
        {
            gsSSLContext = new Poco::Net::Context(Poco::Net::Context::TLS_CLIENT_USE, "", Poco::Net::Context::VERIFY_NONE);
        }
    }

    Poco::URI uri(url);

    std::unique_ptr<Poco::Net::HTTPClientSession> session;

    auto scheme = uri.getScheme();
    if (scheme == "https")
    {
        session = std::make_unique<Poco::Net::HTTPSClientSession>(uri.getHost(), uri.getPort(), gsSSLContext);
    }
    else if (scheme == "http")
    {
        session = std::make_unique<Poco::Net::HTTPClientSession>(uri.getHost(), uri.getPort());
    }
    else
    {
        throw std::runtime_error(fmt::format("Unknown scheme in URL: ", url));
    }

    auto path = uri.getPathAndQuery();
    if (path.empty())
    {
        path = "/";
    }

    session->setTimeout(Poco::Timespan(10, 0));
    Poco::Net::HTTPRequest request(method, path, Poco::Net::HTTPMessage::HTTP_1_1);
    request.setKeepAlive(false);

    session->sendRequest(request);

    Poco::Net::HTTPResponse response;
    std::istream& responseStream = session->receiveResponse(response);

    auto status = response.getStatus();

    if (status == 301)
    {
        auto newURL = response.get("Location");
        std::cout << "Moved permanently to " << newURL << "\n";
        // TODO: limit amount of redirects
        return performHTTPRequest(newURL, method);
    }
    else if (status == 302)
    {
        auto newURL = response.get("Location");
        std::cout << "Moved temporarily to " << newURL << "\n";
        // TODO: limit amount of redirects
        return performHTTPRequest(newURL, method);
    }

    if (status < 200 || status > 299)
    {
        throw std::runtime_error(fmt::format("HTTP status {} received for {} {}", static_cast<uint32_t>(response.getStatus()), method, url));
    }

    std::string resultStr;
    Poco::StreamCopier::copyToString(responseStream, resultStr);

    return resultStr;
}
