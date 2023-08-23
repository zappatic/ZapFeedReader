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

#include "ZapFR/ScriptFolderLocal.h"
#include "ZapFR/Database.h"
#include "ZapFR/PostLocal.h"

using namespace Poco::Data::Keywords;

ZapFR::Engine::ScriptFolderLocal::ScriptFolderLocal(uint64_t id) : ScriptFolder(id)
{
}

std::vector<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::ScriptFolderLocal::getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                                                                             const std::string& searchFilter)
{
    std::vector<std::unique_ptr<Post>> posts;

    auto offset = perPage * (page - 1);

    uint64_t id{0};
    bool isRead{false};
    uint64_t feedID{0};
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
    std::string whereClause = showOnlyUnread ? "AND posts.isRead=FALSE" : "";

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    if (searchFilter.empty())
    {
        selectStmt << Poco::format("SELECT posts.id"
                                   ",posts.isRead"
                                   ",posts.feedID"
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
                                   " LEFT JOIN feeds ON feeds.id = posts.feedID"
                                   " WHERE posts.id IN (SELECT DISTINCT(postID) FROM scriptfolder_posts WHERE scriptfolder_posts.scriptfolderID=?)"
                                   "       %s"
                                   " ORDER BY posts.datePublished DESC"
                                   " LIMIT ? OFFSET ?",
                                   whereClause),
            use(mID), use(perPage), use(offset), into(id), into(isRead), into(feedID), into(title), into(link), into(description), into(author), into(commentsURL),
            into(enclosureURL), into(enclosureLength), into(enclosureMimeType), into(guid), into(guidIsPermalink), into(datePublished), into(sourceURL), into(sourceTitle),
            into(feedTitle), range(0, 1);
    }
    else
    {
        std::string wildcardSearchFilter = "%" + searchFilter + "%";
        selectStmt << Poco::format("SELECT posts.id"
                                   ",posts.isRead"
                                   ",posts.feedID"
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
                                   " LEFT JOIN feeds ON feeds.id = posts.feedID"
                                   " WHERE posts.id IN (SELECT DISTINCT(postID) FROM scriptfolder_posts WHERE scriptfolder_posts.scriptfolderID=?)"
                                   "   AND (posts.title LIKE ? OR posts.description LIKE ?)"
                                   "       %s"
                                   " ORDER BY posts.datePublished DESC"
                                   " LIMIT ? OFFSET ?",
                                   whereClause),
            use(mID), use(wildcardSearchFilter), use(wildcardSearchFilter), use(perPage), use(offset), into(id), into(isRead), into(feedID), into(title), into(link),
            into(description), into(author), into(commentsURL), into(enclosureURL), into(enclosureLength), into(enclosureMimeType), into(guid), into(guidIsPermalink),
            into(datePublished), into(sourceURL), into(sourceTitle), into(feedTitle), range(0, 1);
    }

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

uint64_t ZapFR::Engine::ScriptFolderLocal::getTotalPostCount(bool showOnlyUnread, const std::string& searchFilter)
{
    std::string whereClause = showOnlyUnread ? " AND posts.isRead=FALSE" : "";

    uint64_t postCount;
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    if (searchFilter.empty())
    {
        std::string joinClause = showOnlyUnread ? " LEFT JOIN posts ON posts.id = scriptfolder_posts.postID" : "";
        selectStmt << Poco::format("SELECT COUNT(*)"
                                   " FROM scriptfolder_posts"
                                   " %s"
                                   " WHERE scriptfolderID=?"
                                   " %s",
                                   joinClause, whereClause),
            use(mID), into(postCount), now;
    }
    else
    {
        std::string wildcardSearchFilter = "%" + searchFilter + "%";
        selectStmt << Poco::format("SELECT COUNT(*)"
                                   " FROM scriptfolder_posts"
                                   " LEFT JOIN posts ON posts.id = scriptfolder_posts.postID"
                                   " WHERE scriptfolder_posts.scriptfolderID=?"
                                   "   AND (posts.title LIKE ? OR posts.description LIKE ?)"
                                   " %s",
                                   whereClause),
            use(mID), use(wildcardSearchFilter), use(wildcardSearchFilter), into(postCount), now;
    }
    return postCount;
}

std::vector<std::unique_ptr<ZapFR::Engine::Post>> ZapFR::Engine::ScriptFolderLocal::getFlaggedPosts(FlagColor flagColor, uint64_t perPage, uint64_t page, bool showOnlyUnread)
{
    std::vector<std::unique_ptr<Post>> posts;

    auto offset = perPage * (page - 1);
    auto fc = Flag::idForFlagColor(flagColor);

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

    std::string whereClause = showOnlyUnread ? "AND posts.isRead=FALSE" : "";

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << Poco::format("SELECT posts.id"
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
                               " LEFT JOIN feeds ON feeds.id = posts.feedID"
                               " WHERE posts.id IN (SELECT DISTINCT(postID) FROM flags WHERE flagID=?)"
                               "   AND posts.id IN (SELECT DISTINCT(postID) FROM scriptfolder_posts WHERE scriptfolder_posts.scriptfolderID=?)"
                               " %s"
                               " ORDER BY posts.datePublished DESC"
                               " LIMIT ? OFFSET ?",
                               whereClause),
        use(fc), use(mID), use(perPage), use(offset), into(id), into(feedID), into(isRead), into(title), into(link), into(description), into(author), into(commentsURL),
        into(enclosureURL), into(enclosureLength), into(enclosureMimeType), into(guid), into(guidIsPermalink), into(datePublished), into(sourceURL), into(sourceTitle),
        into(feedTitle), range(0, 1);

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

uint64_t ZapFR::Engine::ScriptFolderLocal::getTotalFlaggedPostCount(FlagColor flagColor, bool showOnlyUnread)
{
    auto fc = Flag::idForFlagColor(flagColor);
    uint64_t postCount;

    std::string whereClause = showOnlyUnread ? "AND posts.isRead=FALSE" : "";

    auto sql = Poco::format("SELECT COUNT(*)"
                            " FROM posts"
                            " WHERE posts.id IN (SELECT DISTINCT(postID) FROM flags WHERE flagID=?)"
                            "   AND posts.id IN (SELECT DISTINCT(postID) FROM scriptfolder_posts WHERE scriptfolder_posts.scriptfolderID=?)"
                            "   %s",
                            whereClause);

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << sql, use(fc), use(mID), into(postCount), now;
    return postCount;
}

void ZapFR::Engine::ScriptFolderLocal::update(const std::string& title)
{
    Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
    updateStmt << "UPDATE scriptfolders SET title=? WHERE id=?", useRef(title), use(mID), now;
}
