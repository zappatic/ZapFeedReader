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

#ifndef ZAPFR_SERVER_DUMMYFEED_H
#define ZAPFR_SERVER_DUMMYFEED_H

#include <vector>


namespace ZapFR
{
    namespace Server
    {
        class DummyFeed
        {
          public:
            DummyFeed(const DummyFeed&) = delete;
            DummyFeed& operator=(const DummyFeed&) = delete;
            virtual ~DummyFeed() = default;

            static DummyFeed* getInstance();

            std::string getRSS20();

            struct Entry
            {
                Entry(const std::string& entryTitle, const std::string& entryContent, const std::string& entryGuid, const Poco::Timestamp& entryDatePublished)
                    : title(entryTitle), content(entryContent), guid(entryGuid), datePublished(entryDatePublished)
                {
                }
                std::string title{""};
                std::string content{""};
                std::string guid{""};
                Poco::Timestamp datePublished{};
            };

            void addEntry(const std::string& title, const std::string& content, const std::string& guid, const Poco::Timestamp& datePublished);

          private:
            explicit DummyFeed();

            std::vector<Entry> mEntries{};
        };
    } // namespace Server
} // namespace ZapFR
#endif // ZAPFR_SERVER_DUMMYFEED_H