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

#include <unordered_set>

#include <Poco/Data/RecordSet.h>

#include "ZapFR/Database.h"
#include "ZapFR/Helpers.h"
#include "ZapFR/base/Category.h"

using namespace Poco::Data::Keywords;

std::vector<std::unique_ptr<ZapFR::Engine::Category>> ZapFR::Engine::Category::queryMultiple(bool distinctTitles, const std::vector<std::string>& whereClause,
                                                                                             const std::string& orderClause, const std::string& limitClause,
                                                                                             const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings)
{
    std::vector<std::unique_ptr<Category>> categories;

    uint64_t id;
    std::string title;

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT categories.id"
          ",categories.title"
          " FROM categories";
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

    std::unordered_set<std::string> seenTitles;
    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            if (distinctTitles && seenTitles.contains(Poco::toLower(title)))
            {
                continue;
            }

            auto cat = std::make_unique<Category>(id);
            cat->setTitle(title);
            categories.emplace_back(std::move(cat));

            if (distinctTitles)
            {
                seenTitles.insert(Poco::toLower(title));
            }
        }
    }
    return categories;
}

std::optional<std::unique_ptr<ZapFR::Engine::Category>> ZapFR::Engine::Category::querySingle(const std::vector<std::string>& whereClause,
                                                                                             const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings)
{
    uint64_t id;
    std::string title;

    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));

    std::stringstream ss;
    ss << "SELECT categories.id"
          ",categories.title"
          " FROM categories";
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
    selectStmt.addExtract(into(title));

    selectStmt.execute();

    auto rs = Poco::Data::RecordSet(selectStmt);
    if (rs.rowCount() == 1)
    {
        auto cat = std::make_unique<Category>(id);
        cat->setTitle(title);
        return cat;
    }

    return {};
}

std::vector<uint64_t> ZapFR::Engine::Category::getMatchingCategories(const std::string& categoryTitle)
{
    std::vector<uint64_t> matchingCategories;

    uint64_t catID{0};
    Poco::Data::Statement selectStmt(*(Database::getInstance()->session()));
    selectStmt << "SELECT DISTINCT(id) FROM categories WHERE title=? COLLATE NOCASE", into(catID), useRef(categoryTitle), range(0, 1);
    while (!selectStmt.done())
    {
        if (selectStmt.execute() > 0)
        {
            matchingCategories.emplace_back(catID);
        }
    }

    return matchingCategories;
}

Poco::JSON::Object ZapFR::Engine::Category::toJSON()
{
    Poco::JSON::Object o;

    o.set(JSON::Category::ID, mID);
    o.set(JSON::Category::Title, mTitle);

    return o;
}

std::unique_ptr<ZapFR::Engine::Category> ZapFR::Engine::Category::fromJSON(const Poco::JSON::Object::Ptr o)
{
    auto catID = o->getValue<uint64_t>(JSON::Category::ID);

    auto cat = std::make_unique<Category>(catID);
    cat->setTitle(o->getValue<std::string>(JSON::Category::Title));

    return cat;
}
