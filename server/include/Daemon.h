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

#ifndef ZAPFR_SERVER_DAEMON_H
#define ZAPFR_SERVER_DAEMON_H

#include "HTTPServer.h"
#include <Poco/Util/JSONConfiguration.h>
#include <Poco/Util/ServerApplication.h>
#include <inja/inja.hpp>

namespace ZapFR
{
    namespace Server
    {
        struct Account
        {
            Account(const std::string& accountLogin, const std::string& accountPassword) : login(accountLogin), password(accountPassword) {}
            std::string login;
            std::string password;
        };

        namespace ConfigKeys
        {
            inline const std::string LOGLEVEL{"loglevel"};
            inline const std::string AUTOREFRESH_ENABLED{"zapfr.autorefresh.enabled"};
            inline const std::string AUTOREFRESH_INTERVAL{"zapfr.autorefresh.interval"};
            inline const std::string BIND_TO{"zapfr.bind"};
            inline const std::string PORT{"zapfr.port"};
            inline const std::string PUB_CERT{"zapfr.ssl_pubcert"};
            inline const std::string PRIV_KEY{"zapfr.ssl_privkey"};
            inline const std::string ACCOUNTS{"zapfr.accounts"};
            inline const std::string ACCOUNT_LOGIN{"login"};
            inline const std::string ACCOUNT_PASSWORD{"password"};
            inline const std::string RUN_AS_USER{"zapfr.user"};
            inline const std::string RUN_AS_GROUP{"zapfr.group"};
            inline const std::string SERVERNAME{"zapfr.servername"};
            inline const std::string SERVER_WEB_INTERFACE{"zapfr.servewebinterface"};
        } // namespace ConfigKeys

        class Daemon
        {
          public:
            Daemon();
            virtual ~Daemon();
            void boot();
            void setDataDir(const std::string& dataDir);

            Poco::Path configRoot() const;
            Poco::Path webinterfaceRoot() const;
            std::string configString(const std::string& key);
            bool configBool(const std::string& key);
            bool hasAccounts() const noexcept;
            bool areCredentialsValid(const std::string& login, const std::string& password) const;
            inja::Environment& injaEnv() const;

          private:
            std::string mDataDir{""};
            Poco::AutoPtr<Poco::Util::JSONConfiguration> mConfiguration{nullptr};
            std::vector<Account> mAccounts{};
            std::unique_ptr<HTTPServer> mHTTPServer{nullptr};

            void loadAccounts();
        };
    } // namespace Server
} // namespace ZapFR

#endif // ZAPFR_SERVER_DAEMON_H