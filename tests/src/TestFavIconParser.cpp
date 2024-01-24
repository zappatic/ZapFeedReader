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

#include "ZapFR/feed_handling/FavIconParser.h"

TEST_CASE("Parse HTML with <link> in <head>", "[faviconparser]")
{
    auto tryParse = [](const std::string& html)
    {
        auto fp = ZapFR::Engine::FavIconParser();
        fp.parseString(html, "https://example.com");
        return fp.favIcon();
    };

    // valid xml + self closing link tag
    std::string html = R"(<!DOCTYPE html><html><head><title>Test</title><link rel="icon" href="https://example.com/found/favicon.ico" /></head><body></body></html>)";
    REQUIRE(tryParse(html) == "https://example.com/found/favicon.ico");

    // valid xml + self closing link tag + relative url
    html = R"(<!DOCTYPE html><html><head><title>Test</title><link rel="icon" href="/found/favicon.ico" /></head><body></body></html>)";
    REQUIRE(tryParse(html) == "https://example.com/found/favicon.ico");

    // valid xml + unclosed link tag
    html = R"(<!DOCTYPE html><html><head><title>Test</title><link rel="icon" href="https://example.com/found/favicon.ico"></head><body></body></html>)";
    REQUIRE(tryParse(html) == "https://example.com/found/favicon.ico");

    // invalid xml + self closing link tag
    html = R"(<!DOCTYPE html><><<<>>><html><head><title>Test</title><link rel="icon" href="https://example.com/found/favicon.ico" /></head><body></body></html>)";
    REQUIRE(tryParse(html) == "https://example.com/found/favicon.ico");

    // invalid xml + unclosed link tag
    html = R"(<!DOCTYPE html><><<<>>><html><head><title>Test</title><link rel="icon" href="https://example.com/found/favicon.ico"></head><body></body></html>)";
    REQUIRE(tryParse(html) == "https://example.com/found/favicon.ico");

    // no link tag, last resort /favicon.ico
    html = R"(<!DOCTYPE html><html><head><title>Test</title></head><body></body></html>)";
    REQUIRE(tryParse(html) == "https://example.com/favicon.ico");

    // youtube initial data containing avater image link
    html = "<!DOCTYPE html><html><head><script>var ytInitialData = "
           R"({
                "metadata": {
                    "channelMetadataRenderer": {
                    "title": "C++ Weekly With Jason Turner",
                    "description": "Making C++ Fun and Accessible.",
                    "rssUrl": "https://www.youtube.com/feeds/videos.xml?channel_id=UCxHAlbZQNFU2LgEtiqd2Maw",
                    "externalId": "UCxHAlbZQNFU2LgEtiqd2Maw",
                    "keywords": "C++ Programming Learning C++11 C++17 C++20 C++14",
                    "ownerUrls": ["http://www.youtube.com/@cppweekly"],
                    "avatar": {
                        "thumbnails": [
                        {
                            "url":
                            "https://yt3.googleusercontent.com/Hb_hQpJGonrMd9kihI-5gj8PBwVZvasJpdpWn9qxmhXo8UjZeJ9sCWN31FShYUUAGGLFofDLjg=s900-c-k-c0x00ffffff-no-rj",
                            "width": 900,
                            "height": 900
                        }
                        ]
                    },
                    "channelUrl": "https://www.youtube.com/channel/UCxHAlbZQNFU2LgEtiqd2Maw",
                    "isFamilySafe": true,
                    "androidDeepLink": "android-app://com.google.android.youtube/http/www.youtube.com/channel/UCxHAlbZQNFU2LgEtiqd2Maw",
                    "androidAppindexingLink": "android-app://com.google.android.youtube/http/www.youtube.com/channel/UCxHAlbZQNFU2LgEtiqd2Maw",
                    "iosAppindexingLink": "ios-app://544007664/vnd.youtube/www.youtube.com/channel/UCxHAlbZQNFU2LgEtiqd2Maw",
                    "vanityChannelUrl": "http://www.youtube.com/@cppweekly"
                    }
                }
                };
                )"
           "</script></head><body></body></html>";

    auto fp = ZapFR::Engine::FavIconParser();
    fp.parseString(html, "https://youtube.com/@cppweekly");
    REQUIRE(fp.favIcon() == "https://yt3.googleusercontent.com/Hb_hQpJGonrMd9kihI-5gj8PBwVZvasJpdpWn9qxmhXo8UjZeJ9sCWN31FShYUUAGGLFofDLjg=s900-c-k-c0x00ffffff-no-rj");

    // invalid youtube initial data
    html = R"(<!DOCTYPE html><html><head><script>var ytInitialData = { error };)";
    fp = ZapFR::Engine::FavIconParser();
    fp.parseString(html, "https://youtube.com/@cppweekly");
    REQUIRE(fp.favIcon() == "https://youtube.com/favicon.ico");

    fp = ZapFR::Engine::FavIconParser();
    fp.parseURL("https://zapfeedreader.zappatic.net", 0);
    REQUIRE(fp.favIcon() == "https://zapfeedreader.zappatic.net/favicon.ico");

    fp = ZapFR::Engine::FavIconParser();
    fp.parseURL("", 0);
    REQUIRE(fp.favIcon().empty());
}