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

#ifndef ZAPFR_ENGINE_STATEMENT_H
#define ZAPFR_ENGINE_STATEMENT_H

#include <Poco/Data/Statement.h>
#include <chrono>

namespace ZapFR
{
    namespace Engine
    {
        class LoggingStatement : public Poco::Data::Statement
        {
          public:
            using Poco::Data::Statement::Statement;

            std::size_t execute(bool reset = true)
            {
                using namespace std::chrono;

                auto start = high_resolution_clock::now();

                auto result = Poco::Data::Statement::execute(reset);

                auto end = high_resolution_clock::now();
                auto ms = duration_cast<milliseconds>(end - start).count();

                std::println(std::cout, "[SQL]: {}", this->toString());
                std::println(std::cout, " -duration: {} ms\n", ms);

                return result;
            }
        };
    } // namespace Engine
} // namespace ZapFR

#endif