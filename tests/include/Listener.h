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

#ifndef ZAPFR_TESTS_LISTENER_H
#define ZAPFR_TESTS_LISTENER_H

#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <thread>

namespace ZapFR
{
    namespace Tests
    {
        class TestRunListener : public Catch::EventListenerBase
        {
          public:
            using Catch::EventListenerBase::EventListenerBase;

            void testRunStarting(Catch::TestRunInfo const&) override;
            void testRunEnded(Catch::TestRunStats const&) override;

          private:
            std::string mTemporaryDirectory{""};
            std::string mTemporaryConfigFilePath{""};
            std::thread mServerThread{};
            bool mRunServer{true};

            void createFakeTestServerConfig();
            void launchTestServer();
            void createRemoteSource();

            static constexpr uint16_t msPort{16017};
        };
    } // namespace Tests
} // namespace ZapFR

#endif // ZAPFR_TESTS_LISTENER_H