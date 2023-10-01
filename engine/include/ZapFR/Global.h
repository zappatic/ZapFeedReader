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

#ifndef ZAPFR_ENGINE_GLOBAL_H
#define ZAPFR_ENGINE_GLOBAL_H

#include <memory>
#include <vector>

namespace ZapFR
{
    namespace Engine
    {
        enum class ApplicationType
        {
            Server,
            Client
        };

        enum class SortMethod
        {
            AlphabeticallyAscending,
        };

        struct ThumbnailDataPost
        {
            ThumbnailDataPost() = default;
            ThumbnailDataPost(uint64_t tdpPostID, const std::string& tdpTitle, const std::string& tdpThumbnail, const std::string& tdpLink, time_t tdpTimestamp)
                : postID(tdpPostID), title(tdpTitle), thumbnail(tdpThumbnail), link(tdpLink), timestamp(tdpTimestamp)
            {
            }
            uint64_t postID{0};
            std::string title{""};
            std::string thumbnail{""};
            std::string link{""};
            time_t timestamp{0};
        };

        struct ThumbnailData
        {
            uint64_t feedID{0};
            std::string feedTitle{""};
            std::vector<ThumbnailDataPost> posts{};
        };

        static const uint64_t DBVersion{2};
        static const uint64_t APIVersion{1};
        static const uint64_t DefaultFeedAutoRefreshInterval{15 * 60};
        static const uint16_t DefaultServerPort{16016};
        [[maybe_unused]] static const char* IdentifierLocalServer{"zapfeedreader.local"};
        [[maybe_unused]] static const char* IdentifierRemoteServer{"zapfeedreader.remote"};

        [[maybe_unused]] static const char* JSONIdentifierRemoteConfigDataHost{"host"};
        [[maybe_unused]] static const char* JSONIdentifierRemoteConfigDataPort{"port"};
        [[maybe_unused]] static const char* JSONIdentifierRemoteConfigDataLogin{"login"};
        [[maybe_unused]] static const char* JSONIdentifierRemoteConfigDataPassword{"password"};
        [[maybe_unused]] static const char* JSONIdentifierRemoteConfigDataUseHTTPS{"useHTTPS"};

    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_GLOBAL_H