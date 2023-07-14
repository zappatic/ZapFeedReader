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

#include "FeedLocal.h"
#include "Database.h"
#include "FeedFetcher.h"

using namespace Poco::Data::Keywords;

ZapFR::Engine::FeedLocal::FeedLocal(uint64_t id) : Feed(id)
{
}

std::vector<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::FeedLocal::getPosts(uint64_t perPage, uint64_t page)
{
    std::vector<std::unique_ptr<Post>> posts;

    auto offset = perPage * (page - 1);

    uint64_t id{0};
    std::string title{""};
    std::string link{""};
    std::string description{""};
    std::string author{""};
    std::string commentsURL{""};
    std::string enclosureURL{""};
    std::string enclosureLength{""};
    std::string enclosureMimeType{""};
    std::string guid{""};
    bool guidIsPermalink{false};
    std::string datePublished{""};
    std::string sourceURL{""};
    std::string sourceTitle{""};

    Poco::Data::Statement selectStmt(*(msDatabase->session()));
    selectStmt << "SELECT id"
                  ",title"
                  ",link"
                  ",description"
                  ",author"
                  ",commentsURL"
                  ",enclosureURL"
                  ",enclosureLength"
                  ",enclosureMimeType"
                  ",guid"
                  ",guidIsPermalink"
                  ",datePublished"
                  ",sourceURL"
                  ",sourceTitle"
                  " FROM posts"
                  " WHERE feedID=?"
                  " ORDER BY datePublished DESC"
                  " LIMIT ? OFFSET ?",
        use(mID), use(perPage), use(offset), into(id), into(title), into(link), into(description), into(author), into(commentsURL), into(enclosureURL), into(enclosureLength),
        into(enclosureMimeType), into(guid), into(guidIsPermalink), into(datePublished), into(sourceURL), into(sourceTitle), range(0, 1);

    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            auto p = std::make_unique<Post>(id);
            p->setFeedID(mID);
            p->setTitle(title);
            p->setLink(link);
            p->setDescription(description);
            p->setAuthor(author);
            p->setCommentsURL(commentsURL);
            p->setEnclosureURL(enclosureURL);
            p->setEnclosureLength(enclosureLength);
            p->setEnclosureMimeType(enclosureMimeType);
            p->setGuid(guid);
            p->setGuidIsPermalink(guidIsPermalink);
            p->setDatePublished(datePublished);
            p->setSourceURL(sourceURL);
            p->setSourceTitle(sourceTitle);
            posts.emplace_back(std::move(p));
        }
    }
    return posts;
}

std::optional<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::FeedLocal::getPost(uint64_t postID)
{
    uint64_t id{0};
    std::string title{""};
    std::string link{""};
    std::string description{""};
    std::string author{""};
    std::string commentsURL{""};
    std::string enclosureURL{""};
    std::string enclosureLength{""};
    std::string enclosureMimeType{""};
    std::string guid{""};
    bool guidIsPermalink{false};
    std::string datePublished{""};
    std::string sourceURL{""};
    std::string sourceTitle{""};

    Poco::Data::Statement selectStmt(*(msDatabase->session()));
    selectStmt << "SELECT id"
                  ",title"
                  ",link"
                  ",description"
                  ",author"
                  ",commentsURL"
                  ",enclosureURL"
                  ",enclosureLength"
                  ",enclosureMimeType"
                  ",guid"
                  ",guidIsPermalink"
                  ",datePublished"
                  ",sourceURL"
                  ",sourceTitle"
                  " FROM posts"
                  " WHERE feedID=?"
                  "   AND id=?",
        use(mID), use(postID), into(id), into(title), into(link), into(description), into(author), into(commentsURL), into(enclosureURL), into(enclosureLength),
        into(enclosureMimeType), into(guid), into(guidIsPermalink), into(datePublished), into(sourceURL), into(sourceTitle), now;

    auto rs = Poco::Data::RecordSet(selectStmt);
    if (rs.rowCount() == 1)
    {
        auto p = std::make_unique<Post>(id);
        p->setFeedID(mID);
        p->setTitle(title);
        p->setLink(link);
        p->setDescription(description);
        p->setAuthor(author);
        p->setCommentsURL(commentsURL);
        p->setEnclosureURL(enclosureURL);
        p->setEnclosureLength(enclosureLength);
        p->setEnclosureMimeType(enclosureMimeType);
        p->setGuid(guid);
        p->setGuidIsPermalink(guidIsPermalink);
        p->setDatePublished(datePublished);
        p->setSourceURL(sourceURL);
        p->setSourceTitle(sourceTitle);
        return p;
    }

    return {};
}

