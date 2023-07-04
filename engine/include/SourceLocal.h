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

#ifndef ZAPFR_ENGINE_SOURCELOCAL_H
#define ZAPFR_ENGINE_SOURCELOCAL_H

#include "Source.h"

namespace ZapFR
{
    namespace Engine
    {
        class Database;

        class SourceLocal : public Source
        {
          public:
            explicit SourceLocal(Database* db);
            ~SourceLocal() = default;

            Poco::JSON::Array getFeeds() override;
            std::optional<Poco::JSON::Object> getFeed(uint64_t feedID) override;
            Poco::JSON::Array getPosts(uint64_t feedID, uint64_t perPage, uint64_t page) override;
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_SOURCELOCAL_HZAPFR_ENGINE_SOURCELOCAL_HZAPFR_ENGINE_SOURCELOCAL_HZAPFR_ENGINE_SOURCELOCAL_HZAPFR_ENGINE_SOURCELOCAL_H