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

#ifndef ZAPFR_ENGINE_DATABASE_H
#define ZAPFR_ENGINE_DATABASE_H

#include <Poco/Data/Session.h>

#include "ZapFR/Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class Source;
        class Feed;

        class Database
        {
          public:
            Database() = default;
            Database(const Database&) = delete;
            Database& operator=(const Database&) = delete;
            virtual ~Database() = default;

            static Database* getInstance();
            void initialize(const std::string& dbPath, ApplicationType appType);

            Poco::Data::Session* session() const noexcept;

          private:
            std::unique_ptr<Poco::Data::Session> mSession{nullptr};
            ApplicationType mAppType{ApplicationType::Client};

            void upgrade();
            void installDBSchemaV1();
            void upgradeToDBSchemaV2();
            void upgradeToDBSchemaV3();
            void upgradeToDBSchemaV4();
            void upgradeToDBSchemaV5();
            void upgradeToDBSchemaV6();
            void upgradeToDBSchemaV7();
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_DATABASE_H