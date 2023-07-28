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

#ifndef ZAPFR_CLIENT_FEEDICONCACHE_H
#define ZAPFR_CLIENT_FEEDICONCACHE_H

#include "ClientGlobal.h"

namespace ZapFR
{
    namespace Client
    {
        class FeedIconCache
        {
          public:
            static void cache(uint64_t feedID, const std::string& hash, const QPixmap& pixmap);
            static QPixmap icon(uint64_t feedID);
            static bool isCached(uint64_t feedID);
            static bool isSameHash(uint64_t feedID, const std::string& hash);

          private:
            static std::unordered_map<uint64_t, QPixmap> msPixmapCache;
            static std::unordered_map<uint64_t, std::string> msHashCache;
            static std::mutex msCacheMutex;
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_FEEDICONCACHE_H
