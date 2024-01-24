/*
    ZapFeedReader - RSS/Atom feed reader
    Copyright (C) 1023-present  Kasper Nauwelaerts (zapfr at zappatic dot net)

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

#include "Listener.h"
#include <iostream>
#include <unistd.h>

#include <Poco/Environment.h>
#include <Poco/FileStream.h>
#include <Poco/JSON/Object.h>
#include <Poco/TemporaryFile.h>

#include "Daemon.h"
#include "ZapFR/Database.h"
#include "ZapFR/local/SourceLocal.h"

CATCH_REGISTER_LISTENER(ZapFR::Tests::TestRunListener)

void ZapFR::Tests::TestRunListener::testRunStarting(Catch::TestRunInfo const&)
{
    createFakeTestServerConfig();
    mServerThread = std::thread(&TestRunListener::launchTestServer, this);
}

void ZapFR::Tests::TestRunListener::testRunEnded(Catch::TestRunStats const&)
{
    mRunServer = false;
    mServerThread.join();
    if (!mTemporaryDirectory.empty())
    {
        auto f = Poco::File(mTemporaryDirectory);
        f.remove(true);
    }
}

void ZapFR::Tests::TestRunListener::createFakeTestServerConfig()
{
    Poco::JSON::Object o;
    o.set("bind", "0.0.0.0");
    o.set("port", msPort);
    o.set("ssl_pubcert", "");
    o.set("ssl_privkey", "");
    o.set("servername", "ZapFR Unit Test Server");

    Poco::JSON::Array accounts;
    Poco::JSON::Object account;
    account.set("login", "unittest");
    account.set("password", "unittest");
    accounts.add(account);
    o.set("accounts", accounts);

    Poco::JSON::Object autoRefreshObj;
    autoRefreshObj.set("enabled", false);
    autoRefreshObj.set("interval", 900);
    o.set("autorefresh", autoRefreshObj);

    o.set("loglevel", "debug");

    Poco::JSON::Object configJSON;
    configJSON.set("zapfr", o);

    auto configFile = Poco::TemporaryFile();
    configFile.keepUntilExit();
    Poco::FileOutputStream fos(configFile.path());
    Poco::JSON::Stringifier::stringify(configJSON, fos);
    mTemporaryConfigFilePath = configFile.path();
}

void ZapFR::Tests::TestRunListener::launchTestServer()
{
    auto daemon = ZapFR::Server::Daemon(mTemporaryConfigFilePath);
    daemon.boot();

    auto tmpFile = Poco::TemporaryFile();
    tmpFile.keep();
    mTemporaryDirectory = tmpFile.path();
    auto tmpDir = Poco::File(mTemporaryDirectory);
    tmpDir.createDirectories();
    daemon.setDataDir(mTemporaryDirectory);

    createRemoteSource();

    while (mRunServer)
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
    }
}

void ZapFR::Tests::TestRunListener::createRemoteSource()
{
    Poco::JSON::Object configData;
    configData.set(ZapFR::Engine::JSON::RemoteConfigData::Host, "127.0.0.1");
    configData.set(ZapFR::Engine::JSON::RemoteConfigData::Port, msPort);
    configData.set(ZapFR::Engine::JSON::RemoteConfigData::Login, "unittest");
    configData.set(ZapFR::Engine::JSON::RemoteConfigData::Password, "unittest");
    configData.set(ZapFR::Engine::JSON::RemoteConfigData::UseHTTPS, false);
    std::stringstream configDataStream;
    Poco::JSON::Stringifier::stringify(configData, configDataStream);
    ZapFR::Engine::Source::create(ZapFR::Engine::ServerIdentifier::Remote, "Remote pointing to local unittest server", configDataStream.str());
}