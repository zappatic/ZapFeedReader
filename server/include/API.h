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

#ifndef ZAPFR_SERVER_API_H
#define ZAPFR_SERVER_API_H

#include <functional>
#include <memory>
#include <unordered_set>

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/RegularExpression.h>

namespace ZapFR
{
    namespace Server
    {
        class Daemon;
        class APIRequest;

        class URIParameter
        {
          public:
            URIParameter(const std::string& name, const std::string& description) : mName(name), mDescription(description) {}

            std::string name() const noexcept { return mName; }
            std::string description() const noexcept { return mDescription; }

          private:
            std::string mName;
            std::string mDescription;
        };

        class BodyParameter
        {
          public:
            BodyParameter(const std::string& name, bool isRequired, const std::string& description) : mName(name), mRequired(isRequired), mDescription(description) {}

            std::string name() const noexcept { return mName; }
            bool isRequired() const noexcept { return mRequired; }
            std::string description() const noexcept { return mDescription; }

          private:
            std::string mName;
            bool mRequired;
            std::string mDescription;
        };

        class API
        {
          public:
            API(Daemon* daemon, const std::string& section, const std::string& description);
            virtual ~API() = default;
            API(const API&) = delete;
            API& operator=(const API&) = delete;
            API(API&&) = delete;
            API& operator=(API&&) = delete;

            Daemon* daemon() const noexcept;
            void setMethod(const std::string& method);
            std::string method() const noexcept;
            void setPath(const std::string& path, const std::string& prettyPath);
            std::string pathPattern() const noexcept;
            std::string prettyPath() const noexcept;
            Poco::RegularExpression* pathRegex() const noexcept;
            void addURIParameter(const URIParameter& uriParameter);
            void addBodyParameter(const BodyParameter& bodyParameter);
            std::unordered_set<std::string> requiredBodyParameters() const;
            void setRequiresCredentials(bool b);
            bool requiresCredentials() const noexcept;
            void setContentType(const std::string& ct);
            std::string contentType() const noexcept;
            void setJSONOutput(const std::string& jo);
            void setHandler(const std::function<Poco::Net::HTTPResponse::HTTPStatus(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)>& handler);
            auto handler() const noexcept { return mHandler; }

            static void initializeAPIs(Daemon* daemon);
            static API* findAPIForRequest(const Poco::Net::HTTPServerRequest& request);
            static std::vector<std::string> findAPIMethodsForRequest(const Poco::Net::HTTPServerRequest& request);

          private:
            Daemon* mDaemon{nullptr};
            std::string mSection;
            std::string mDescription;

            std::string mMethod{""};
            std::string mPath{""};
            std::unique_ptr<Poco::RegularExpression> mPathRegex{nullptr};
            std::string mPrettyPath{""};
            std::vector<URIParameter> mURIParameters{};
            std::vector<BodyParameter> mBodyParameters{};
            bool mRequiresCredentials{false};
            std::string mContentType{""};
            std::string mJSONOutput{""};

            std::function<Poco::Net::HTTPResponse::HTTPStatus(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)> mHandler{};

            static std::vector<std::unique_ptr<API>> msAPIs;
            static bool msAPIsLoaded;

            static void registerAPI(std::unique_ptr<API> api) { msAPIs.emplace_back(std::move(api)); }
        };
    } // namespace Server
} // namespace ZapFR

#endif // ZAPFR_SERVER_APIREQUEST_H
