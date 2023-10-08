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

#include <catch2/catch_test_macros.hpp>

#include "DataFetcher.h"
#include "ZapFR/feed_handling/FeedDiscovery.h"

TEST_CASE("Feed Discovery on hackernews", "[feeddiscovery]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "WebPageHackernews.html");

    auto fd = ZapFR::Engine::FeedDiscovery("https://news.ycombinator.com", input);
    fd.discover();

    REQUIRE(fd.discoveredFeeds().size() == 1);
    REQUIRE(fd.discoveredFeeds().at(0).title == "RSS");
    REQUIRE(fd.discoveredFeeds().at(0).url == "https://news.ycombinator.com/rss");
    REQUIRE(fd.discoveredFeeds().at(0).type == ZapFR::Engine::Feed::Type::RSS);
}

TEST_CASE("Feed Discovery on pawelgrzybek.com (no quotes around attribs, unclosed link tags)", "[feeddiscovery]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "WebPagePawel.html");

    auto fd = ZapFR::Engine::FeedDiscovery("https://pawelgrzybek.com/", input);
    fd.discover();

    REQUIRE(fd.discoveredFeeds().size() == 2);
    REQUIRE(fd.discoveredFeeds().at(0).title == "pawelgrzybek.com");
    REQUIRE(fd.discoveredFeeds().at(0).url == "https://pawelgrzybek.com/feed.xml");
    REQUIRE(fd.discoveredFeeds().at(0).type == ZapFR::Engine::Feed::Type::RSS);
    REQUIRE(fd.discoveredFeeds().at(1).title == "pawelgrzybek.com");
    REQUIRE(fd.discoveredFeeds().at(1).url == "https://pawelgrzybek.com/feed.json");
    REQUIRE(fd.discoveredFeeds().at(1).type == ZapFR::Engine::Feed::Type::JSON);
}

TEST_CASE("Feed Discovery on youtube.com", "[feeddiscovery]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "WebPageYoutubeMrBeast.html");

    auto fd = ZapFR::Engine::FeedDiscovery("https://www.youtube.com/@MrBeast", input);
    fd.discover();

    REQUIRE(fd.discoveredFeeds().size() == 1);
    REQUIRE(fd.discoveredFeeds().at(0).title == "MrBeast - YouTube");
    REQUIRE(fd.discoveredFeeds().at(0).url == "https://www.youtube.com/feeds/videos.xml?channel_id=UCX6OQ3DkcsbYNE6H8uQQuVA");
    REQUIRE(fd.discoveredFeeds().at(0).type == ZapFR::Engine::Feed::Type::Atom);
}

TEST_CASE("Feed Discovery on direct link to RSS feed", "[feeddiscovery]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "FeedRSS20Hackernews.xml");

    auto fd = ZapFR::Engine::FeedDiscovery("https://news.ycombinator.com/rss", input);
    fd.discover();

    REQUIRE(fd.discoveredFeeds().size() == 1);
    REQUIRE(fd.discoveredFeeds().at(0).title == "RSS Feed");
    REQUIRE(fd.discoveredFeeds().at(0).url == "https://news.ycombinator.com/rss");
    REQUIRE(fd.discoveredFeeds().at(0).type == ZapFR::Engine::Feed::Type::RSS);
}

TEST_CASE("Feed Discovery on direct link to Atom feed", "[feeddiscovery]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "FeedATOM10DaringFireball.xml");

    auto fd = ZapFR::Engine::FeedDiscovery("https://daringfireball.net/feeds/main", input);
    fd.discover();

    REQUIRE(fd.discoveredFeeds().size() == 1);
    REQUIRE(fd.discoveredFeeds().at(0).title == "Atom Feed");
    REQUIRE(fd.discoveredFeeds().at(0).url == "https://daringfireball.net/feeds/main");
    REQUIRE(fd.discoveredFeeds().at(0).type == ZapFR::Engine::Feed::Type::Atom);
}

TEST_CASE("Feed Discovery on direct link to JSON feed", "[feeddiscovery]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "FeedJSON11DaringFireball.json");

    auto fd = ZapFR::Engine::FeedDiscovery("https://daringfireball.net/feeds/json", input);
    fd.discover();

    REQUIRE(fd.discoveredFeeds().size() == 1);
    REQUIRE(fd.discoveredFeeds().at(0).title == "Daring Fireball");
    REQUIRE(fd.discoveredFeeds().at(0).url == "https://daringfireball.net/feeds/json");
    REQUIRE(fd.discoveredFeeds().at(0).type == ZapFR::Engine::Feed::Type::JSON);
}
