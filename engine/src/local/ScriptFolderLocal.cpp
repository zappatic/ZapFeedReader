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

#include "ZapFR/local/ScriptFolderLocal.h"
#include "ZapFR/Database.h"
#include "ZapFR/Helpers.h"
#include "ZapFR/local/PostLocal.h"

using namespace Poco::Data::Keywords;

ZapFR::Engine::ScriptFolderLocal::ScriptFolderLocal(uint64_t id, Source* parentSource) : ScriptFolder(id, parentSource)
{
}

std::tuple<uint64_t, std::vector<std::unique_ptr<ZapFR::Engine::Post>>> ZapFR::Engine::ScriptFolderLocal::getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread,
                                                                                                                   const std::string& searchFilter, FlagColor flagColor)
{
    std::vector<std::string> whereClause;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindingsPostQuery;
    std::vector<Poco::Data::AbstractBinding::Ptr> bindingsCountQuery;
    std::string wildcardSearchFilter = "%" + searchFilter + "%";
    auto fc = Flag::idForFlagColor(flagColor);

    whereClause.emplace_back("posts.id IN (SELECT DISTINCT(postID) FROM scriptfolder_posts WHERE scriptfolder_posts.scriptfolderID=?)");
    bindingsPostQuery.emplace_back(useRef(mID, "scriptFolderID"));
    bindingsCountQuery.emplace_back(useRef(mID, "scriptFolderID"));

    if (showOnlyUnread)
    {
        whereClause.emplace_back("posts.isRead=FALSE");
    }
    if (!searchFilter.empty())
    {
        whereClause.emplace_back("(posts.title LIKE ? OR posts.content LIKE ?)");
        bindingsPostQuery.emplace_back(useRef(wildcardSearchFilter, "searchFilter"));
        bindingsPostQuery.emplace_back(useRef(wildcardSearchFilter, "searchFilter"));
        bindingsCountQuery.emplace_back(useRef(wildcardSearchFilter, "searchFilter"));
        bindingsCountQuery.emplace_back(useRef(wildcardSearchFilter, "searchFilter"));
    }
    if (flagColor != FlagColor::Gray)
    {
        whereClause.emplace_back("posts.id IN (SELECT DISTINCT(postID) FROM flags WHERE flagID=?)");
        bindingsPostQuery.emplace_back(use(fc, "flagColor"));
        bindingsCountQuery.emplace_back(use(fc, "flagColor"));
    }

    auto offset = perPage * (page - 1);
    bindingsPostQuery.emplace_back(use(perPage, "perPage"));
    bindingsPostQuery.emplace_back(use(offset, "offset"));

    auto posts = PostLocal::queryMultiple(whereClause, "ORDER BY posts.datePublished DESC", "LIMIT ? OFFSET ?", bindingsPostQuery);
    auto count = PostLocal::queryCount(whereClause, bindingsCountQuery);
    return std::make_tuple(count, std::move(posts));
}

void ZapFR::Engine::ScriptFolderLocal::update(const std::string& title)
{
    Poco::Data::Statement updateStmt(*(Database::getInstance()->session()));
    updateStmt << "UPDATE scriptfolders SET title=? WHERE id=?", useRef(title), use(mID), now;
}

std::vector<std::unique_ptr<ZapFR::Engine::ScriptFolder>> ZapFR::Engine::ScriptFolderLocal::queryMultiple(Source* parentSource, const std::vector<std::string>& whereClause,
                                                                                                          const std::string& orderClause, const std::string& limitClause,
                                                                                                          const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings)
{
    std::vector<std::unique_ptr<ScriptFolder>> scriptFolders;

    uint64_t id{0};
    std::string title{""};

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT scriptfolders.id"
          ",scriptfolders.title"
          " FROM scriptfolders";
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
    selectStmt.addExtract(into(title));

    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            auto sfl = std::make_unique<ScriptFolderLocal>(id, parentSource);
            sfl->setTitle(title);
            scriptFolders.emplace_back(std::move(sfl));
        }
    }
    return scriptFolders;
}

std::optional<std::unique_ptr<ZapFR::Engine::ScriptFolder>> ZapFR::Engine::ScriptFolderLocal::querySingle(Source* parentSource, const std::vector<std::string>& whereClause,
                                                                                                          const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings)
{
    uint64_t id{0};
    std::string title{""};

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT scriptfolders.id"
          ",scriptfolders.title"
          " FROM scriptfolders";
    if (!whereClause.empty())
    {
        ss << " WHERE ";
        ss << Helpers::joinString(whereClause, " AND ");
    }

    auto sql = ss.str();

    selectStmt << sql, range(0, 1);

    for (const auto& binding : bindings)
    {
        selectStmt.addBind(binding);
    }

    selectStmt.addExtract(into(id));
    selectStmt.addExtract(into(title));

    selectStmt.execute();

    auto rs = Poco::Data::RecordSet(selectStmt);
    if (rs.rowCount() == 1)
    {
        auto sfl = std::make_unique<ScriptFolderLocal>(id, parentSource);
        sfl->setTitle(title);
        return sfl;
    }
    return {};
}

void ZapFR::Engine::ScriptFolderLocal::create(const std::string& title)
{
    Poco::Data::Statement insertStmt(*(Database::getInstance()->session()));
    insertStmt << "INSERT INTO scriptfolders (title) VALUES (?)", useRef(title), now;
}

void ZapFR::Engine::ScriptFolderLocal::remove(uint64_t scriptFolderID)
{
    {
        Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
        deleteStmt << "DELETE FROM scriptfolders WHERE id=?", use(scriptFolderID), now;
    }

    {
        Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
        deleteStmt << "DELETE FROM scriptfolder_posts WHERE scriptFolderID=?", use(scriptFolderID), now;
    }
}
