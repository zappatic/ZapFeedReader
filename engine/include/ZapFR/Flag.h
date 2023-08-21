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

#ifndef ZAPFR_ENGINE_FLAG_H
#define ZAPFR_ENGINE_FLAG_H

#include "Database.h"
#include "Global.h"

namespace ZapFR
{
    namespace Engine
    {
        enum class FlagColor
        {
            Gray,
            Blue,
            Green,
            Yellow,
            Orange,
            Red,
            Purple
        };

        class Flag
        {
          public:
            static std::tuple<uint8_t, uint8_t, uint8_t> rgbForColor(FlagColor color);
            static FlagColor flagColorForID(uint8_t id);
            static FlagColor flagColorForName(const std::string& name);
            static uint8_t idForFlagColor(FlagColor flagColor);
            static const std::unordered_set<FlagColor>& allFlagColors();

          private:
            static std::unordered_map<FlagColor, std::tuple<uint8_t, uint8_t, uint8_t>> msColorRGBMapping;
            static std::unordered_map<FlagColor, uint8_t> msColorIDMapping;
            static std::unordered_map<uint8_t, FlagColor> msIDColorMapping;
        };

    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FLAG_H