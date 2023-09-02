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

#ifndef ZAPFR_SERVER_HTTPSERVER_H
#define ZAPFR_SERVER_HTTPSERVER_H

#include "ServerGlobal.h"

namespace ZapFR
{
    namespace Server
    {
        class Daemon;

        class HTTPServer
        {
          public:
            HTTPServer(Daemon* daemon, const std::string& bindAddress, uint16_t port, const std::string& pubCert, const std::string& privKey);
            virtual ~HTTPServer() = default;
            HTTPServer(const HTTPServer& e) = delete;
            HTTPServer& operator=(const HTTPServer&) = delete;
            HTTPServer(HTTPServer&&) = delete;
            HTTPServer& operator=(HTTPServer&&) = delete;

            void start();
            const char* dropRootPrivilege(const std::string& user, const std::string& group) const;

          private:
            Daemon* mDaemon{nullptr};
            std::string mBindAddress{""};
            uint16_t mBindPort{0};
            std::string mPubCert{""};
            std::string mPrivKey{""};

            Poco::Net::Context::Ptr mHTTPSContext{nullptr};
            std::unique_ptr<Poco::Net::HTTPServer> mPocoHTTPServer{nullptr};
        };
    } // namespace Server
} // namespace ZapFR

#endif // ZAPFR_SERVER_DAEMON_H