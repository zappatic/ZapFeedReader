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

#include "Daemon.h"
#include "HTTPServer.h"
#include "ZapFR/Database.h"
#include "ZapFR/local/FeedLocal.h"
#include "ZapFR/local/ScriptLocal.h"

namespace
{
    static const std::string gsConfigurationPath{"/etc/zapfeedreader/zapfeedreader.conf"};
    static std::string gsUserHomePath{""};
} // namespace

int ZapFR::Server::Daemon::main(const std::vector<std::string>& /*args*/)
{
    mConfiguration = Poco::AutoPtr<Poco::Util::JSONConfiguration>(new Poco::Util::JSONConfiguration(gsConfigurationPath));

    auto bindAddress = mConfiguration->getString("zapfr.bind", "0.0.0.0");
    auto bindPort = static_cast<uint16_t>(mConfiguration->getUInt("zapfr.port", ZapFR::Engine::DefaultServerPort));
    auto sslPubCert = mConfiguration->getString("zapfr.ssl_pubcert", "");
    auto sslPrivKey = mConfiguration->getString("zapfr.ssl_privkey", "");

    HTTPServer server(this, bindAddress, bindPort, sslPubCert, sslPrivKey);
    server.start();

    auto user = mConfiguration->getString("zapfr.user", "");
    auto group = mConfiguration->getString("zapfr.group", "");
    gsUserHomePath = server.dropRootPrivilege(user, group);

    ZapFR::Engine::ScriptLocal::setScriptDir(dataDir() + Poco::Path::separator() + "scripts");
    ZapFR::Engine::FeedLocal::setIconDir(dataDir() + Poco::Path::separator() + "icons");
    ZapFR::Engine::Database::getInstance()->initialize(dataDir() + Poco::Path::separator() + "zapfeedreader.db", ZapFR::Engine::ApplicationType::Server);

    waitForTerminationRequest();

    return Poco::Util::Application::ExitCode::EXIT_OK;
}

void ZapFR::Server::Daemon::initialize(Poco::Util::Application& /*self*/)
{
    Poco::Net::initializeSSL();
    Poco::Net::initializeNetwork();
}

void ZapFR::Server::Daemon::uninitialize()
{
    Poco::Net::uninitializeSSL();
    Poco::Net::uninitializeNetwork();
}

std::string ZapFR::Server::Daemon::dataDir()
{
    if (gsUserHomePath.empty())
    {
        throw std::runtime_error("No HOME folder found");
    }

    Poco::File dir(gsUserHomePath + Poco::Path::separator() + ".local/share/ZapFeedReader/server");
    if (!dir.exists())
    {
        dir.createDirectories();
    }
    return dir.path();
}

std::string ZapFR::Server::Daemon::configString(const std::string& key)
{
    return mConfiguration->getString(key, "");
}
