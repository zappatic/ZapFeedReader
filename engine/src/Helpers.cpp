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

#include "Helpers.h"

std::string ZapFR::Engine::Helpers::joinString(const std::vector<std::string>& sourceVector, const char* delimiter)
{
    switch (sourceVector.size())
    {
        case 0:
            return "";
        case 1:
            return sourceVector[0];
        default:
            std::ostringstream os;
            std::copy(sourceVector.begin(), sourceVector.end() - 1, std::ostream_iterator<std::string>(os, delimiter));
            os << *sourceVector.rbegin();
            return os.str();
    }
}
