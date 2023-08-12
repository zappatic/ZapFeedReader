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

#include "ZapFR/Flag.h"

std::unordered_map<ZapFR::Engine::FlagColor, std::tuple<uint8_t, uint8_t, uint8_t>> ZapFR::Engine::Flag::msColorRGBMapping{
    {FlagColor::Gray, {158, 158, 158}},  {FlagColor::Blue, {12, 147, 205}}, {FlagColor::Green, {135, 186, 35}}, {FlagColor::Yellow, {251, 213, 55}},
    {FlagColor::Orange, {235, 148, 95}}, {FlagColor::Red, {224, 74, 104}},  {FlagColor::Purple, {152, 57, 154}}};

std::unordered_map<ZapFR::Engine::FlagColor, uint8_t> ZapFR::Engine::Flag::msColorIDMapping{
    {FlagColor::Gray, 1}, {FlagColor::Blue, 2}, {FlagColor::Green, 3}, {FlagColor::Yellow, 4}, {FlagColor::Orange, 5}, {FlagColor::Red, 6}, {FlagColor::Purple, 7}};

std::unordered_map<uint8_t, ZapFR::Engine::FlagColor> ZapFR::Engine::Flag::msIDColorMapping{
    {1, FlagColor::Gray}, {2, FlagColor::Blue}, {3, FlagColor::Green}, {4, FlagColor::Yellow}, {5, FlagColor::Orange}, {6, FlagColor::Red}, {7, FlagColor::Purple}};

std::tuple<uint8_t, uint8_t, uint8_t> ZapFR::Engine::Flag::rgbForColor(FlagColor color)
{
    if (msColorRGBMapping.contains(color))
    {
        return msColorRGBMapping.at(color);
    }
    return std::make_tuple(255, 0, 255);
}

ZapFR::Engine::FlagColor ZapFR::Engine::Flag::flagColorForID(uint8_t id)
{
    if (msIDColorMapping.contains(id))
    {
        return msIDColorMapping.at(id);
    }
    throw std::runtime_error("Unknown flag color ID requested");
}

uint8_t ZapFR::Engine::Flag::idForFlagColor(FlagColor flagColor)
{
    if (msColorIDMapping.contains(flagColor))
    {
        return msColorIDMapping.at(flagColor);
    }
    throw std::runtime_error("Unknown flag color requested");
}
