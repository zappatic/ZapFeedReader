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

#include <grp.h>
#include <iostream>
#include <pwd.h>
#include <string>

#include <Poco/File.h>

namespace ZapFR
{
    namespace Server
    {
        class ServerApp : public Poco::Util::ServerApplication
        {
          public:
            int main(const std::vector<std::string>& /*args*/) override
            {
                auto daemon = Daemon("/etc/zapfeedreader/zapfeedreader.conf");
                daemon.boot();

                auto user = daemon.configString("zapfr.user");
                auto group = daemon.configString("zapfr.group");
                auto homeDir = std::string(dropRootPrivilege(user, group));
                if (homeDir.empty())
                {
                    throw std::runtime_error("No HOME folder found");
                }

                Poco::File dir(homeDir + Poco::Path::separator() + ".local/share/ZapFeedReader/server");
                if (!dir.exists())
                {
                    dir.createDirectories();
                }
                daemon.setDataDir(dir.path());

                waitForTerminationRequest();

                return Poco::Util::Application::ExitCode::EXIT_OK;
            }

            const char* dropRootPrivilege(const std::string& user, const std::string& group) const
            {
                auto groupInfo = getgrnam(group.c_str());
                auto userInfo = getpwnam(user.c_str());
                if (groupInfo == nullptr)
                {
                    std::cerr << "Unknown group name specified in zapfeedreader.conf; cannot drop root privilege\n";
                    return "";
                }
                if (userInfo == nullptr)
                {
                    std::cerr << "Unknown user name specified in zapfeedreader.conf; cannot drop root privilege\n";
                    return "";
                }

                if (setgid(groupInfo->gr_gid) == -1)
                {
                    std::cerr << "Failed setting group ID to " << groupInfo->gr_gid << "\n";
                    return "";
                }

                if (setuid(userInfo->pw_uid) == -1)
                {
                    std::cerr << "Failed setting user ID to " << groupInfo->gr_gid << "\n";
                    return "";
                }

                return getpwuid(getuid())->pw_dir;
            }
        };
    } // namespace Server
} // namespace ZapFR

POCO_SERVER_MAIN(ZapFR::Server::ServerApp)