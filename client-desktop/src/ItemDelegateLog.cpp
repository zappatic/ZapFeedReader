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

#include "ItemDelegateLog.h"
#include "FeedIconCache.h"

ZapFR::Client::ItemDelegateLog::ItemDelegateLog(QObject* parent) : QStyledItemDelegate(parent)
{
}

void ZapFR::Client::ItemDelegateLog::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto currentColumn = index.column();
    switch (currentColumn)
    {
        case LogsColumnFeed:
        {
            auto pixmap = FeedIconCache::icon(index.data(LogFeedIDRole).toULongLong());
            if (!pixmap.isNull())
            {
                auto targetWidth = static_cast<float>(option.rect.height()) * 0.6f;
                auto targetX = static_cast<float>(option.rect.left()) + ((static_cast<float>(option.rect.width()) / 2.0f) - (targetWidth / 2.0f));
                auto targetY = option.rect.top() + ((option.rect.height() / 2.0) - (targetWidth / 2.0f));
                auto target = QRectF(targetX, targetY, targetWidth, targetWidth);
                painter->drawPixmap(target, pixmap, pixmap.rect());
            }
            break;
        }
        default:
        {
            QStyledItemDelegate::paint(painter, option, index);
        }
    }
}
