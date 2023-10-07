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

#ifndef ZAPFR_ENGINE_CATEGORY_H
#define ZAPFR_ENGINE_CATEGORY_H

#include <Poco/Data/AbstractBinding.h>
#include <Poco/JSON/Object.h>

namespace ZapFR
{
    namespace Engine
    {
        class Category
        {
          public:
            explicit Category(uint64_t id) : mID(id) {}
            virtual ~Category() = default;
            Category(const Category& e) = delete;
            Category& operator=(const Category&) = delete;
            Category(Category&&) = delete;
            Category& operator=(Category&&) = delete;

            const uint64_t& id() const noexcept { return mID; }
            const std::string& title() const noexcept { return mTitle; }

            void setTitle(const std::string& t) { mTitle = t; }

            Poco::JSON::Object toJSON();
            static std::unique_ptr<Category> fromJSON(const Poco::JSON::Object::Ptr o);

            static std::vector<std::unique_ptr<Category>> queryMultiple(const std::vector<std::string>& whereClause, const std::string& orderClause,
                                                                        const std::string& limitClause, const std::vector<Poco::Data::AbstractBinding::Ptr>& bindings);

          protected:
            uint64_t mID{0};
            std::string mTitle{""};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_CATEGORY_H