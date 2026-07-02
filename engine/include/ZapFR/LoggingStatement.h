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
#include <source_location>

namespace ZapFR
{
    namespace Engine
    {
        class LoggingStatement : public Poco::Data::Statement
        {
          public:
            using Poco::Data::Statement::Statement;

            std::size_t execute(bool reset = true, const std::source_location loc = std::source_location::current())
            {
                using namespace std::chrono;

                auto start = high_resolution_clock::now();

                size_t result;
                try
                {
                    result = Poco::Data::Statement::execute(reset);
                }
                catch (const Poco::Exception& e)
                {
                    throw std::runtime_error(e.displayText() + " (thrown at " + loc.file_name() + ":" + std::to_string(loc.line()) + ", in " + loc.function_name() +
                                             ") SQL: " + this->toString());
                }

                auto end = high_resolution_clock::now();
                auto ms = duration_cast<milliseconds>(end - start).count();

                if (msLogQueriesToStdout && (!msOmitZeroMsQueries || ms > 0))
                {
                    std::cout << "[SQL]: " << this->toString() << "\n";
                    std::cout << " -duration: " << ms << " ms\n";
                }
                return result;
            }

            static void setLogQueriesToStdout(bool b) { msLogQueriesToStdout = b; }
            static void setOmitZeroMsQueries(bool b) { msOmitZeroMsQueries = b; }

          private:
            static inline bool msLogQueriesToStdout{false};
            static inline bool msOmitZeroMsQueries{true};
        };
    } // namespace Engine
} // namespace ZapFR

#endif
