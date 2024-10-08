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

#include <QBuffer>

#include "FeedIconCache.h"

std::unordered_map<uint64_t, std::unordered_map<uint64_t, QPixmap>> ZapFR::Client::FeedIconCache::msPixmapCache{};
std::unordered_map<uint64_t, std::unordered_map<uint64_t, QString>> ZapFR::Client::FeedIconCache::msBase64Cache{};
std::unordered_map<uint64_t, std::unordered_map<uint64_t, QPixmap>> ZapFR::Client::FeedIconCache::msPixmapGrayscaleCache{};
std::unordered_map<uint64_t, std::unordered_map<uint64_t, std::string>> ZapFR::Client::FeedIconCache::msHashCache{};
std::mutex ZapFR::Client::FeedIconCache::msCacheMutex{};

void ZapFR::Client::FeedIconCache::cache(uint64_t sourceID, uint64_t feedID, const std::string& hash, const QPixmap& pixmap)
{
    std::lock_guard<std::mutex> lock(msCacheMutex);
    if (!msPixmapCache.contains(sourceID))
    {
        msPixmapCache[sourceID] = {};
    }
    if (!msPixmapGrayscaleCache.contains(sourceID))
    {
        msPixmapGrayscaleCache[sourceID] = {};
    }
    if (!msHashCache.contains(sourceID))
    {
        msHashCache[sourceID] = {};
    }
    msPixmapCache[sourceID][feedID] = pixmap;
    msHashCache[sourceID][feedID] = hash;

    // create a grayscale version of the icon
    auto img = pixmap.toImage();
    auto imgGrayscale = img.convertToFormat(QImage::Format_Grayscale8);
    msPixmapGrayscaleCache[sourceID][feedID] = QPixmap::fromImage(imgGrayscale);
}

const QPixmap& ZapFR::Client::FeedIconCache::icon(uint64_t sourceID, uint64_t feedID)
{
    std::lock_guard<std::mutex> lock(msCacheMutex);
    if (msPixmapCache.contains(sourceID))
    {
        auto& s = msPixmapCache.at(sourceID);
        if (s.contains(feedID))
        {
            return s.at(feedID);
        }
    }
    static QPixmap rssIcon(":/rss.png");
    return rssIcon;
}

const QPixmap& ZapFR::Client::FeedIconCache::iconGrayscale(uint64_t sourceID, uint64_t feedID)
{
    std::lock_guard<std::mutex> lock(msCacheMutex);
    if (msPixmapGrayscaleCache.contains(sourceID))
    {
        auto& s = msPixmapGrayscaleCache.at(sourceID);
        if (s.contains(feedID))
        {
            return s.at(feedID);
        }
    }
    static QPixmap rssIcon;

    if (rssIcon.isNull())
    {
        auto img = QImage(":/rss.png");
        auto imgGrayscale = img.convertToFormat(QImage::Format_Grayscale8);
        rssIcon = QPixmap::fromImage(imgGrayscale);
    }
    return rssIcon;
}

const QString& ZapFR::Client::FeedIconCache::base64icon(uint64_t sourceID, uint64_t feedID)
{
    // avoid double lock below
    {
        std::lock_guard<std::mutex> lock(msCacheMutex);
        if (msBase64Cache.contains(sourceID))
        {
            auto& s = msBase64Cache.at(sourceID);
            if (s.contains(feedID))
            {
                return s.at(feedID);
            }
        }
    }

    // we leave the caching here, not in the cache function, as the base64 icon is only requested
    // for the thumbnail overview, so avoid this work until that request is made
    const auto& icon = FeedIconCache::icon(sourceID, feedID).scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    icon.save(&buffer, "PNG");
    auto base64Icon = QString("data:image/png;base64, %1").arg(QString(buffer.data().toBase64()));

    // avoid double lock again
    {
        std::lock_guard<std::mutex> lock(msCacheMutex);
        if (!msBase64Cache.contains(sourceID))
        {
            msBase64Cache[sourceID] = {};
        }
        msBase64Cache[sourceID][feedID] = base64Icon;
    }

    return FeedIconCache::base64icon(sourceID, feedID);
}

bool ZapFR::Client::FeedIconCache::isCached(uint64_t sourceID, uint64_t feedID)
{
    std::lock_guard<std::mutex> lock(msCacheMutex);
    return (msPixmapCache.contains(sourceID) && msPixmapCache.at(sourceID).contains(feedID));
}

bool ZapFR::Client::FeedIconCache::isSameHash(uint64_t sourceID, uint64_t feedID, const std::string& hash)
{
    std::lock_guard<std::mutex> lock(msCacheMutex);
    if (msHashCache.contains(sourceID))
    {
        auto& s = msHashCache.at(sourceID);
        if (s.contains(feedID))
        {
            return s.at(feedID) == hash;
        }
    }
    return false;
}
