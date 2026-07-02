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

#include <Poco/Data/RecordSet.h>

#include "ZapFR/Database.h"
#include "ZapFR/Helpers.h"
#include "ZapFR/local/PostLocal.h"

using namespace Poco::Data::Keywords;

ZapFR::Engine::PostLocal::PostLocal(uint64_t id) : Post(id)
{
}

void ZapFR::Engine::PostLocal::markFlagged(FlagColor flagColor)
{
    if (flagColor == FlagColor::Gray)
    {
        return;
    }

    markUnflagged(flagColor);

    auto fc = Flag::idForFlagColor(flagColor);
    DBStatement insertStmt(*(Database::getInstance()->session()));
    insertStmt << "INSERT INTO flags (postID, flagID) VALUES (?, ?)", use(mID), use(fc), now;
}

void ZapFR::Engine::PostLocal::markUnflagged(FlagColor flagColor)
{
    auto fc = Flag::idForFlagColor(flagColor);
    DBStatement deleteStmt(*(Database::getInstance()->session()));
    deleteStmt << "DELETE FROM flags WHERE postID=? AND flagID=?", use(mID), use(fc), now;
}

void ZapFR::Engine::PostLocal::markAsRead()
{
    DBStatement deleteStmt(*(Database::getInstance()->session()));
    deleteStmt << "DELETE FROM post_read WHERE postID = ? AND feedID = ?", use(mID), use(mFeedID), now;
}

void ZapFR::Engine::PostLocal::markAsUnread()
{
    DBStatement insertStmt(*(Database::getInstance()->session()));
    insertStmt << "INSERT OR IGNORE INTO post_read (postID, feedID) VALUES (?, ?)", use(mID), use(mFeedID), now;
}

void ZapFR::Engine::PostLocal::assignToScriptFolder(uint64_t scriptFolderID)
{
    unassignFromScriptFolder(scriptFolderID);
    DBStatement insertStmt(*(Database::getInstance()->session()));
    insertStmt << "INSERT INTO scriptfolder_posts (scriptFolderID, postID) VALUES (?, ?)", use(scriptFolderID), use(mID), now;
}

void ZapFR::Engine::PostLocal::unassignFromScriptFolder(uint64_t scriptFolderID)
{
    DBStatement deleteStmt(*(Database::getInstance()->session()));
    deleteStmt << "DELETE FROM scriptfolder_posts WHERE postID=? AND scriptFolderID=?", use(mID), use(scriptFolderID), now;
}

std::vector<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::PostLocal::queryMultiple(const std::vector<std::string>& whereClause, const std::string& orderClause,
                                                                                          const std::string& limitClause,
                                                                                          const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings)
{
    std::vector<std::unique_ptr<Post>> posts;

    uint64_t id{0};
    uint64_t feedID{0};
    bool isRead{false};
    std::string title{""};
    std::string link{""};
    std::string content{""};
    std::string author{""};
    std::string commentsURL{""};
    std::string guid{""};
    std::string datePublished{""};
    std::string thumbnail{""};
    std::string feedTitle{""};
    std::string feedLink{""};

    DBStatement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT posts.id"
          ",posts.feedID"
          ",CASE WHEN post_read.id IS NULL THEN 1 ELSE 0 END AS isRead"
          ",posts.title"
          ",posts.link"
          ",posts.content"
          ",posts.author"
          ",posts.commentsURL"
          ",posts.guid"
          ",posts.datePublished"
          ",posts.thumbnail"
          ",feeds.title"
          ",feeds.link"
          " FROM posts"
          " LEFT JOIN feeds ON feeds.id = posts.feedID"
          " LEFT JOIN post_read ON post_read.feedID = posts.feedID AND post_read.postID = posts.id";
    if (!whereClause.empty())
    {
        ss << " WHERE ";
        ss << Helpers::joinString(whereClause, " AND ");
    }
    ss << " " << orderClause << " " << limitClause;

    auto sql = ss.str();

    selectStmt << sql, range(0, 1);

    for (const auto& binding : bindings)
    {
        selectStmt.addBind(binding);
    }

    selectStmt.addExtract(into(id));
    selectStmt.addExtract(into(feedID));
    selectStmt.addExtract(into(isRead));
    selectStmt.addExtract(into(title));
    selectStmt.addExtract(into(link));
    selectStmt.addExtract(into(content));
    selectStmt.addExtract(into(author));
    selectStmt.addExtract(into(commentsURL));
    selectStmt.addExtract(into(guid));
    selectStmt.addExtract(into(datePublished));
    selectStmt.addExtract(into(thumbnail));
    selectStmt.addExtract(into(feedTitle));
    selectStmt.addExtract(into(feedLink));

    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            auto p = std::make_unique<PostLocal>(id);
            p->setIsRead(isRead);
            p->setFeedID(feedID);
            p->setFeedTitle(feedTitle);
            p->setFeedLink(feedLink);
            p->setTitle(title);
            p->setLink(link);
            p->setContent(content);
            p->setAuthor(author);
            p->setCommentsURL(commentsURL);
            p->setGuid(guid);
            p->setDatePublished(datePublished);
            p->setThumbnail(thumbnail);

            // query flags
            std::unordered_set<FlagColor> flags;
            uint8_t flagID{0};
            DBStatement selectFlagsStmt(*(Database::getInstance()->session()));
            selectFlagsStmt << "SELECT DISTINCT(flagID) FROM flags WHERE postID=?", use(id), into(flagID), range(0, 1);
            while (!selectFlagsStmt.done())
            {
                if (selectFlagsStmt.execute() > 0)
                {
                    flags.insert(Flag::flagColorForID(flagID));
                }
            }
            p->setFlagColors(flags);

            // query enclosures
            Enclosure e;
            DBStatement selectEnclosuresStmt(*(Database::getInstance()->session()));
            selectEnclosuresStmt << "SELECT url,size,mimetype FROM post_enclosures WHERE postID=?", use(id), into(e.url), into(e.size), into(e.mimeType), range(0, 1);
            while (!selectEnclosuresStmt.done())
            {
                if (selectEnclosuresStmt.execute() > 0)
                {
                    p->addEnclosure(e);
                }
            }

            queryCategories(p.get());

            posts.emplace_back(std::move(p));
        }
    }
    return posts;
}

