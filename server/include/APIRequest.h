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

#ifndef ZAPFR_SERVER_APIREQUEST_H
#define ZAPFR_SERVER_APIREQUEST_H

#include "ServerGlobal.h"

namespace ZapFR
{
    namespace Server
    {
        class API;

        class APIRequest
        {
          public:
            APIRequest(API* api, Poco::Net::HTTPServerRequest& request);
            ~APIRequest() = default;
            APIRequest(const APIRequest&) = delete;
            APIRequest& operator=(const APIRequest&) = delete;
            APIRequest(APIRequest&&) = delete;
            APIRequest& operator=(APIRequest&&) = delete;

            API* api() const noexcept { return mAPI; }
            Poco::Net::HTTPServerRequest* request() const noexcept { return mRequest; }
            std::string pathComponentAt(size_t index) const;
            std::string parameter(const std::string& key, const std::string& defaultValue = "") const;
            bool hasParameter(const std::string& key) const noexcept;
            const Poco::Net::HTMLForm& parameters() const noexcept { return mParameters; }
            const Poco::URI& uri() const noexcept { return mURI; }
            std::string clientIPAddress() const noexcept { return mClientIPAddress; }

          private:
            API* mAPI{nullptr};
            std::vector<std::string> mPathComponents{};
            Poco::Net::HTTPServerRequest* mRequest{nullptr};
            Poco::Net::HTMLForm mParameters{};
            Poco::URI mURI{};
            std::string mClientIPAddress{""};
        };
    } // namespace Server
} // namespace ZapFR

#endif // ZAPFR_SERVER_APIREQUEST_H
