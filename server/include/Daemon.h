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

#include <Poco/Util/JSONConfiguration.h>
#include <Poco/Util/ServerApplication.h>

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

        class Daemon : public Poco::Util::ServerApplication
        {
          public:
            int main(const std::vector<std::string>& args) override;
            void initialize(Poco::Util::Application& self) override;
            void uninitialize() override;

            std::string configString(const std::string& key);
            bool hasAccounts() const noexcept;
            bool areCredentialsValid(const std::string& login, const std::string& password) const;

          private:
            Poco::AutoPtr<Poco::Util::JSONConfiguration> mConfiguration{nullptr};
            std::vector<Account> mAccounts{};

            void loadAccounts();
            std::string dataDir();
        };
    } // namespace Server
} // namespace ZapFR

#endif // ZAPFR_SERVER_DAEMON_H