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

#include "HTTPServer.h"
#include "APIRequestHandlerFactory.h"

ZapFR::Server::HTTPServer::HTTPServer(Daemon* daemon, const std::string& bindAddress, uint16_t port, const std::string& pubCert, const std::string& privKey)
    : mDaemon(daemon), mBindAddress(bindAddress), mBindPort(port), mPubCert(pubCert), mPrivKey(privKey)
{
}

void ZapFR::Server::HTTPServer::start()
{
    auto serverParams = new Poco::Net::HTTPServerParams;
    serverParams->setMaxQueued(100);
    serverParams->setMaxThreads(16);

    Poco::Net::SocketAddress socketAddress(static_cast<Poco::UInt16>(mBindPort));
    if (mPrivKey.empty())
    {
        Poco::Net::ServerSocket socket(socketAddress);
        mPocoHTTPServer = std::make_unique<Poco::Net::HTTPServer>(new APIRequestHandlerFactory(mDaemon), socket, serverParams);
    }
    else
    {
        mHTTPSContext = new Poco::Net::Context(Poco::Net::Context::TLS_SERVER_USE, mPrivKey, mPubCert, mPubCert, Poco::Net::Context::VERIFY_NONE);
        mHTTPSContext->requireMinimumProtocol(Poco::Net::Context::PROTO_TLSV1_3);

        Poco::Net::SecureServerSocket socket(socketAddress, 64, mHTTPSContext.get());
        mPocoHTTPServer = std::make_unique<Poco::Net::HTTPServer>(new APIRequestHandlerFactory(mDaemon), socket, serverParams);
    }

    mPocoHTTPServer->start();
}

void ZapFR::Server::HTTPServer::dropRootPrivilege(const std::string& user, const std::string& group) const
{
    auto groupInfo = getgrnam(group.c_str());
    auto userInfo = getpwnam(user.c_str());
    if (groupInfo == nullptr)
    {
        std::cerr << "Unknown group name specified in zapfeedreader.conf; cannot drop root privilege\n";
        return;
    }
    if (userInfo == nullptr)
    {
        std::cerr << "Unknown user name specified in zapfeedreader.conf; cannot drop root privilege\n";
        return;
    }

    if (setgid(groupInfo->gr_gid) == -1)
    {
        std::cerr << "Failed setting group ID to " << groupInfo->gr_gid << "\n";
        return;
    }

    if (setuid(userInfo->pw_uid) == -1)
    {
        std::cerr << "Failed setting user ID to " << groupInfo->gr_gid << "\n";
        return;
    }
}