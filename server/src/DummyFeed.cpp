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

#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <sstream>

#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Stringifier.h>

#include "DummyFeed.h"

ZapFR::Server::DummyFeed::DummyFeed()
{
#ifdef DEBUG
    addEntry(
        "Ever present item",
        "<p>Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aliquam malesuada maximus sapien, a molestie felis sagittis vitae. Sed venenatis velit ac justo vehicula "
        "posuere. Praesent accumsan non metus quis gravida. Vivamus malesuada lacus ut volutpat vehicula. Cras tincidunt scelerisque placerat. Pellentesque dolor libero, "
        "blandit eget ligula id, blandit feugiat nisi. Nam mollis iaculis massa, id placerat nulla vulputate eget. Morbi facilisis, magna vestibulum maximus dictum, enim "
        "mauris elementum massa, sit amet pharetra eros sem sit amet mi. Aliquam sollicitudin odio ac libero blandit, id faucibus sem porttitor. Integer luctus ac odio ac "
        "pharetra. Morbi et pellentesque augue, vel laoreet tortor.</p><p>Maecenas pretium quam erat, vel finibus metus dapibus in. Nulla vitae justo quis orci fringilla "
        "dictum nec id justo. Quisque egestas consectetur justo id feugiat. In lobortis at arcu sit amet fermentum. Cras ornare eros sed erat dictum, ac tempus turpis "
        "vulputate. Pellentesque vestibulum molestie nisl venenatis blandit. Proin porta ut dui eget posuere. Sed venenatis at felis lobortis imperdiet. Phasellus sed dolor "
        "sit amet velit mollis placerat et vitae enim. Maecenas egestas imperdiet aliquam. Praesent aliquet quis nisi non vestibulum.</p>",
        "01518d4f-030e-44ed-ba87-8add1fd866cc", Poco::Timestamp());
#endif
}

ZapFR::Server::DummyFeed* ZapFR::Server::DummyFeed::getInstance()
{
    static DummyFeed instance{};
    return &instance;
}

std::string ZapFR::Server::DummyFeed::getRSS20()
{
    std::stringstream ss;

#ifdef DEBUG
    ss << R"(<?xml version="1.0" encoding="utf-8"?>)"
       << R"(<rss version="2.0">)"
       << R"(   <channel>)"
       << R"(       <title>ZapFeedReader RSS 2.0 dummy feed</title>)"
       << R"(       <link>https://zapfeedreader.zappatic.net</link>)"
       << R"(       <description>A dummy feed (RSS 2.0) for testing purposes</description>)"
       << R"(   </channel>)";

    for (const auto& entry : mEntries)
    {
        ss << "<item>";
        ss << fmt::format("<title>{}</title>", entry.title);
        ss << fmt::format("<description>{}</description>", entry.content);
        ss << fmt::format("<pubDate>{}</pubDate>", Poco::DateTimeFormatter::format(entry.datePublished, Poco::DateTimeFormat::RFC822_FORMAT));
        ss << fmt::format("<guid>{}</guid>", entry.guid);
        ss << "</item>";
    }

    ss << R"(</rss>)";
#endif

    return ss.str();
}

std::string ZapFR::Server::DummyFeed::getATOM10()
{
    std::stringstream ss;

#ifdef DEBUG
    ss << R"(<?xml version="1.0" encoding="utf-8"?>)"
       << R"(<feed xmlns="http://www.w3.org/2005/Atom">)"
       << R"(   <title>ZapFeedReader Atom 1.0 dummy feed</title>)"
       << R"(   <link>https://zapfeedreader.zappatic.net</link>)"
       << R"(   <id>https://zapfeedreader.zappatic.net</id>)"
       << fmt::format(R"(   <updated>{}</updated>)", Poco::DateTimeFormatter::format(Poco::DateTime(), Poco::DateTimeFormat::ISO8601_FORMAT));

    for (const auto& entry : mEntries)
    {
        ss << "<entry>";
        ss << fmt::format("<title>{}</title>", entry.title);
        ss << fmt::format("<summary>{}</summary>", entry.content);
        ss << fmt::format("<updated>{}</updated>", Poco::DateTimeFormatter::format(entry.datePublished, Poco::DateTimeFormat::ISO8601_FORMAT));
        ss << fmt::format("<id>{}</id>", entry.guid);
        ss << R"(<category term="Test category 1" />)";
        ss << R"(<category term="Test category 2" />)";
        ss << "</entry>";
    }

    ss << R"(</feed>)";
#endif

    return ss.str();
}

std::string ZapFR::Server::DummyFeed::getJSON11()
{
    Poco::JSON::Object o;

    o.set("version", "https://jsonfeed.org/version/1.1");
    o.set("title", "ZapFeedReader JSON 1.1 dummy feed");
    o.set("home_page_url", "https://zapfeedreader.zappatic.net");

    Poco::JSON::Array itemArr;
    for (const auto& entry : mEntries)
    {
        Poco::JSON::Object entryObj;
        entryObj.set("id", entry.guid);
        entryObj.set("title", entry.title);
        entryObj.set("content_html", entry.content);
        entryObj.set("date_published", Poco::DateTimeFormatter::format(entry.datePublished, Poco::DateTimeFormat::ISO8601_FORMAT));

        Poco::JSON::Array tagsArr;
        tagsArr.add("Category 1");
        tagsArr.add("Category 2");
        entryObj.set("tags", tagsArr);

        itemArr.add(entryObj);
    }
    o.set("items", itemArr);

    std::stringstream ss;
    Poco::JSON::Stringifier::stringify(o, ss);
    return ss.str();
}

void ZapFR::Server::DummyFeed::addEntry(const std::string& title, const std::string& content, const std::string& guid, const Poco::Timestamp& datePublished)
{
    mEntries.emplace_back(title, content, guid, datePublished);
}
