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

#include "APIRequest.h"
#include "API.h"
#include "ZapFR/Helpers.h"

ZapFR::Server::APIRequest::APIRequest(API* api, Poco::Net::HTTPServerRequest& request) : mAPI(api)
{
    mRequest = &request;
    mURI = Poco::URI("https://" + request.getHost() + request.getURI());

    std::vector<std::string> ipAddressComponents;
    ZapFR::Engine::Helpers::splitString(request.clientAddress().toString(), ':', ipAddressComponents);
    ipAddressComponents.pop_back();
    mClientIPAddress = ZapFR::Engine::Helpers::joinString(ipAddressComponents, ":");

    if (api->requiresCredentials())
    {
        // TODO
    }

    // Load path components from the URI
    api->pathRegex()->split(mURI.getPath(), mPathComponents);

    mParameters.setValueLengthLimit(25 * 1024 * 1024);
    auto contentType = request.getContentType();
    bool parseBodyAsForm = false;
    bool bodyConsumed = false;

    if (contentType.starts_with("application/x-www-form-urlencoded"))
    {
        mParameters.setEncoding("application/x-www-form-urlencoded");
        parseBodyAsForm = true;
    }
    else if (contentType.starts_with("multipart/form-data"))
    {
        mParameters.setEncoding("multipart/form-data");
        parseBodyAsForm = true;
    }
    if (parseBodyAsForm)
    {
        try
        {
            mParameters.load(request, request.stream());
        }
        catch (...)
        {
            // noop : ignore parsing the body of the stream, a POST request without a body will trigger an exception, but it's fine, so let it pass
        }
        bodyConsumed = true;
    }
    else
    {
        mParameters.load(request);
    }

    if (!bodyConsumed)
    {
        // throw away the request body ( so it doesn't end up in the next request )
        auto nos = Poco::NullOutputStream();
        Poco::StreamCopier::copyStream64(request.stream(), nos);
    }

    for (const auto& requiredParameter : mAPI->requiredBodyParameters())
    {
        if (!mParameters.has(requiredParameter) || mParameters.get(requiredParameter).empty())
        {
            throw std::runtime_error("Missing parameter: " + requiredParameter);
        }
    }
}

std::string ZapFR::Server::APIRequest::pathComponentAt(size_t index) const
{
    if (index < mPathComponents.size())
    {
        return mPathComponents.at(index);
    }
    return "";
}

std::string ZapFR::Server::APIRequest::parameter(const std::string& key, const std::string& defaultValue) const
{
    return mParameters.get(key, defaultValue);
}

bool ZapFR::Server::APIRequest::hasParameter(const std::string& key) const noexcept
{
    return mParameters.has(key);
}
