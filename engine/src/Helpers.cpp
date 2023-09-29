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

#include <Poco/Base64Decoder.h>
#include <Poco/Base64Encoder.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/URI.h>

#include "ZapFR/Global.h"
#include "ZapFR/Helpers.h"
#include "ZapFR/Log.h"

namespace
{
    static Poco::Net::Context::Ptr gsSSLContext{nullptr};
    static std::mutex gsSSLContextMutex{};
} // namespace

void ZapFR::Engine::Helpers::splitString(const std::string& sourceString, char delimiter, std::vector<std::string>& outSubstrings)
{
    std::stringstream ss(sourceString);
    std::string item;
    while (std::getline(ss, item, delimiter))
    {
        if (!item.empty())
        {
            outSubstrings.emplace_back(item);
        }
    }
}

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

std::string ZapFR::Engine::Helpers::performHTTPRequest(Poco::URI& url, const std::string& method, Poco::Net::HTTPCredentials& credentials,
                                                       const std::map<std::string, std::string>& parameters, std::optional<uint64_t> associatedFeedID)
{
    {
        std::lock_guard<std::mutex> lock(gsSSLContextMutex);
        if (gsSSLContext.isNull())
        {
#if POCO_VERSION < 0x010A0000
            gsSSLContext = new Poco::Net::Context(Poco::Net::Context::TLSV1_2_CLIENT_USE, "", Poco::Net::Context::VERIFY_NONE);
#else
            gsSSLContext = new Poco::Net::Context(Poco::Net::Context::TLS_CLIENT_USE, "", Poco::Net::Context::VERIFY_NONE);
            gsSSLContext->requireMinimumProtocol(Poco::Net::Context::PROTO_TLSV1_2);
#endif
        }
    }

    // stupid exception for YouTube's consent form bullshit
    auto host = url.getHost();
    if (Poco::endsWith(host, std::string("youtube.com")) || Poco::endsWith(host, std::string("youtu.be")) || Poco::endsWith(host, std::string("ytimg.com")))
    {
        auto consentBullshitFound{false};
        auto q = url.getQueryParameters();
        for (const auto& [k, v] : q)
        {
            if (k == "ucbcb")
            {
                consentBullshitFound = true;
                break;
            }
        }
        if (!consentBullshitFound)
        {
            q.emplace_back("ucbcb", "1");
            url.setQueryParameters(q);
        }
    }

    // lambda to convert relative url to absolute url, in case a 301/302 redirect is received
    const auto ensureRedirectLocationIsAbsolute = [](const Poco::URI& originalURL, const std::string& newLocation) -> Poco::URI
    {
        Poco::URI newURI(originalURL);
        if (!newLocation.starts_with("http"))
        {
            newURI.setPathEtc(newLocation);
        }
        else
        {
            newURI = Poco::URI(newLocation);
        }
        return newURI;
    };

    std::unique_ptr<Poco::Net::HTTPClientSession> session;

    auto scheme = url.getScheme();
    if (scheme == "https")
    {
        session = std::make_unique<Poco::Net::HTTPSClientSession>(url.getHost(), url.getPort(), gsSSLContext);
    }
    else if (scheme == "http")
    {
        session = std::make_unique<Poco::Net::HTTPClientSession>(url.getHost(), url.getPort());
    }
    else
    {
        throw std::runtime_error(fmt::format("Unknown scheme in URL: {}", url.toString()));
    }

    if (method == Poco::Net::HTTPRequest::HTTP_GET)
    {
        for (const auto& [k, v] : parameters)
        {
            url.addQueryParameter(k, v);
        }
    }

    auto path = url.getPathAndQuery();
    if (path.empty())
    {
        path = "/";
    }

    session->setTimeout(Poco::Timespan(10, 0));
    Poco::Net::HTTPRequest request(method, path, Poco::Net::HTTPMessage::HTTP_1_1);
    request.setKeepAlive(false);

    static const auto userAgent = fmt::format("ZapFeedReader/{}", ZapFR::Engine::APIVersion);
    request.set("User-Agent", userAgent);
    if (!credentials.empty())
    {
        std::stringstream b64Stream;
        Poco::Base64Encoder encoder(b64Stream);
        encoder << credentials.getUsername() << ":" << credentials.getPassword();
        encoder.close();
        request.set("Authorization", fmt::format("Basic {}", b64Stream.str()));
    }

    if (method == Poco::Net::HTTPRequest::HTTP_POST || method == Poco::Net::HTTPRequest::HTTP_PATCH)
    {
        Poco::Net::HTMLForm form;
        for (const auto& [k, v] : parameters)
        {
            form.add(k, v);
        }
        form.prepareSubmit(request);
        form.write(session->sendRequest(request));
    }
    else
    {
        session->sendRequest(request);
    }

    Poco::Net::HTTPResponse response;
    std::istream& responseStream = session->receiveResponse(response);

    auto status = response.getStatus();
    std::string resultStr;
    Poco::StreamCopier::copyToString(responseStream, resultStr);

    if (status == 301)
    {
        auto newURL = ensureRedirectLocationIsAbsolute(url, response.get("Location"));
        Log::log(LogLevel::Info, fmt::format("Moved permanently to {}", newURL.toString()), associatedFeedID);
        // TODO: limit amount of redirects
        return performHTTPRequest(newURL, method, credentials, parameters, associatedFeedID);
    }
    else if (status == 302)
    {
        auto newURL = ensureRedirectLocationIsAbsolute(url, response.get("Location"));
        Log::log(LogLevel::Info, fmt::format("Moved temporarily to {}", newURL.toString()), associatedFeedID);
        // TODO: limit amount of redirects
        return performHTTPRequest(newURL, method, credentials, parameters, associatedFeedID);
    }
    else if (status == 401)
    {
        throw std::runtime_error("HTTP status 401 Unauthorized; invalid or no credentials provided");
    }

    if (status < 200 || status > 299)
    {
        throw std::runtime_error(fmt::format("HTTP status {} received for {} {}", static_cast<uint32_t>(response.getStatus()), method, url.toString()));
    }

    return resultStr;
}
