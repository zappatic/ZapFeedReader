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

#include "delegates/ItemDelegateScriptFolder.h"
#include "FeedIconCache.h"
#include "widgets/TableViewScriptFolders.h"
#include "widgets/TreeViewSources.h"

ZapFR::Client::ItemDelegateScriptFolder::ItemDelegateScriptFolder(QObject* parent) : QStyledItemDelegate(parent)
{
}

void ZapFR::Client::ItemDelegateScriptFolder::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    static bool initDone{false};
    static auto titleTextOptions = QTextOption(Qt::AlignLeft | Qt::AlignVCenter);
    static auto unreadTextOptions = QTextOption(Qt::AlignCenter | Qt::AlignVCenter);
    if (!initDone)
    {
        titleTextOptions.setWrapMode(QTextOption::NoWrap);
        unreadTextOptions.setWrapMode(QTextOption::NoWrap);
        initDone = true;
    }

    auto parentTableView = qobject_cast<QTableView*>(parent());
    auto fontMetrics = QFontMetrics(painter->font());

    // calculate the size of the badge
    auto showTotal = index.data(TableViewScriptFolders::Role::ShowTotal).toBool();
    auto showUnread = index.data(TableViewScriptFolders::Role::ShowUnread).toBool();
    auto postsTotal = index.data(TableViewScriptFolders::Role::TotalPostCount).toUInt();
    auto postsUnread = index.data(TableViewScriptFolders::Role::TotalUnreadCount).toUInt();

    QString badgeCaption;
    if (showTotal && showUnread && postsTotal > 0 && postsUnread > 0)
    {
        badgeCaption = QString("%1 / %2").arg(postsUnread).arg(postsTotal);
    }
    else if (showTotal && postsTotal > 0)
    {
        badgeCaption = QString::number(postsTotal);
    }
    else if (showUnread && postsUnread > 0)
    {
        badgeCaption = QString::number(postsUnread);
    }

    int32_t badgeWidth{0};
    int32_t badgeWidthWithMargin{0};
    if (!badgeCaption.isEmpty())
    {
        auto s = fontMetrics.size(Qt::TextSingleLine, badgeCaption);
        badgeWidth = std::max(33, s.width() + 6);
        badgeWidthWithMargin = badgeWidth + 5;
    }

    // the area where the title is drawn
    auto titleRect = option.rect;
    titleRect.adjust(4, 0, -5 - badgeWidthWithMargin, 0);

    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->setRenderHint(QPainter::Antialiasing, true);

    // determine colors to use
    auto palette = parentTableView->palette();
    QBrush brushBackground;
    auto brushText = palette.text();
    auto brushUnreadBadgeBackground = palette.highlight();
    auto penUnreadBadgeText = QPen(QBrush(Qt::white), 1.0);
    bool paintBackground{false};
    palette.setCurrentColorGroup((option.state & QStyle::State_Active) == QStyle::State_Active ? QPalette::ColorGroup::Active : QPalette::ColorGroup::Inactive);
    if ((option.state & QStyle::State_Selected) == QStyle::State_Selected)
    {
        paintBackground = true;
        brushBackground = palette.highlight();
        brushText = palette.highlightedText();
        brushUnreadBadgeBackground = QBrush(Qt::white);
        penUnreadBadgeText = QPen(QBrush(Qt::black), 1.0);
    }

    // paint the background
    if (paintBackground)
    {
        painter->fillRect(option.rect, brushBackground);
    }

    // draw the title
    auto title = index.data(Qt::DisplayRole).toString();
    auto elidedTitle = fontMetrics.elidedText(title, Qt::ElideRight, titleRect.width());
    painter->setPen(QPen(brushText, 1.0));
    painter->drawText(titleRect, elidedTitle, titleTextOptions);

    // draw the total/unread amount badge
    if (!badgeCaption.isEmpty())
    {
        auto unreadRect = option.rect;
        unreadRect.moveLeft(option.rect.right() - badgeWidthWithMargin);
        unreadRect.moveTop(option.rect.top() + 2);
        unreadRect.setWidth(badgeWidth);
        unreadRect.setHeight(option.rect.height() - 4);

        QPainterPath path;
        path.addRoundedRect(unreadRect, 6, 6);
        painter->fillPath(path, brushUnreadBadgeBackground);

        painter->setPen(penUnreadBadgeText);
        painter->drawText(unreadRect, badgeCaption, unreadTextOptions);
    }
}