std::vector<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::PostLocal::queryMultipleUnreadFirst(const std::vector<std::string>& whereClause,
                                                                                                     const std::vector<BindingFactory>& bindingFactories, uint64_t perPage,
                                                                                                     uint64_t offset, bool useDatePublishedIndex)
{
    std::vector<std::unique_ptr<Post>> posts;

    uint64_t id{0}, feedID{0};
    bool isRead{false};
    std::string title{""}, link{""}, content{""}, author{""}, commentsURL{""}, guid{""}, datePublished{""}, thumbnail{""}, feedTitle{""}, feedLink{""};

    DBStatement selectStmt(*(Database::getInstance()->session()));

    std::string whereSql;
    if (!whereClause.empty())
    {
        whereSql = " AND " + Helpers::joinString(whereClause, " AND ");
    }

    const char* branchColumnsFmt = "posts.id, posts.feedID, %d AS isRead, posts.title, posts.link, posts.content, posts.author,"
                                   " posts.commentsURL, posts.guid, posts.datePublished, posts.thumbnail, feeds.title, feeds.link";
    std::string indexedBy{""};
    if (useDatePublishedIndex)
    {
        indexedBy = " INDEXED BY IX_posts_datePublished ";
    }

    std::stringstream ss;
    ss << "SELECT * FROM ("
       << " SELECT " << Poco::format(branchColumnsFmt, 0) << " FROM posts " << indexedBy << "  LEFT JOIN feeds ON feeds.id = posts.feedID"
       << " WHERE EXISTS (SELECT 1 FROM post_read WHERE post_read.feedID = posts.feedID AND post_read.postID = posts.id)" << whereSql << " ORDER BY posts.datePublished DESC"
       << " LIMIT -1" // <-- forces SQLite to honor this ORDER BY
       << ")"
       << " UNION ALL "
       << "SELECT * FROM ("
       << " SELECT " << Poco::format(branchColumnsFmt, 1) << " FROM posts " << indexedBy << "LEFT JOIN feeds ON feeds.id = posts.feedID"
       << " WHERE NOT EXISTS (SELECT 1 FROM post_read WHERE post_read.feedID = posts.feedID AND post_read.postID = posts.id)" << whereSql
       << " ORDER BY posts.datePublished DESC"
       << " LIMIT -1"
       << ")"
       << " LIMIT ? OFFSET ?";

    auto sql = ss.str();
    selectStmt << sql, range(0, 1);

    for (const auto& factory : bindingFactories)
    {
        selectStmt.addBind(factory()); // branch 1 (unread)
    }
    for (const auto& factory : bindingFactories)
    {
        selectStmt.addBind(factory()); // branch 2 (read)
    }
    selectStmt.addBind(use(perPage, "perPage"));
    selectStmt.addBind(use(offset, "offset"));

    selectStmt.addExtract(into(id));
    selectStmt.addExtract(into(feedID));
    selectStmt.addExtract(into(isRead));
    selectStmt.addExtract(into(title));
    selectStmt.addExtract(into(link));
    selectStmt.addExtract(into(content));
    selectStmt.addExtract(into(author));
    selectStmt.addExtract(into(commentsURL));
    selectStmt.addExtract(into(guid));
    selectStmt.addExtract(into(datePublished));
    selectStmt.addExtract(into(thumbnail));
    selectStmt.addExtract(into(feedTitle));
    selectStmt.addExtract(into(feedLink));

    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            auto p = std::make_unique<PostLocal>(id);
            p->setIsRead(isRead);
            p->setFeedID(feedID);
            p->setFeedTitle(feedTitle);
            p->setFeedLink(feedLink);
            p->setTitle(title);
            p->setLink(link);
            p->setContent(content);
            p->setAuthor(author);
            p->setCommentsURL(commentsURL);
            p->setGuid(guid);
            p->setDatePublished(datePublished);
            p->setThumbnail(thumbnail);

            // query flags
            std::unordered_set<FlagColor> flags;
            uint8_t flagID{0};
            DBStatement selectFlagsStmt(*(Database::getInstance()->session()));
            selectFlagsStmt << "SELECT DISTINCT(flagID) FROM flags WHERE postID=?", use(id), into(flagID), range(0, 1);
            while (!selectFlagsStmt.done())
            {
                if (selectFlagsStmt.execute() > 0)
                {
                    flags.insert(Flag::flagColorForID(flagID));
                }
            }
            p->setFlagColors(flags);

            // query enclosures
            Enclosure e;
            DBStatement selectEnclosuresStmt(*(Database::getInstance()->session()));
            selectEnclosuresStmt << "SELECT url,size,mimetype FROM post_enclosures WHERE postID=?", use(id), into(e.url), into(e.size), into(e.mimeType), range(0, 1);
            while (!selectEnclosuresStmt.done())
            {
                if (selectEnclosuresStmt.execute() > 0)
                {
                    p->addEnclosure(e);
                }
            }

            queryCategories(p.get());

            posts.emplace_back(std::move(p));
        }
    }

    return posts;
}

