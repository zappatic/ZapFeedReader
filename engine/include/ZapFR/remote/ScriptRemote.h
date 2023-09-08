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

#ifndef ZAPFR_ENGINE_SCRIPTREMOTE_H
#define ZAPFR_ENGINE_SCRIPTREMOTE_H

#include "ZapFR/base/Script.h"

namespace ZapFR
{
    namespace Engine
    {
        class ScriptRemote : public Script
        {
          public:
            ScriptRemote(uint64_t id, Source* parentSource);
            ~ScriptRemote() = default;

            void update(Type type, const std::string& filename, bool enabled, const std::unordered_set<Event>& events,
                        const std::optional<std::unordered_set<uint64_t>>& feedIDs) override;

            static std::unique_ptr<Script> fromJSON(Source* parentSource, const Poco::JSON::Object::Ptr o);
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_SCRIPTREMOTE_H