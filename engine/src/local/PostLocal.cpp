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

#include "ZapFR/local/PostLocal.h"
#include "ZapFR/Database.h"
#include "ZapFR/Helpers.h"

using namespace Poco::Data::Keywords;

std::mutex ZapFR::Engine::PostLocal::msCreatePostMutex{};

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
    Poco::Data::Statement insertStmt(*(Database::getInstance()->session()));
    insertStmt << "INSERT INTO flags (postID, flagID) VALUES (?, ?)", use(mID), use(fc), now;
}

void ZapFR::Engine::PostLocal::markUnflagged(FlagColor flagColor)
{
    auto fc = Flag::idForFlagColor(flagColor);
    Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
    deleteStmt << "DELETE FROM flags WHERE postID=? AND flagID=?", use(mID), use(fc), now;
}

void ZapFR::Engine::PostLocal::markAsRead()
{
    updateIsRead(true, {"posts.feedID=?", "posts.id=?"}, {use(mFeedID, "feedID"), use(mID, "id")});
}

void ZapFR::Engine::PostLocal::markAsUnread()
{
    updateIsRead(false, {"posts.feedID=?", "posts.id=?"}, {use(mFeedID, "feedID"), use(mID, "id")});
}

void ZapFR::Engine::PostLocal::assignToScriptFolder(uint64_t scriptFolderID)
{
    unassignFromScriptFolder(scriptFolderID);
    Poco::Data::Statement insertStmt(*(Database::getInstance()->session()));
    insertStmt << "INSERT INTO scriptfolder_posts (scriptFolderID, postID) VALUES (?, ?)", use(scriptFolderID), use(mID), now;
}

void ZapFR::Engine::PostLocal::unassignFromScriptFolder(uint64_t scriptFolderID)
{
    Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
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
    std::string feedTitle{""};

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT posts.id"
          ",posts.feedID"
          ",posts.isRead"
          ",posts.title"
          ",posts.link"
          ",posts.description"
          ",posts.author"
          ",posts.commentsURL"
          ",posts.enclosureURL"
          ",posts.enclosureLength"
          ",posts.enclosureMimeType"
          ",posts.guid"
          ",posts.guidIsPermalink"
          ",posts.datePublished"
          ",posts.sourceURL"
          ",posts.sourceTitle"
          ",feeds.title"
          " FROM posts"
          " LEFT JOIN feeds ON feeds.id = posts.feedID";
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
    selectStmt.addExtract(into(description));
    selectStmt.addExtract(into(author));
    selectStmt.addExtract(into(commentsURL));
    selectStmt.addExtract(into(enclosureURL));
    selectStmt.addExtract(into(enclosureLength));
    selectStmt.addExtract(into(enclosureMimeType));
    selectStmt.addExtract(into(guid));
    selectStmt.addExtract(into(guidIsPermalink));
    selectStmt.addExtract(into(datePublished));
    selectStmt.addExtract(into(sourceURL));
    selectStmt.addExtract(into(sourceTitle));
    selectStmt.addExtract(into(feedTitle));

    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            auto p = std::make_unique<PostLocal>(id);
            p->setIsRead(isRead);
            p->setFeedID(feedID);
            p->setFeedTitle(feedTitle);
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

            // query flags
            std::unordered_set<FlagColor> flags;
            uint8_t flagID{0};
            Poco::Data::Statement selectFlagsStmt(*(Database::getInstance()->session()));
            selectFlagsStmt << "SELECT DISTINCT(flagID) FROM flags WHERE postID=?", use(id), into(flagID), range(0, 1);
            while (!selectFlagsStmt.done())
            {
                if (selectFlagsStmt.execute() > 0)
                {
                    flags.insert(Flag::flagColorForID(flagID));
                }
            }
            p->setFlagColors(flags);

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
    std::string feedTitle{""};

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT posts.id"
          ",posts.feedID"
          ",posts.isRead"
          ",posts.title"
          ",posts.link"
          ",posts.description"
          ",posts.author"
          ",posts.commentsURL"
          ",posts.enclosureURL"
          ",posts.enclosureLength"
          ",posts.enclosureMimeType"
          ",posts.guid"
          ",posts.guidIsPermalink"
          ",posts.datePublished"
          ",posts.sourceURL"
          ",posts.sourceTitle"
          ",feeds.title"
          " FROM posts"
          " LEFT JOIN feeds ON feeds.id = posts.feedID";
    if (!whereClause.empty())
    {
        ss << " WHERE ";
        ss << Helpers::joinString(whereClause, " AND ");
    }

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
    selectStmt.addExtract(into(description));
    selectStmt.addExtract(into(author));
    selectStmt.addExtract(into(commentsURL));
    selectStmt.addExtract(into(enclosureURL));
    selectStmt.addExtract(into(enclosureLength));
    selectStmt.addExtract(into(enclosureMimeType));
    selectStmt.addExtract(into(guid));
    selectStmt.addExtract(into(guidIsPermalink));
    selectStmt.addExtract(into(datePublished));
    selectStmt.addExtract(into(sourceURL));
    selectStmt.addExtract(into(sourceTitle));
    selectStmt.addExtract(into(feedTitle));

    selectStmt.execute();

    auto rs = Poco::Data::RecordSet(selectStmt);
    if (rs.rowCount() == 1)
    {
        auto p = std::make_unique<PostLocal>(id);
        p->setFeedID(feedID);
        p->setFeedTitle(feedTitle);
        p->setIsRead(isRead);
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

        // query flags
        std::unordered_set<FlagColor> flags;
        uint8_t flagID{0};
        Poco::Data::Statement selectFlagsStmt(*(Database::getInstance()->session()));
        selectFlagsStmt << "SELECT DISTINCT(flagID) FROM flags WHERE postID=?", use(id), into(flagID), range(0, 1);
        while (!selectFlagsStmt.done())
        {
            if (selectFlagsStmt.execute() > 0)
            {
                flags.insert(Flag::flagColorForID(flagID));
            }
        }
        p->setFlagColors(flags);

        return p;
    }

    return {};
}

