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

#ifndef ZAPFR_SERVER_APIREQUESTHANDLER_H
#define ZAPFR_SERVER_APIREQUESTHANDLER_H

#include "ServerGlobal.h"

namespace ZapFR
{
    namespace Server
    {
        class API;

        class APIRequestHandler : public Poco::Net::HTTPRequestHandler
        {
          public:
            explicit APIRequestHandler(API* api);
            ~APIRequestHandler() = default;
            APIRequestHandler(const APIRequestHandler& e) = delete;
            APIRequestHandler& operator=(const APIRequestHandler&) = delete;
            APIRequestHandler(APIRequestHandler&&) = delete;
            APIRequestHandler& operator=(APIRequestHandler&&) = delete;

            void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override;

          private:
            API* mAPI{nullptr};
        };
    } // namespace Server
} // namespace ZapFR

#endif // ZAPFR_SERVER_APIREQUESTHANDLER_H
