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

#include <Poco/DOM/DOMParser.h>
#include <Poco/DigestStream.h>
#include <Poco/JSON/Parser.h>
#include <Poco/MD5Engine.h>
#include <catch2/catch_test_macros.hpp>

#include "DataFetcher.h"
#include "ZapFR/feed_handling/FeedParser.h"
#include "ZapFR/feed_handling/FeedParserATOM10.h"
#include "ZapFR/feed_handling/FeedParserJSON11.h"
#include "ZapFR/feed_handling/FeedParserRSS10.h"
#include "ZapFR/feed_handling/FeedParserRSS20.h"

TEST_CASE("Parse ATOM 1.0 (Daring Fireball)", "[feedparsing]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "FeedATOM10DaringFireball.xml");

    Poco::XML::DOMParser parser;
    Poco::AutoPtr<Poco::XML::Document> xmlDoc = parser.parseString(input);

    auto feed = std::make_unique<ZapFR::Engine::FeedParserATOM10>("https://example.com");
    feed->setXMLDoc(xmlDoc);

    REQUIRE(feed->guid() == "https://daringfireball.net/feeds/main");
    REQUIRE(feed->title() == "Daring Fireball");
    REQUIRE(feed->subtitle() == "By John Gruber");
    REQUIRE(feed->link() == "https://daringfireball.net/");
    REQUIRE(feed->description() == "");
    REQUIRE(feed->iconURL() == "");
    REQUIRE(feed->language() == "");
    REQUIRE(feed->copyright() == "Copyright © 2023, John Gruber");

    auto items = feed->items();
    REQUIRE(items.size() == 48);

    const auto& item = items.at(0);
    REQUIRE(item.title == "Orion: Use an iPad as an External HDMI Display");
    REQUIRE(item.link == "https://www.lux.camera/meet-orion/");
    REQUIRE(item.guid == "tag:daringfireball.net,2023:/linked//6.40276");
    REQUIRE(item.content.starts_with("\n<p>Speaking of "));
    REQUIRE(item.author == "John Gruber");
    REQUIRE(item.commentsURL == "");
    REQUIRE(item.datePublished == "2023-10-05T23:57:09Z");
    REQUIRE(item.thumbnail == "");
}

