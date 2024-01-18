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
#include "ZapFR/base/Feed.h"
#include "ZapFR/feed_handling/FeedFetcher.h"
#include "ZapFR/feed_handling/FeedParser.h"

TEST_CASE("Feed Fetcher - detect as RSS 2.0", "[feedfetcher]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "FeedRSS20Hackernews.xml");

    auto feedFetcher = ZapFR::Engine::FeedFetcher();
    const auto& feedParser = feedFetcher.parseString(input, "https://news.ycombinator.com/rss");

    REQUIRE(feedParser != nullptr);
    REQUIRE(feedParser->type() == ZapFR::Engine::Feed::Type::RSS);
}

TEST_CASE("Feed Fetcher - detect as RSS 1.0", "[feedfetcher]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "FeedRSS10Slashdot.xml");

    auto feedFetcher = ZapFR::Engine::FeedFetcher();
    const auto& feedParser = feedFetcher.parseString(input, "https://rss.slashdot.org/Slashdot/slashdotMain");

    REQUIRE(feedParser != nullptr);
    REQUIRE(feedParser->type() == ZapFR::Engine::Feed::Type::RSS);
}

TEST_CASE("Feed Fetcher - detect as Atom", "[feedfetcher]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "FeedATOM10DaringFireball.xml");

    auto feedFetcher = ZapFR::Engine::FeedFetcher();
    const auto& feedParser = feedFetcher.parseString(input, "https://daringfireball.net/feeds/json");

    REQUIRE(feedParser != nullptr);
    REQUIRE(feedParser->type() == ZapFR::Engine::Feed::Type::Atom);
}

TEST_CASE("Feed Fetcher - detect as JSON", "[feedfetcher]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "FeedJSON11DaringFireball.json");

    auto feedFetcher = ZapFR::Engine::FeedFetcher();
    const auto& feedParser = feedFetcher.parseString(input, "https://daringfireball.net/feeds/json");

    REQUIRE(feedParser != nullptr);
    REQUIRE(feedParser->type() == ZapFR::Engine::Feed::Type::JSON);
}

TEST_CASE("Feed Fetcher - fetch valid rss from remote site", "[feedfetcher]")
{
    auto feedFetcher = ZapFR::Engine::FeedFetcher();
    const auto& feedParser = feedFetcher.parseURL("https://zapfeedreader.zappatic.net/unittests/rss.xml", 0, {});

    REQUIRE(feedParser.has_value());
    REQUIRE(feedParser.value()->type() == ZapFR::Engine::Feed::Type::RSS);
}

TEST_CASE("Feed Fetcher - fetch empty file from remote site", "[feedfetcher]")
{
    auto feedFetcher = ZapFR::Engine::FeedFetcher();
    const auto& feedParser = feedFetcher.parseURL("https://zapfeedreader.zappatic.net/unittests/empty.xml", 0, {});

    REQUIRE(!feedParser.has_value());
}

TEST_CASE("Feed Fetcher - pass empty string to parseString", "[feedfetcher]")
{
    auto feedFetcher = ZapFR::Engine::FeedFetcher();
    const auto& feedParser = feedFetcher.parseString("", "");

    REQUIRE(feedParser == nullptr);
}

TEST_CASE("Feed Fetcher - parse unknown feed type", "[feedfetcher]")
{
    auto feedFetcher = ZapFR::Engine::FeedFetcher();
    REQUIRE_THROWS(feedFetcher.parseString(R"(<?xml version="1.0" encoding="utf-8"?><invalid-feed-tag><data></data></invalid-feed-tag>)", ""));
}

TEST_CASE("Feed Fetcher - parse non-xml/json", "[feedfetcher]")
{
    auto feedFetcher = ZapFR::Engine::FeedFetcher();
    const auto& feedParser = feedFetcher.parseString("?", "");

    REQUIRE(feedParser == nullptr);
}