std::optional<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::PostLocal::querySingle(const std::vector<std::string>& whereClause,
                                                                                          const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings)
{
    uint64_t id{0};
    uint64_t feedID{0};
    bool isRead{false};
    std::string title{""};
    std::string link{""};
    std::string content{""};
    std::string author{""};
    std::string commentsURL{""};
    std::string guid{""};
    std::string datePublished{""};
    std::string thumbnail{""};
    std::string feedTitle{""};
    std::string feedLink{""};

    DBStatement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT posts.id"
          ",posts.feedID"
          ",CASE WHEN post_read.id IS NULL THEN 1 ELSE 0 END AS isRead"
          ",posts.title"
          ",posts.link"
          ",posts.content"
          ",posts.author"
          ",posts.commentsURL"
          ",posts.guid"
          ",posts.datePublished"
          ",posts.thumbnail"
          ",feeds.title"
          ",feeds.link"
          " FROM posts"
          " LEFT JOIN feeds ON feeds.id = posts.feedID"
          " LEFT JOIN post_read ON post_read.feedID = posts.feedID AND post_read.postID = posts.id";
    if (!whereClause.empty())
    {
        ss << " WHERE ";
        ss << Helpers::joinString(whereClause, " AND ");
    }
    ss << " LIMIT 1";

    auto sql = ss.str();

    selectStmt << sql;

    for (const auto& binding : bindings)
    {
        selectStmt.addBind(binding);
    }

    selectStmt.addExtract(into(id));
    selectStmt.addExtract(into(feedID));
    selectStmt.addExtract(into(isRead));
    selectStmt.addExtract(into(title));
    selectStmt.addExtract(into(link));
    selectStmt.addExtract(into(content));
    selectStmt.addExtract(into(author));
    selectStmt.addExtract(into(commentsURL));
    selectStmt.addExtract(into(guid));
    selectStmt.addExtract(into(datePublished));
    selectStmt.addExtract(into(thumbnail));
    selectStmt.addExtract(into(feedTitle));
    selectStmt.addExtract(into(feedLink));

    selectStmt.execute();

    auto rs = Poco::Data::RecordSet(selectStmt);
    if (rs.rowCount() == 1)
    {
        auto p = std::make_unique<PostLocal>(id);
        p->setFeedID(feedID);
        p->setFeedTitle(feedTitle);
        p->setFeedLink(feedLink);
        p->setIsRead(isRead);
        p->setTitle(title);
        p->setLink(link);
        p->setContent(content);
        p->setAuthor(author);
        p->setCommentsURL(commentsURL);
        p->setGuid(guid);
        p->setDatePublished(datePublished);
        p->setThumbnail(thumbnail);

        // query flags
        std::unordered_set<FlagColor> flags;
        uint8_t flagID{0};
        DBStatement selectFlagsStmt(*(Database::getInstance()->session()));
        selectFlagsStmt << "SELECT DISTINCT(flagID) FROM flags WHERE postID=?", use(id), into(flagID), range(0, 1);
        while (!selectFlagsStmt.done())
        {
            if (selectFlagsStmt.execute() > 0)
            {
                flags.insert(Flag::flagColorForID(flagID));
            }
        }
        p->setFlagColors(flags);

        // query enclosures
        Enclosure e;
        DBStatement selectEnclosuresStmt(*(Database::getInstance()->session()));
        selectEnclosuresStmt << "SELECT url,size,mimetype FROM post_enclosures WHERE postID=?", use(id), into(e.url), into(e.size), into(e.mimeType), range(0, 1);
        while (!selectEnclosuresStmt.done())
        {
            if (selectEnclosuresStmt.execute() > 0)
            {
                p->addEnclosure(e);
            }
        }

        queryCategories(p.get());

        return p;
    }

    return {};
}

