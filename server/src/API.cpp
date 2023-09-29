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

#include <Poco/URI.h>

#include "API.h"
#include "Daemon.h"

ZapFR::Server::API::API(Daemon* daemon, const std::string& section, const std::string& description) : mDaemon(daemon), mSection(section), mDescription(description)
{
}

ZapFR::Server::Daemon* ZapFR::Server::API::daemon() const noexcept
{
    return mDaemon;
}

void ZapFR::Server::API::setMethod(const std::string& method)
{
    mMethod = method;
}

std::string ZapFR::Server::API::method() const noexcept
{
    return mMethod;
}

void ZapFR::Server::API::setPath(const std::string& path, const std::string& prettyPath)
{
    mPath = path;
    mPathRegex = std::make_unique<Poco::RegularExpression>(path);
    mPrettyPath = prettyPath;
}

std::string ZapFR::Server::API::pathPattern() const noexcept
{
    return mPath;
}

std::string ZapFR::Server::API::prettyPath() const noexcept
{
    return mPrettyPath;
}

Poco::RegularExpression* ZapFR::Server::API::pathRegex() const noexcept
{
    return mPathRegex.get();
}

void ZapFR::Server::API::addURIParameter(const URIParameter& uriParameter)
{
    mURIParameters.emplace_back(uriParameter);
}

void ZapFR::Server::API::addBodyParameter(const BodyParameter& bodyParameter)
{
    mBodyParameters.emplace_back(bodyParameter);
}

std::unordered_set<std::string> ZapFR::Server::API::requiredBodyParameters() const
{
    std::unordered_set<std::string> reqdParams;
    for (const auto& parameter : mBodyParameters)
    {
        if (parameter.isRequired())
        {
            reqdParams.insert(parameter.name());
        }
    }
    return reqdParams;
}

void ZapFR::Server::API::setRequiresCredentials(bool b)
{
    mRequiresCredentials = b;
}

bool ZapFR::Server::API::requiresCredentials() const noexcept
{
    return mRequiresCredentials;
}

void ZapFR::Server::API::setContentType(const std::string& ct)
{
    mContentType = ct;
}

std::string ZapFR::Server::API::contentType() const noexcept
{
    return mContentType;
}

void ZapFR::Server::API::setJSONOutput(const std::string& jo)
{
    mJSONOutput = jo;
}

void ZapFR::Server::API::setHandler(const std::function<Poco::Net::HTTPResponse::HTTPStatus(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)>& handler)
{
    mHandler = handler;
}

ZapFR::Server::API* ZapFR::Server::API::findAPIForRequest(const Poco::Net::HTTPServerRequest& request)
{
    auto requestedMethod = request.getMethod();
    for (const auto& api : ZapFR::Server::API::msAPIs)
    {
        if (requestedMethod != "OPTIONS" && requestedMethod != api->method())
        {
            continue;
        }
        Poco::URI uri("https://" + request.getHost() + request.getURI());
        if (api->pathRegex()->match(uri.getPath()))
        {
            return api.get();
        }
    }
    return nullptr;
}

std::vector<std::string> ZapFR::Server::API::findAPIMethodsForRequest(const Poco::Net::HTTPServerRequest& request)
{
    std::vector<std::string> v;
    Poco::URI uri("https://" + request.getHost() + request.getURI());
    for (const auto& api : ZapFR::Server::API::msAPIs)
    {
        if (api->pathRegex()->match(uri.getPath()))
        {
            v.push_back(api->method());
        }
    }
    return v;
}
