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

#ifndef ZAPFR_CLIENT_CLIENTGLOBAL_H
#define ZAPFR_CLIENT_CLIENTGLOBAL_H

#include "APIErrors.h"
#include "ZapFR/Global.h"

#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/SecureServerSocket.h>
#include <Poco/NullStream.h>
#include <Poco/Util/JSONConfiguration.h>

#include <grp.h>
#include <pwd.h>

namespace ZapFR
{
    namespace Server
    {
        class APIError : public std::runtime_error
        {
          public:
            explicit APIError(APIErrorCode c) : std::runtime_error(std::to_string(static_cast<uint64_t>(c))) {}
        };

        class FourOhFourError : public std::runtime_error
        {
          public:
            explicit FourOhFourError() : std::runtime_error("404 Not found") {}
        };

        class UnauthorizedError : public std::runtime_error
        {
          public:
            explicit UnauthorizedError(const std::string& s = "") : std::runtime_error(s) {}
        };
    } // namespace Server
} // namespace ZapFR

#endif // ZAPFR_CLIENT_CLIENTGLOBAL_H