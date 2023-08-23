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
                                                                                             const std::string& searchFilter, FlagColor flagColor)
{

    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;
    std::string wildcardSearchFilter = "%" + searchFilter + "%";
    auto fc = Flag::idForFlagColor(flagColor);

    whereClause.emplace_back("posts.id IN (SELECT DISTINCT(postID) FROM scriptfolder_posts WHERE scriptfolder_posts.scriptfolderID=?)");
    bindings.emplace_back(useRef(mID, "scriptFolderID"));

    if (showOnlyUnread)
    {
        whereClause.emplace_back("posts.isRead=FALSE");
    }
    if (!searchFilter.empty())
    {
        whereClause.emplace_back("(posts.title LIKE ? OR posts.description LIKE ?)");
        bindings.emplace_back(useRef(wildcardSearchFilter, "searchFilter"));
        bindings.emplace_back(useRef(wildcardSearchFilter, "searchFilter"));
    }
    if (flagColor != FlagColor::Gray)
    {
        whereClause.emplace_back("posts.id IN (SELECT DISTINCT(postID) FROM flags WHERE flagID=?)");
        bindings.emplace_back(use(fc, "flagColor"));
    }

    auto offset = perPage * (page - 1);
    bindings.emplace_back(use(perPage, "perPage"));
    bindings.emplace_back(use(offset, "offset"));

    return PostLocal::queryMultiple(whereClause, "ORDER BY posts.datePublished DESC", "LIMIT ? OFFSET ?", bindings);
}

uint64_t ZapFR::Engine::ScriptFolderLocal::getTotalPostCount(bool showOnlyUnread, const std::string& searchFilter, FlagColor flagColor)
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindings;
    std::string wildcardSearchFilter = "%" + searchFilter + "%";
    auto fc = Flag::idForFlagColor(flagColor);

    whereClause.emplace_back("posts.id IN (SELECT DISTINCT(postID) FROM scriptfolder_posts WHERE scriptfolder_posts.scriptfolderID=?)");
    bindings.emplace_back(useRef(mID, "scriptFolderID"));

    if (showOnlyUnread)
    {
        whereClause.emplace_back("posts.isRead=FALSE");
    }
    if (!searchFilter.empty())
    {
        whereClause.emplace_back("(posts.title LIKE ? OR posts.description LIKE ?)");
        bindings.emplace_back(useRef(wildcardSearchFilter, "searchFilter"));
        bindings.emplace_back(useRef(wildcardSearchFilter, "searchFilter"));
    }
    if (flagColor != FlagColor::Gray)
    {
        whereClause.emplace_back("posts.id IN (SELECT DISTINCT(postID) FROM flags WHERE flagID=?)");
        bindings.emplace_back(use(fc, "flagColor"));
    }

    return PostLocal::queryCount(whereClause, bindings);
}

void ZapFR::Engine::ScriptFolderLocal::update(const std::string& title)
{
    Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
    updateStmt << "UPDATE scriptfolders SET title=? WHERE id=?", useRef(title), use(mID), now;
}
