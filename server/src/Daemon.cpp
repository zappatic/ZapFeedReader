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
#include "FeedFetcher.h"

int ZapFR::Server::Daemon::main(const std::vector<std::string>& /*args*/)
{
    mDatabase = std::make_unique<Engine::Database>(dataDir() + Poco::Path::separator() + "zapfeedreader.db");

    // auto url = "https://en.wikipedia.org/w/api.php?hidebots=1&hidecategorization=1&hideWikibase=1&urlversion=1&days=7&limit=50&action=feedrecentchanges&feedformat=rss";
    // auto url = "https://www.vrt.be/vrtnieuws/nl.rss.articles.xml";
    auto url = "https://news.ycombinator.com/rss";
    auto ff = Engine::FeedFetcher(mDatabase.get());
    ff.subscribeToFeed(url);

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
    Poco::File dir(config().getString("system.dataHomeDir") + Poco::Path::separator() + "ZapFeedReader");
    if (!dir.exists())
    {
        dir.createDirectories();
    }
    return dir.path();
}
