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

#include <Poco/Net/SecureServerSocket.h>

#include "APIRequestHandlerFactory.h"
#include "HTTPServer.h"

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
#if POCO_VERSION < 0x010A0000
        mHTTPSContext = new Poco::Net::Context(Poco::Net::Context::TLSV1_3_SERVER_USE, mPrivKey, mPubCert, mPubCert, Poco::Net::Context::VERIFY_NONE);
#else
        mHTTPSContext = new Poco::Net::Context(Poco::Net::Context::TLS_SERVER_USE, mPrivKey, mPubCert, mPubCert, Poco::Net::Context::VERIFY_NONE);
        mHTTPSContext->requireMinimumProtocol(Poco::Net::Context::PROTO_TLSV1_3);
#endif

        Poco::Net::SecureServerSocket socket(socketAddress, 64, mHTTPSContext.get());
        mPocoHTTPServer = std::make_unique<Poco::Net::HTTPServer>(new APIRequestHandlerFactory(mDaemon), socket, serverParams);
    }
    mPocoHTTPServer->start();
}
