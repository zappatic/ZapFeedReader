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

#include "ZapFR/Log.h"
#include "ZapFR/base/Folder.h"
#include "ZapFR/base/ScriptFolder.h"
#include "ZapFR/dummy/FeedDummy.h"
#include "ZapFR/dummy/PostDummy.h"
#include "ZapFR/dummy/SourceDummy.h"

TEST_CASE("Dummy Feed/Post/Source", "[dummy]")
{
    auto dummySource = ZapFR::Engine::SourceDummy(0);
    auto dummyFeed = ZapFR::Engine::FeedDummy(0, &dummySource);
    auto dummyPost = ZapFR::Engine::PostDummy(0);

    dummySource.setAssociatedDummyFeed(&dummyFeed);
    dummySource.setAssociatedDummyPost(&dummyPost);

    std::string logOutput{};
    dummyPost.setLogCallback([&](const std::string& s) { logOutput = s; });

    // source - feed related
    REQUIRE_THROWS(dummySource.getFeeds(ZapFR::Engine::Source::FetchInfo::None));
    REQUIRE_THROWS(dummySource.getFeed(1, ZapFR::Engine::Source::FetchInfo::None));
    REQUIRE_THROWS(dummySource.addFeed("", 0));
    REQUIRE_THROWS(dummySource.moveFeed(0, 1, 2));
    REQUIRE_THROWS(dummySource.removeFeed(1));

    // source - folder related
    REQUIRE_THROWS(dummySource.getFolders(0, ZapFR::Engine::Source::FetchInfo::None));
    REQUIRE_THROWS(dummySource.getFolder(1, ZapFR::Engine::Source::FetchInfo::None));
    REQUIRE_THROWS(dummySource.addFolder("", 0));
    REQUIRE_THROWS(dummySource.moveFolder(0, 1, 2));
    REQUIRE_THROWS(dummySource.removeFolder(1));

    // source - post related
    REQUIRE_THROWS(dummySource.getPosts(10, 1, false, false, "", 0, ZapFR::Engine::FlagColor::Gray));

    REQUIRE_THROWS(dummySource.markAsRead(1));
    REQUIRE(!dummyPost.isRead());
    dummySource.setPostsReadStatus(true, {});
    REQUIRE(dummyPost.isRead());
    dummySource.setPostsReadStatus(false, {});
    REQUIRE(!dummyPost.isRead());

    REQUIRE(dummyPost.flagColors().empty());
    dummySource.setPostsFlagStatus(true, {ZapFR::Engine::FlagColor::Blue}, {});
    REQUIRE(dummyPost.flagColors().contains(ZapFR::Engine::FlagColor::Blue));
    dummySource.setPostsFlagStatus(true, {ZapFR::Engine::FlagColor::Gray}, {});
    REQUIRE(dummyPost.flagColors().contains(ZapFR::Engine::FlagColor::Blue));
    dummySource.setPostsFlagStatus(false, {ZapFR::Engine::FlagColor::Blue}, {});
    REQUIRE(!dummyPost.flagColors().contains(ZapFR::Engine::FlagColor::Blue));

    logOutput = "";
    dummySource.assignPostsToScriptFolder(1, true, {});
    REQUIRE(logOutput == "Assigned to script folder with ID 1");
    dummySource.assignPostsToScriptFolder(1, false, {});
    REQUIRE(logOutput == "Unassigned from script folder with ID 1");

    // source - log related
    REQUIRE_THROWS(dummySource.getLogs(1, 1));

    // source - flag related
    REQUIRE_THROWS(dummySource.getUsedFlagColors());

    // source - category related
    REQUIRE_THROWS(dummySource.getCategories());

    // source - script folder related
    REQUIRE_THROWS(dummySource.getScriptFolders());
    REQUIRE_THROWS(dummySource.getScriptFolder(1, ZapFR::Engine::Source::FetchInfo::None));
    REQUIRE_THROWS(dummySource.addScriptFolder("", true, true));
    REQUIRE_THROWS(dummySource.removeScriptFolder(1));

    // source - script related
    REQUIRE_THROWS(dummySource.getScripts());
    REQUIRE_THROWS(dummySource.getScript(1, ZapFR::Engine::Source::FetchInfo::None));
    REQUIRE_THROWS(dummySource.removeScript(1));
    REQUIRE_THROWS(dummySource.addScript(ZapFR::Engine::Script::Type::Lua, "", true, {}, {}, ""));

    // source - source related
    REQUIRE_THROWS(dummySource.fetchStatistics());
    REQUIRE_THROWS(dummySource.fetchThumbnailData());
    REQUIRE_THROWS(dummySource.importOPML("", 0));
    REQUIRE_THROWS(dummySource.clearLogs());
    REQUIRE_THROWS(dummySource.getStatus());

    // feed
    REQUIRE_THROWS(dummyFeed.getPosts(10, 1, false, false, "", 0, ZapFR::Engine::FlagColor::Gray));
    REQUIRE_THROWS(dummyFeed.getPost(0));
    REQUIRE_THROWS(dummyFeed.refresh());
    REQUIRE_THROWS(dummyFeed.markAsRead(0));
    REQUIRE_THROWS(dummyFeed.getLogs(10, 1));
    REQUIRE_THROWS(dummyFeed.clearLogs());
    REQUIRE_THROWS(dummyFeed.updateProperties("", 1));
    REQUIRE_THROWS(dummyFeed.getCategories());

    // post
    Poco::SharedPtr<Poco::JSON::Object> o(new Poco::JSON::Object());
    o->set(ZapFR::Engine::JSON::Post::ID, 1);
    o->set(ZapFR::Engine::JSON::Post::IsRead, true);
    o->set(ZapFR::Engine::JSON::Post::FeedID, 2);
    o->set(ZapFR::Engine::JSON::Post::FeedTitle, "feed title");
    o->set(ZapFR::Engine::JSON::Post::FeedLink, "feed link");
    o->set(ZapFR::Engine::JSON::Post::Title, "post title");
    o->set(ZapFR::Engine::JSON::Post::Link, "post link");
    o->set(ZapFR::Engine::JSON::Post::Content, "content");
    o->set(ZapFR::Engine::JSON::Post::Author, "author");
    o->set(ZapFR::Engine::JSON::Post::CommentsURL, "comments url");
    o->set(ZapFR::Engine::JSON::Post::Guid, "guid");
    o->set(ZapFR::Engine::JSON::Post::DatePublished, "2024-01-01T00:00:00Z");
    o->set(ZapFR::Engine::JSON::Post::Thumbnail, "file:///thumb.jpg");
    o->set(ZapFR::Engine::JSON::Post::FlagColors, "red,green,blue");

    Poco::SharedPtr<Poco::JSON::Array> enclosuresArr(new Poco::JSON::Array());
    Poco::SharedPtr<Poco::JSON::Object> enclosureObj(new Poco::JSON::Object());
    enclosureObj->set(ZapFR::Engine::JSON::Post::EnclosureURL, "url");
    enclosureObj->set(ZapFR::Engine::JSON::Post::EnclosureMimeType, "mimetype");
    enclosureObj->set(ZapFR::Engine::JSON::Post::EnclosureSize, 100);
    enclosuresArr->add(enclosureObj);
    o->set(ZapFR::Engine::JSON::Post::Enclosures, enclosuresArr);

    Poco::SharedPtr<Poco::JSON::Array> categoryArr(new Poco::JSON::Array());
    Poco::SharedPtr<Poco::JSON::Object> categoryObj(new Poco::JSON::Object());
    categoryObj->set(ZapFR::Engine::JSON::Post::CategoryID, 1);
    categoryObj->set(ZapFR::Engine::JSON::Post::CategoryTitle, "cat1");
    categoryArr->add(categoryObj);
    o->set(ZapFR::Engine::JSON::Post::Categories, categoryArr);

    auto p = ZapFR::Engine::PostDummy::createFromJSON(o);
    REQUIRE(p->id() == 1);
    REQUIRE(p->isRead() == 1);
    REQUIRE(p->feedID() == 2);
    REQUIRE(p->feedTitle() == "feed title");
    REQUIRE(p->feedLink() == "feed link");
    REQUIRE(p->title() == "post title");
    REQUIRE(p->link() == "post link");
    REQUIRE(p->content() == "content");
    REQUIRE(p->author() == "author");
    REQUIRE(p->commentsURL() == "comments url");
    REQUIRE(p->guid() == "guid");
    REQUIRE(p->datePublished() == "2024-01-01T00:00:00Z");
    REQUIRE(p->thumbnail() == "file:///thumb.jpg");
    REQUIRE(p->flagColors().size() == 3);
    REQUIRE(p->flagColors().contains(ZapFR::Engine::FlagColor::Red));
    REQUIRE(p->flagColors().contains(ZapFR::Engine::FlagColor::Green));
    REQUIRE(p->flagColors().contains(ZapFR::Engine::FlagColor::Blue));
    REQUIRE(p->categories().size() == 1);
    REQUIRE(p->categories().at(0).id == 1);
    REQUIRE(p->categories().at(0).title == "cat1");
}