uint64_t ZapFR::Engine::PostLocal::queryCount(const std::vector<std::string>& whereClause, const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings)
{
    uint64_t postCount;
    DBStatement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT COUNT(*) FROM ("
          "SELECT posts.*"
          ",CASE WHEN EXISTS ("
          "SELECT 1 FROM post_read"
          " WHERE post_read.feedID = posts.feedID AND post_read.postID = posts.id"
          ") THEN 0 ELSE 1 END AS isRead"
          " FROM posts"
          ") AS posts";

    if (!whereClause.empty())
    {
        ss << " WHERE ";
        ss << Helpers::joinString(whereClause, " AND ");
    }

    auto sql = ss.str();
    selectStmt << sql, into(postCount);

    for (const auto& binding : bindings)
    {
        selectStmt.addBind(binding);
    }

    selectStmt.execute();

    return postCount;
}

void ZapFR::Engine::PostLocal::queryCategories(Post* post)
{
    auto postID = post->id();
    Category cat;
    DBStatement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT post_categories.categoryID"
                  ",categories.title"
                  " FROM post_categories"
                  " LEFT JOIN categories ON categories.id = post_categories.categoryID"
                  " WHERE post_categories.postID=?",
        use(postID), into(cat.id), into(cat.title), range(0, 1);
    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            post->addCategory(cat);
        }
    }
}

void ZapFR::Engine::PostLocal::update(const std::string& title, const std::string& link, const std::string& content, const std::string& author, const std::string& commentsURL,
                                      const std::string& guid, const std::string& datePublished, const std::string& thumbnail, const std::vector<Enclosure>& enclosures,
                                      const std::vector<std::string>& categories)
{
    Poco::Nullable<std::string> thumbnailNullable;
    if (!thumbnail.empty())
    {
        thumbnailNullable = thumbnail;
    }

    DBStatement updateStmt(*(Database::getInstance()->session()));
    updateStmt << "UPDATE posts SET"
                  " title=?"
                  ",link=?"
                  ",content=?"
                  ",author=?"
                  ",commentsURL=?"
                  ",guid=?"
                  ",datePublished=?"
                  ",thumbnail=?"
                  " WHERE id=?",
        useRef(title), useRef(link), useRef(content), useRef(author), useRef(commentsURL), useRef(guid), useRef(datePublished), useRef(thumbnailNullable), use(mID);
    updateStmt.execute();

    replaceEnclosures(mID, enclosures);
    replaceCategories(mID, mFeedID, categories);
}