uint64_t ZapFR::Engine::PostLocal::queryCount(const std::vector<std::string>& whereClause, const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings)
{
    uint64_t postCount;
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT COUNT(*) FROM posts";
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

void ZapFR::Engine::PostLocal::updateIsRead(bool isRead, const std::vector<std::string>& whereClause, const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings)
{
    Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "UPDATE posts SET isRead=?";
    if (!whereClause.empty())
    {
        ss << " WHERE ";
        ss << Helpers::joinString(whereClause, " AND ");
    }

    auto sql = ss.str();

    updateStmt << sql;

    updateStmt.addBind(use(isRead, "isRead"));
    for (const auto& binding : bindings)
    {
        updateStmt.addBind(binding);
    }

    updateStmt.execute();
}

void ZapFR::Engine::PostLocal::update(const std::string& title, const std::string& link, const std::string& description, const std::string& author,
                                      const std::string& commentsURL, const std::string& enclosureURL, const std::string& enclosureLength,
                                      const std::string& enclosureMimeType, const std::string& guid, bool guidIsPermalink, const std::string& datePublished,
                                      const std::string& sourceURL, const std::string& sourceTitle)
{
    Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
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
                  " WHERE id=?",
        useRef(title), useRef(link), useRef(description), useRef(author), useRef(commentsURL), useRef(enclosureURL), useRef(enclosureLength), useRef(enclosureMimeType),
        useRef(guid), use(guidIsPermalink), useRef(datePublished), useRef(sourceURL), useRef(sourceTitle), use(mID);
    updateStmt.execute();
}

std::unique_ptr<ZapFR::Engine::Post> ZapFR::Engine::PostLocal::create(uint64_t feedID, const std::string& feedTitle, const std::string& title, const std::string& link,
                                                                      const std::string& description, const std::string& author, const std::string& commentsURL,
                                                                      const std::string& enclosureURL, const std::string& enclosureLength,
                                                                      const std::string& enclosureMimeType, const std::string& guid, bool guidIsPermalink,
                                                                      const std::string& datePublished, const std::string& sourceURL, const std::string& sourceTitle)
{
    Poco::Data::Statement insertStmt(*(Database::getInstance()->session()));
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
        use(feedID), useRef(title), useRef(link), useRef(description), useRef(author), useRef(commentsURL), useRef(enclosureURL), useRef(enclosureLength),
        useRef(enclosureMimeType), useRef(guid), use(guidIsPermalink), useRef(datePublished), useRef(sourceURL), useRef(sourceTitle);

    uint64_t postID{0};
    {
        const std::lock_guard<std::mutex> lock(msCreatePostMutex);
        insertStmt.execute();
        Poco::Data::Statement selectInsertRowIDStmt(*(Database::getInstance()->session()));
        selectInsertRowIDStmt << "SELECT last_insert_rowid()", into(postID), now;
    }

    auto p = std::make_unique<PostLocal>(postID);
    p->setFeedID(feedID);
    p->setFeedTitle(feedTitle);
    p->setIsRead(false);
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

    // query flags
    std::unordered_set<FlagColor> flags;
    uint8_t flagID{0};
    Poco::Data::Statement selectFlagsStmt(*(Database::getInstance()->session()));
    selectFlagsStmt << "SELECT DISTINCT(flagID) FROM flags WHERE postID=?", use(postID), into(flagID), range(0, 1);
    while (!selectFlagsStmt.done())
    {
        if (selectFlagsStmt.execute() > 0)
        {
            flags.insert(Flag::flagColorForID(flagID));
        }
    }
    p->setFlagColors(flags);

    return p;
}
