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

#include "ItemDelegateSource.h"
#include "TreeViewSources.h"

ZapFR::Client::ItemDelegateSource::ItemDelegateSource(QObject* parent) : QStyledItemDelegate(parent)
{
}

void ZapFR::Client::ItemDelegateSource::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    static bool initDone{false};
    static constexpr int32_t unreadBadgeWidth{33};
    static constexpr int32_t unreadBadgeWidthWithMargin{unreadBadgeWidth + 5};
    static auto titleTextOptions = QTextOption(Qt::AlignLeft | Qt::AlignVCenter);
    static auto unreadTextOptions = QTextOption(Qt::AlignCenter | Qt::AlignVCenter);
    if (!initDone)
    {
        titleTextOptions.setWrapMode(QTextOption::NoWrap);
        unreadTextOptions.setWrapMode(QTextOption::NoWrap);
        initDone = true;
    }

    auto parentTreeView = qobject_cast<TreeViewSources*>(parent());

    // the area where the title is drawn
    auto titleRect = option.rect;
    titleRect.adjust(0, 0, -5 - unreadBadgeWidthWithMargin, 0);

    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->setRenderHint(QPainter::Antialiasing, true);

    // determine colors to use
    auto palette = parentTreeView->palette();
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

    // draw the icon
    if (index.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FEED)
    {
        QPixmap iconPixmap;
        auto iconVariant = index.data(SourceTreeEntryIcon);
        if (!iconVariant.isNull() && iconVariant.isValid())
        {
            iconPixmap = iconVariant.value<QPixmap>();
        }
        if (iconPixmap.isNull())
        {
            iconPixmap = QPixmap(":/rss.png");
        }
        auto iconSize = option.rect.height() * .75;
        auto iconTargetRect = QRectF(option.rect.left(), option.rect.top() + ((option.rect.height() - iconSize) / 2.0), iconSize, iconSize);
        painter->drawPixmap(iconTargetRect, iconPixmap, iconPixmap.rect());
        titleRect.adjust(static_cast<int32_t>(iconSize) + 9, 0, 0, 0);
    }

    // draw the title
    auto title = index.data(Qt::DisplayRole).toString();
    auto fm = QFontMetrics(painter->font());
    auto elidedTitle = fm.elidedText(title, Qt::ElideRight, titleRect.width());
    painter->setPen(QPen(brushText, 1.0));
    painter->drawText(titleRect, elidedTitle, titleTextOptions);

    // draw the unread amount badge
    if (parentTreeView->showUnreadBadges() && index.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FEED)
    {
        auto unreadCount = index.data(SourceTreeEntryUnreadCount).toULongLong();
        if (unreadCount > 0)
        {
            auto unreadCountString = QString::number(unreadCount);
            if (unreadCount > 9999)
            {
                unreadCountString = "9K+";
            }
            else if (unreadCount >= 1000)
            {
                float f = static_cast<float>(unreadCount) / 1000.0f;
                unreadCountString = QString("%1K").arg(QString::number(f, 'f', 1));
            }

            auto unreadRect = option.rect;
            unreadRect.moveLeft(option.rect.right() - unreadBadgeWidthWithMargin);
            unreadRect.moveTop(option.rect.top() + 2);
            unreadRect.setWidth(unreadBadgeWidth);
            unreadRect.setHeight(option.rect.height() - 4);

            QPainterPath path;
            path.addRoundedRect(unreadRect, 6, 6);
            painter->fillPath(path, brushUnreadBadgeBackground);

            painter->setPen(penUnreadBadgeText);
            painter->drawText(unreadRect, unreadCountString, unreadTextOptions);
        }
    }
}

QSize ZapFR::Client::ItemDelegateSource::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto s = QStyledItemDelegate::sizeHint(option, index);
    s.setHeight(s.height() + 8);
    return s;
}
