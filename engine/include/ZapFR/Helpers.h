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

#ifndef ZAPFR_ENGINE_HELPERS_H
#define ZAPFR_ENGINE_HELPERS_H

#include <optional>

#include <Poco/Net/HTTPCredentials.h>

namespace ZapFR
{
    namespace Engine
    {
        class Helpers
        {
          public:
            static void splitString(const std::string& sourceString, char delimiter, std::vector<std::string>& outSubstrings);
            static std::string joinString(const std::vector<std::string>& sourceVector, const char* delimiter);
            static std::string joinIDNumbers(const std::vector<uint64_t>& sourceVector, const char* delimiter);
            static std::tuple<std::string, std::string> performHTTPRequest(Poco::URI& url, const std::string& method, Poco::Net::HTTPCredentials& credentials,
                                                                           const std::map<std::string, std::string>& parameters, std::optional<uint64_t> associatedFeedID = {},
                                                                           std::optional<std::string> conditionalGetInfo = {});
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_HELPERS_H