TEST_CASE("Parse ATOM 1.0 (YouTube @cppweekly)", "[feedparsing]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "FeedATOM10YouTubeCppweekly.xml");

    Poco::XML::DOMParser parser;
    Poco::AutoPtr<Poco::XML::Document> xmlDoc = parser.parseString(input);

    auto feed = std::make_unique<ZapFR::Engine::FeedParserATOM10>("https://example.com");
    feed->setXMLDoc(xmlDoc);

    REQUIRE(feed->guid() == "yt:channel:xHAlbZQNFU2LgEtiqd2Maw");
    REQUIRE(feed->title() == "C++ Weekly With Jason Turner");
    REQUIRE(feed->subtitle() == "");
    REQUIRE(feed->link() == "https://www.youtube.com/channel/UCxHAlbZQNFU2LgEtiqd2Maw");
    REQUIRE(feed->description() == "");
    REQUIRE(feed->iconURL() == "");
    REQUIRE(feed->language() == "");
    REQUIRE(feed->copyright() == "");

    auto items = feed->items();
    REQUIRE(items.size() == 15);

    const auto& item = items.at(0);
    REQUIRE(item.title == "CS101++ - What Are The Parts of a Computer?");
    REQUIRE(item.link == "https://www.youtube.com/watch?v=46Czrc2Uwvc");
    REQUIRE(item.guid == "yt:video:46Czrc2Uwvc");
    REQUIRE(item.content.starts_with(R"(<a href="https://www.youtube.com/watch?v)"));
    REQUIRE(item.author == "C++ Weekly With Jason Turner");
    REQUIRE(item.commentsURL == "");
    REQUIRE(item.datePublished == "2024-01-17T16:29:54Z");
    REQUIRE(item.thumbnail == "https://i1.ytimg.com/vi/46Czrc2Uwvc/hqdefault.jpg");
}

TEST_CASE("Parse ATOM 1.0 (custom example)", "[feedparsing]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "FeedATOM10CustomExample.xml");

    Poco::XML::DOMParser parser;
    Poco::AutoPtr<Poco::XML::Document> xmlDoc = parser.parseString(input);

    auto feed = std::make_unique<ZapFR::Engine::FeedParserATOM10>("https://example.com");
    feed->setXMLDoc(xmlDoc);

    REQUIRE(feed->language() == "en");

    auto items = feed->items();
    REQUIRE(items.size() == 3);

    auto& item = items.at(0);
    REQUIRE(item.datePublished == "2003-12-13T18:30:02Z");
    REQUIRE(item.categories.size() == 2);
    REQUIRE(item.categories.at(0) == "term1");

    REQUIRE(item.enclosures.size() == 3);
    REQUIRE(item.enclosures.at(0).size == 100);
    REQUIRE(item.enclosures.at(0).url == "file:///dummy");
    REQUIRE(item.enclosures.at(0).mimeType == "image/jpeg");
    REQUIRE(item.enclosures.at(1).size == 200);
    REQUIRE(item.enclosures.at(1).url == "file:///dummy2");
    REQUIRE(item.enclosures.at(1).mimeType == "image/png");
    REQUIRE(item.enclosures.at(2).size == 300);
    REQUIRE(item.enclosures.at(2).url == "file:///dummy3");
    REQUIRE(item.enclosures.at(2).mimeType == "image/gif");

    item = items.at(1);
    REQUIRE(Poco::trim(item.content).starts_with("<b "));

    item = items.at(2);
    REQUIRE(Poco::trim(item.content).starts_with("<p>")); // TODO: this needs looking at, the <p> is added by us, but not sure whether content should be tag-stripped or not
}

TEST_CASE("Parse JSON 1.1 (Daring Fireball)", "[feedparsing]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "FeedJSON11DaringFireball.json");

    Poco::JSON::Parser parser;
    auto root = parser.parse(input);
    auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
    REQUIRE(rootObj != nullptr);

    auto feed = std::make_unique<ZapFR::Engine::FeedParserJSON11>("https://example.com");
    feed->setRootObj(rootObj);

    REQUIRE(feed->guid() == "https://daringfireball.net/feeds/json");
    REQUIRE(feed->title() == "Daring Fireball");
    REQUIRE(feed->subtitle() == "");
    REQUIRE(feed->link() == "https://daringfireball.net/");
    REQUIRE(feed->description() == "");
    REQUIRE(feed->iconURL() == "https://daringfireball.net/graphics/apple-touch-icon.png");
    REQUIRE(feed->language() == "");
    REQUIRE(feed->copyright() == "");

    auto items = feed->items();
    REQUIRE(items.size() == 48);

    const auto& item = items.at(0);
    REQUIRE(item.title == "Orion: Use an iPad as an External HDMI Display");
    REQUIRE(item.link == "https://daringfireball.net/linked/2023/10/05/orion");
    REQUIRE(item.guid == "https://daringfireball.net/linked/2023/10/05/orion");
    REQUIRE(item.content.starts_with("\n<p>Speaking of "));
    REQUIRE(item.author == "John Gruber");
    REQUIRE(item.commentsURL == "");
    REQUIRE(item.datePublished == "2023-10-05T23:57:09Z");
    REQUIRE(item.thumbnail == "");
}

TEST_CASE("Parse JSON 1.1 (custom example)", "[feedparsing]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "FeedJSON11CustomExample.json");

    Poco::JSON::Parser parser;
    auto root = parser.parse(input);
    auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
    REQUIRE(rootObj != nullptr);

    auto feed = std::make_unique<ZapFR::Engine::FeedParserJSON11>("https://example.com");
    feed->setRootObj(rootObj);

    REQUIRE(feed->guid() == "");
    REQUIRE(feed->link() == "");
    REQUIRE(feed->description() == "this is the description");
    REQUIRE(feed->iconURL() == "file://favicon.ico");
    REQUIRE(feed->language() == "en");

    auto items = feed->items();
    REQUIRE(items.size() == 1);

    const auto& item = items.at(0);
    REQUIRE(item.author == "Overriding author");
    REQUIRE(item.content == R"(<pre style="white-space:pre-wrap;">&lt;nohtml&gt;</pre>)");
    REQUIRE(item.thumbnail == "file:///image.png");
    REQUIRE(item.datePublished == "2023-10-05T23:57:08Z");
    REQUIRE(item.categories.size() == 2);
    REQUIRE(item.categories.at(0) == "tag1");
}

TEST_CASE("Parse JSON 1.1 (custom example 2)", "[feedparsing]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "FeedJSON11CustomExample2.json");

    Poco::JSON::Parser parser;
    auto root = parser.parse(input);
    auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
    REQUIRE(rootObj != nullptr);

    auto feed = std::make_unique<ZapFR::Engine::FeedParserJSON11>("https://example.com");
    feed->setRootObj(rootObj);

    REQUIRE(feed->iconURL() == "");

    auto items = feed->items();
    REQUIRE(items.size() == 1);

    const auto& item = items.at(0);
    REQUIRE(item.author == "Overriding author");
}

TEST_CASE("Parse JSON 1.1 (custom example 3)", "[feedparsing]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "FeedJSON11CustomExample3.json");

    Poco::JSON::Parser parser;
    auto root = parser.parse(input);
    auto rootObj = root.extract<Poco::JSON::Object::Ptr>();
    REQUIRE(rootObj != nullptr);

    auto feed = std::make_unique<ZapFR::Engine::FeedParserJSON11>("https://example.com");
    feed->setRootObj(rootObj);

    auto items = feed->items();
    REQUIRE(items.size() == 1);

    const auto& item = items.at(0);
    REQUIRE(item.author == "");
}

std::string md5Hash(const std::string& input)
{
    Poco::MD5Engine md5;
    Poco::DigestOutputStream ds(md5);
    ds << input;
    ds.close();
    return Poco::DigestEngine::digestToHex(md5.digest());
}

TEST_CASE("Parse RSS 1.0 (Slashdot)", "[feedparsing]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "FeedRSS10Slashdot.xml");

    Poco::XML::DOMParser parser;
    Poco::AutoPtr<Poco::XML::Document> xmlDoc = parser.parseString(input);

    auto feed = std::make_unique<ZapFR::Engine::FeedParserRSS10>("https://example.com");
    feed->setXMLDoc(xmlDoc);

    REQUIRE(feed->guid() == "");
    REQUIRE(feed->title() == "Slashdot");
    REQUIRE(feed->subtitle() == "");
    REQUIRE(feed->link() == "https://slashdot.org/");
    REQUIRE(feed->description() == "News for nerds, stuff that matters");
    REQUIRE(feed->iconURL() == "https://a.fsdn.com/sd/topics/topicslashdot.gif");
    REQUIRE(feed->language() == "");
    REQUIRE(feed->copyright() == "");

    auto items = feed->items();
    REQUIRE(items.size() == 15);

    const auto& item = items.at(0);
    REQUIRE(item.title == "An Emergency Alert Test Will Sound On All US Cellphones, TVs and Radios On Wednesday");
    REQUIRE(item.link == "https://mobile.slashdot.org/story/23/10/03/0024216/"
                         "an-emergency-alert-test-will-sound-on-all-us-cellphones-tvs-and-radios-on-wednesday?utm_source=rss1.0mainlinkanon&utm_medium=feed");
    REQUIRE(item.guid == md5Hash(item.link));
    REQUIRE(item.content.starts_with("An anonymous reader quotes"));
    REQUIRE(item.author == "BeauHD");
    REQUIRE(item.commentsURL == "");
    REQUIRE(item.datePublished == "2023-10-03T13:00:00Z");
    REQUIRE(item.thumbnail == "");
}

TEST_CASE("Parse RSS 1.0 (custom example)", "[feedparsing]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "FeedRSS10CustomExample.xml");

    Poco::XML::DOMParser parser;
    Poco::AutoPtr<Poco::XML::Document> xmlDoc = parser.parseString(input);

    auto feed = std::make_unique<ZapFR::Engine::FeedParserRSS10>("https://example.com");
    feed->setXMLDoc(xmlDoc);

    REQUIRE(feed->iconURL() == "");

    auto items = feed->items();
    REQUIRE(items.size() == 3);

    const auto& item1 = items.at(0);
    REQUIRE(item1.content.starts_with("test content"));
    REQUIRE(item1.title == "title-as-guid");
    REQUIRE(item1.guid == md5Hash(item1.title));

    const auto& item2 = items.at(1);
    REQUIRE(item2.content == "content-as-guid");
    REQUIRE(item2.guid == md5Hash(item2.content));

    const auto& item3 = items.at(2);
    REQUIRE(!item3.guid.empty());
}

TEST_CASE("Parse RSS 2.0 (Hackernews)", "[feedparsing]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "FeedRSS20Hackernews.xml");

    Poco::XML::DOMParser parser;
    Poco::AutoPtr<Poco::XML::Document> xmlDoc = parser.parseString(input);

    auto feed = std::make_unique<ZapFR::Engine::FeedParserRSS20>("https://example.com");
    feed->setXMLDoc(xmlDoc);

    REQUIRE(feed->guid() == "");
    REQUIRE(feed->title() == "Hacker News");
    REQUIRE(feed->subtitle() == "");
    REQUIRE(feed->link() == "https://news.ycombinator.com/");
    REQUIRE(feed->description() == "Links for the intellectually curious, ranked by readers.");
    REQUIRE(feed->iconURL() == "");
    REQUIRE(feed->language() == "");
    REQUIRE(feed->copyright() == "");

    auto items = feed->items();
    REQUIRE(items.size() == 30);

    const auto& item = items.at(0);
    REQUIRE(item.title == "Austria rail operator OeBB unveils new night trains");
    REQUIRE(item.link == "https://techxplore.com/news/2023-09-austria-rail-oebb-unveils-night.html");
    REQUIRE(item.guid == md5Hash(item.link));
    REQUIRE(item.content == R"(<a href="https://news.ycombinator.com/item?id=37785300">Comments</a>)");
    REQUIRE(item.author == "");
    REQUIRE(item.commentsURL == "https://news.ycombinator.com/item?id=37785300");
    REQUIRE(item.datePublished == "2023-10-05T22:59:44Z");
    REQUIRE(item.thumbnail == "");
}

TEST_CASE("Parse RSS 2.0 (custom example)", "[feedparsing]")
{
    const auto& input = ZapFR::Tests::DataFetcher::fetch(ZapFR::Tests::DataFetcher::Source::Input, "FeedRSS20CustomExample.xml");

    Poco::XML::DOMParser parser;
    Poco::AutoPtr<Poco::XML::Document> xmlDoc = parser.parseString(input);

    auto feed = std::make_unique<ZapFR::Engine::FeedParserRSS20>("https://example.com");
    feed->setXMLDoc(xmlDoc);

    REQUIRE(feed->iconURL() == "");

    auto items = feed->items();
    REQUIRE(items.size() == 6);

    const auto& item1 = items.at(0);
    REQUIRE(item1.content.starts_with("test content"));
    REQUIRE(item1.title == "title-as-guid");
    REQUIRE(item1.guid == md5Hash(item1.title));

    const auto& item2 = items.at(1);
    REQUIRE(item2.content == "content-as-guid");
    REQUIRE(item2.guid == md5Hash(item2.content));

    const auto& item3 = items.at(2);
    REQUIRE(!item3.guid.empty());

    const auto& item4 = items.at(3);
    REQUIRE(item4.enclosures.size() == 1);
    REQUIRE(item4.enclosures.at(0).mimeType == "image/jpeg");
    REQUIRE(item4.enclosures.at(0).size == 100);
    REQUIRE(item4.enclosures.at(0).url == "https://example.com/file.jpg");

    const auto& item5 = items.at(4);
    REQUIRE(item5.enclosures.size() == 1);
    REQUIRE(item5.enclosures.at(0).mimeType == "application/x-bittorrent");
    REQUIRE(item5.enclosures.at(0).size == 100);
    REQUIRE(item5.enclosures.at(0).url == "magnet:link");

    const auto& item6 = items.at(5);
    REQUIRE(item6.guid == "http://example.com/guid");
    REQUIRE(item6.categories.size() == 2);
    REQUIRE(item6.categories.at(0) == "cat1");
}
