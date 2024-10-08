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

#include <Poco/JSON/Parser.h>

#include "Daemon.h"
#include "ZapFR/AutoRefresh.h"
#include "ZapFR/Database.h"
#include "ZapFR/Log.h"
#include "ZapFR/local/FeedLocal.h"
#include "ZapFR/local/ScriptLocal.h"

namespace
{
    static std::string gsUserHomePath{""};
} // namespace

ZapFR::Server::Daemon::Daemon(const std::string& configurationPath) : mConfigurationPath(configurationPath)
{
    Poco::Net::initializeSSL();
    Poco::Net::initializeNetwork();
}

ZapFR::Server::Daemon::~Daemon()
{
    Poco::Net::uninitializeSSL();
    Poco::Net::uninitializeNetwork();
}

void ZapFR::Server::Daemon::boot()
{
    mConfiguration = Poco::AutoPtr<Poco::Util::JSONConfiguration>(new Poco::Util::JSONConfiguration(mConfigurationPath));

    loadAccounts();
    auto ar = ZapFR::Engine::AutoRefresh::getInstance();
    ar->setEnabled(mConfiguration->getBool("zapfr.autorefresh.enabled", true));
    ar->setFeedRefreshInterval(mConfiguration->getUInt64("zapfr.autorefresh.interval", ZapFR::Engine::DefaultFeedAutoRefreshInterval));

    auto logLevel = mConfiguration->getString("loglevel", "info");
    if (logLevel == "debug")
    {
        ZapFR::Engine::Log::setLogLevel(ZapFR::Engine::LogLevel::Debug);
    }
    else if (logLevel == "info")
    {
        ZapFR::Engine::Log::setLogLevel(ZapFR::Engine::LogLevel::Info);
    }
    else if (logLevel == "warning")
    {
        ZapFR::Engine::Log::setLogLevel(ZapFR::Engine::LogLevel::Warning);
    }
    else if (logLevel == "error")
    {
        ZapFR::Engine::Log::setLogLevel(ZapFR::Engine::LogLevel::Error);
    }

    auto bindAddress = mConfiguration->getString("zapfr.bind", "0.0.0.0");
    auto bindPort = static_cast<uint16_t>(mConfiguration->getUInt("zapfr.port", ZapFR::Engine::DefaultServerPort));
    auto sslPubCert = mConfiguration->getString("zapfr.ssl_pubcert", "");
    auto sslPrivKey = mConfiguration->getString("zapfr.ssl_privkey", "");

    mHTTPServer = std::make_unique<HTTPServer>(this, bindAddress, bindPort, sslPubCert, sslPrivKey);
    mHTTPServer->start();
}

void ZapFR::Server::Daemon::loadAccounts()
{
    try
    {
        if (mConfiguration->has("zapfr.accounts"))
        {
            auto accounts = Poco::JSON::Parser().parse(mConfiguration->getRawString("zapfr.accounts")).extract<Poco::JSON::Array::Ptr>();
            for (size_t i = 0; i < accounts->size(); ++i)
            {
                auto account = accounts->getObject(static_cast<uint32_t>(i));
                auto login = account->getValue<std::string>("login");
                auto password = account->getValue<std::string>("password");
                mAccounts.emplace_back(login, password);
            }
        }
    }
    catch (const Poco::Exception& e)
    {
        std::cerr << "Failed loading accounts from configuration file: " << e.displayText() << "\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed loading accounts from configuration file: " << e.what() << "\n";
    }
    catch (...)
    {
        std::cerr << "Failed loading accounts from configuration file: unknown error\n";
    }
}

void ZapFR::Server::Daemon::setDataDir(const std::string& dataDir)
{
    static bool isSet{false};
    if (isSet)
    {
        throw std::runtime_error("Data dir has already been set!");
    }
    mDataDir = dataDir;
    isSet = true;
    ZapFR::Engine::FeedLocal::setIconDir(mDataDir + Poco::Path::separator() + "icons");
    ZapFR::Engine::Database::getInstance()->initialize(mDataDir + Poco::Path::separator() + "zapfeedreader.db", ZapFR::Engine::ApplicationType::Server);
}

std::string ZapFR::Server::Daemon::configString(const std::string& key)
{
    return mConfiguration->getString(key, "");
}

bool ZapFR::Server::Daemon::hasAccounts() const noexcept
{
    return !mAccounts.empty();
}

bool ZapFR::Server::Daemon::areCredentialsValid(const std::string& login, const std::string& password) const
{
    for (const auto& account : mAccounts)
    {
        if (account.login == login && account.password == password)
        {
            return true;
        }
    }
    return false;
}