std::unique_ptr<ZapFR::Engine::Post> ZapFR::Engine::PostLocal::create(uint64_t feedID, const std::string& feedTitle, const std::string& title, const std::string& link,
                                                                      const std::string& content, const std::string& author, const std::string& commentsURL,
                                                                      const std::string& guid, const std::string& datePublished, const std::string& thumbnail,
                                                                      const std::vector<Enclosure>& enclosures, const std::vector<std::string>& categories)
{
    Poco::Nullable<std::string> thumbnailNullable;
    if (!thumbnail.empty())
    {
        thumbnailNullable = thumbnail;
    }

    DBStatement insertStmt(*(Database::getInstance()->session()));
    uint64_t postID{0};
    insertStmt << "INSERT INTO posts ("
                  " feedID"
                  ",title"
                  ",link"
                  ",content"
                  ",author"
                  ",commentsURL"
                  ",guid"
                  ",datePublished"
                  ",thumbnail"
                  ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?) RETURNING id",
        use(feedID), useRef(title), useRef(link), useRef(content), useRef(author), useRef(commentsURL), useRef(guid), useRef(datePublished), useRef(thumbnailNullable),
        into(postID), now;

    insertStmt.execute();

    DBStatement insertStmt2(*(Database::getInstance()->session()));
    insertStmt2 << "INSERT INTO post_read (postID, feedID) VALUES (?, ?)", use(postID), use(feedID), now;

    replaceEnclosures(postID, enclosures);
    replaceCategories(postID, feedID, categories);

    auto p = std::make_unique<PostLocal>(postID);
    p->setFeedID(feedID);
    p->setFeedTitle(feedTitle);
    p->setIsRead(false);
    p->setTitle(title);
    p->setLink(link);
    p->setContent(content);
    p->setAuthor(author);
    p->setCommentsURL(commentsURL);
    p->setGuid(guid);
    p->setDatePublished(datePublished);
    p->setThumbnail(thumbnail);

    // query flags
    std::unordered_set<FlagColor> flags;
    uint8_t flagID{0};
    DBStatement selectFlagsStmt(*(Database::getInstance()->session()));
    selectFlagsStmt << "SELECT DISTINCT(flagID) FROM flags WHERE postID=?", use(postID), into(flagID), range(0, 1);
    while (!selectFlagsStmt.done())
    {
        if (selectFlagsStmt.execute() > 0)
        {
            flags.insert(Flag::flagColorForID(flagID));
        }
    }
    p->setFlagColors(flags);

    for (const auto& e : enclosures)
    {
        p->addEnclosure(e);
    }

    queryCategories(p.get());

    return p;
}

void ZapFR::Engine::PostLocal::replaceEnclosures(uint64_t postID, const std::vector<Enclosure>& enclosures)
{
    DBStatement deleteStmt(*(Database::getInstance()->session()));
    deleteStmt << "DELETE FROM post_enclosures WHERE postID=?", use(postID), now;

    for (const auto& e : enclosures)
    {
        DBStatement insertStmt(*(Database::getInstance()->session()));
        auto size = e.size; // otherwise poco complains with use(e.size) :/
        insertStmt << "INSERT INTO post_enclosures (postID, url, size, mimetype) VALUES (?, ?, ?, ?)", use(postID), useRef(e.url), use(size), useRef(e.mimeType), now;
    }
}

void ZapFR::Engine::PostLocal::replaceCategories(uint64_t postID, uint64_t feedID, const std::vector<std::string>& categories)
{
    DBStatement deleteStmt(*(Database::getInstance()->session()));
    deleteStmt << "DELETE FROM post_categories WHERE postID=?", use(postID), now;

    for (const auto& catTitle : categories)
    {
        uint64_t catID{0};
        DBStatement selectStmt(*(Database::getInstance()->session()));
        selectStmt << "SELECT id FROM categories WHERE feedID=? AND title=?", into(catID), use(feedID), useRef(catTitle), range(0, 1);
        selectStmt.execute();

        if (catID == 0)
        {
            uint64_t newCatID{0};
            DBStatement insertStmt(*(Database::getInstance()->session()));
            insertStmt << "INSERT INTO categories (title, feedID) VALUES (?, ?) RETURNING id", useRef(catTitle), use(feedID), into(newCatID), now;
            catID = newCatID;
        }

        DBStatement insertStmt(*(Database::getInstance()->session()));
        insertStmt << "INSERT INTO post_categories (postID, categoryID) VALUES (?, ?)", use(postID), use(catID), now;
    }
}

uint64_t ZapFR::Engine::PostLocal::highestID()
{
    uint64_t maxID{0};
    DBStatement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT MAX(id) FROM posts", into(maxID), now;
    return maxID;
}
