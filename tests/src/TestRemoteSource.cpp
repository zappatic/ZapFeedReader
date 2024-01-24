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

#include "ZapFR/remote/SourceRemote.h"

TEST_CASE("Get sources", "[remote-source]")
{
    auto sources = ZapFR::Engine::Source::getSources({});
    REQUIRE(sources.size() == 2);
    sources = ZapFR::Engine::Source::getSources(ZapFR::Engine::ServerIdentifier::Remote);
    REQUIRE(sources.size() == 1);
}

TEST_CASE("Get non-existent source", "[remote-source]")
{
    auto source = ZapFR::Engine::Source::getSource(100);
    REQUIRE(!source.has_value());
}

TEST_CASE("Create source of invalid type", "[remote-source]")
{
    REQUIRE_THROWS(ZapFR::Engine::Source::createSourceInstance(100, "invalid.source.type"));
}

TEST_CASE("Update source", "[remote-source]")
{
    auto source = ZapFR::Engine::Source::getSource(2);
    REQUIRE(source.has_value());
    auto originalConfigData = source.value()->configData();
    source.value()->update("Updated title", originalConfigData);

    source = ZapFR::Engine::Source::getSource(2);
    REQUIRE(source.has_value());
    REQUIRE(source.value()->title() == "Updated title");
    REQUIRE(source.value()->configData() == originalConfigData);

    source.value()->updateLastError("error!");
    source = ZapFR::Engine::Source::getSource(2);
    REQUIRE(source.has_value());
    REQUIRE(source.value()->lastError() == "error!");

    auto sources = ZapFR::Engine::Source::getSources(ZapFR::Engine::ServerIdentifier::Remote);
    REQUIRE(sources.size() == 1);
    REQUIRE(sources.at(0)->lastError() == "error!");

    source.value()->updateLastError("");
}

TEST_CASE("Remove source", "[remote-source]")
{
    ZapFR::Engine::Source::removeSource(1); // this should not delete the local source as that's forbidden
    auto sources = ZapFR::Engine::Source::getSources({});
    REQUIRE(sources.size() == 2);

    auto newSource = ZapFR::Engine::Source::create(ZapFR::Engine::ServerIdentifier::Remote, "Source to be deleted immediately", "");
    REQUIRE(newSource.has_value());

    sources = ZapFR::Engine::Source::getSources({});
    REQUIRE(sources.size() == 3);

    ZapFR::Engine::Source::removeSource(newSource.value()->id());

    sources = ZapFR::Engine::Source::getSources({});
    REQUIRE(sources.size() == 2);
}

TEST_CASE("Get source status", "[remote-source]")
{
    auto source = ZapFR::Engine::Source::getSource(2);
    REQUIRE(source.has_value());

    auto status = source.value()->getStatus();
    REQUIRE(status.has(ZapFR::Engine::JSON::SourceStatus::FeedErrors));
    REQUIRE(status.isArray(ZapFR::Engine::JSON::SourceStatus::FeedErrors));
    REQUIRE(status.getArray(ZapFR::Engine::JSON::SourceStatus::FeedErrors)->size() == 0);

    REQUIRE(status.has(ZapFR::Engine::JSON::SourceStatus::HighestPostID));
    REQUIRE(status.getValue<uint64_t>(ZapFR::Engine::JSON::SourceStatus::HighestPostID) == 0);

    REQUIRE(status.has(ZapFR::Engine::JSON::SourceStatus::UnreadCounts));
    REQUIRE(status.isArray(ZapFR::Engine::JSON::SourceStatus::UnreadCounts));
    REQUIRE(status.getArray(ZapFR::Engine::JSON::SourceStatus::UnreadCounts)->size() == 0);
}
