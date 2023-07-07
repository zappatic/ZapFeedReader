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

#include "SourceLocal.h"
#include "Database.h"
#include "FeedFetcher.h"
#include "FeedLocal.h"

using namespace Poco::Data::Keywords;

ZapFR::Engine::SourceLocal::SourceLocal(uint64_t id) : Source(id)
{
}

std::vector<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceLocal::getFeeds()
{
    std::vector<std::unique_ptr<Feed>> feeds;

    uint64_t id;
    std::string url;
    std::string folderHierarchy;
    std::string guid;
    std::string title;
    std::string subtitle;
    std::string link;
    std::string description;
    std::string language;
    std::string copyright;
    std::string lastChecked;
    uint64_t sortOrder;

    Poco::Data::Statement selectStmt(*(msDatabase->session()));
    selectStmt << "SELECT id"
                  ",url"
                  ",folderHierarchy"
                  ",guid"
                  ",title"
                  ",subtitle"
                  ",link"
                  ",description"
                  ",language"
                  ",copyright"
                  ",lastChecked"
                  ",sortOrder"
                  " FROM feeds"
                  " ORDER BY sortOrder ASC",
        into(id), into(url), into(folderHierarchy), into(guid), into(title), into(subtitle), into(link), into(description), into(language), into(copyright), into(lastChecked),
        into(sortOrder), range(0, 1);

    while (!selectStmt.done())
    {
        selectStmt.execute();

        auto f = std::make_unique<FeedLocal>(id);
        f->setURL(url);
        f->setFolderHierarchy(folderHierarchy);
        f->setGuid(guid);
        f->setTitle(title);
        f->setSubtitle(subtitle);
        f->setLink(link);
        f->setDescription(description);
        f->setLanguage(language);
        f->setCopyright(copyright);
        f->setLastChecked(lastChecked);
        f->setSortOrder(sortOrder);
        feeds.emplace_back(std::move(f));
    }

    return feeds;
}

std::optional<std::unique_ptr<ZapFR::Engine::Feed>> ZapFR::Engine::SourceLocal::getFeed(uint64_t /*feedID*/)
{
    return {};
}

void ZapFR::Engine::SourceLocal::addFeed(const std::string& url)
{
    FeedFetcher ff(msDatabase);
    ff.subscribeToFeed(url);
}