bool ZapFR::Engine::FeedLocal::fetchData()
{
    if (!mDataFetched)
    {
        Poco::Data::Statement selectStmt(*(msDatabase->session()));
        selectStmt << "SELECT url"
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
                      " WHERE id=?",
            use(mID), into(mURL), into(mFolderHierarchy), into(mGuid), into(mTitle), into(mSubtitle), into(mLink), into(mDescription), into(mLanguage), into(mCopyright),
            into(mLastChecked), into(mSortOrder), now;

        mDataFetched = true;
        auto rs = Poco::Data::RecordSet(selectStmt);
        return (rs.rowCount() == 1);
    }
    return true;
}

void ZapFR::Engine::FeedLocal::refresh()
{
    fetchData();

    FeedFetcher ff;
    auto parsedFeed = ff.parse(mURL);
    processItems(parsedFeed.get());
}

void ZapFR::Engine::FeedLocal::processItems(FeedParser* parsedFeed)
{
    for (const auto& item : parsedFeed->items())
    {
        auto isPermaLink = item.guidIsPermalink ? 1 : 0;
        auto guid = item.guid;

        // see if it already exists
        Poco::Data::Statement selectStmt(*(msDatabase->session()));
        selectStmt << "SELECT id FROM posts WHERE feedID=? AND guid=?", use(mID), useRef(guid), now;
        selectStmt.execute();
        auto rs = Poco::Data::RecordSet(selectStmt);
        if (rs.rowCount() == 1)
        {
            Poco::Data::Statement updateStmt(*(msDatabase->session()));
            updateStmt << "UPDATE posts SET"
                          " title=?"
                          ",link=?"
                          ",description=?"
                          ",author=?"
                          ",commentsURL=?"
                          ",enclosureURL=?"
                          ",enclosureLength=?"
                          ",enclosureMimeType=?"
                          ",guid=?"
                          ",guidIsPermalink=?"
                          ",datePublished=?"
                          ",sourceURL=?"
                          ",sourceTitle=?"
                          " WHERE feedID=? AND guid=?",
                useRef(item.title), useRef(item.link), useRef(item.description), useRef(item.author), useRef(item.commentsURL), useRef(item.enclosureURL),
                useRef(item.enclosureLength), useRef(item.enclosureMimeType), useRef(item.guid), use(isPermaLink), useRef(item.datePublished), useRef(item.sourceURL),
                useRef(item.sourceTitle), use(mID), useRef(guid);
            updateStmt.execute();
        }
        else
        {
            Poco::Data::Statement insertStmt(*(msDatabase->session()));
            insertStmt << "INSERT INTO posts ("
                          " feedID"
                          ",title"
                          ",link"
                          ",description"
                          ",author"
                          ",commentsURL"
                          ",enclosureURL"
                          ",enclosureLength"
                          ",enclosureMimeType"
                          ",guid"
                          ",guidIsPermalink"
                          ",datePublished"
                          ",sourceURL"
                          ",sourceTitle"
                          ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                use(mID), useRef(item.title), useRef(item.link), useRef(item.description), useRef(item.author), useRef(item.commentsURL), useRef(item.enclosureURL),
                useRef(item.enclosureLength), useRef(item.enclosureMimeType), useRef(item.guid), use(isPermaLink), useRef(item.datePublished), useRef(item.sourceURL),
                useRef(item.sourceTitle);
            insertStmt.execute();
        }
    }
}
