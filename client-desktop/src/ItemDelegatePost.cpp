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

#include "ItemDelegatePost.h"
#include "FeedIconCache.h"
#include "TableViewPosts.h"
#include "Utilities.h"

ZapFR::Client::ItemDelegatePost::ItemDelegatePost(QObject* parent) : QStyledItemDelegate(parent)
{
}

void ZapFR::Client::ItemDelegatePost::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    static bool initDone{false};
    static auto titleTextOptions = QTextOption(Qt::AlignLeft | Qt::AlignVCenter);
    if (!initDone)
    {
        titleTextOptions.setWrapMode(QTextOption::NoWrap);
        initDone = true;
    }

    auto parentTableView = qobject_cast<TableViewPosts*>(parent());

    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->setRenderHint(QPainter::Antialiasing, true);
    auto regularFont = parentTableView->font();
    auto boldFont = regularFont;
    boldFont.setBold(true);

    // determine colors to use
    auto palette = parentTableView->palette();
    QBrush brushBackground;
    QBrush brushUnreadIndicator = palette.highlight();
    QBrush brushText = palette.text();
    bool paintBackground{false};
    palette.setCurrentColorGroup((option.state & QStyle::State_Active) == QStyle::State_Active ? QPalette::ColorGroup::Active : QPalette::ColorGroup::Inactive);
    if ((option.state & QStyle::State_Selected) == QStyle::State_Selected)
    {
        paintBackground = true;
        brushBackground = palette.highlight();
        brushUnreadIndicator = palette.text();
        brushText = palette.highlightedText();
    }

    // paint the background
    if (paintBackground)
    {
        painter->fillRect(option.rect, brushBackground);
    }

    auto currentColumn = index.column();
    switch (currentColumn)
    {
        case PostColumnUnread:
        {
            if (!index.data(PostIsReadRole).toBool())
            {
                QPainterPath p;
                p.addEllipse(Utilities::centeredSquareInRectangle(option.rect, 0.4f));
                painter->setBrush(brushUnreadIndicator);
                painter->setPen(Qt::PenStyle::NoPen);
                painter->drawPath(p);
            }
            break;
        }
        case PostColumnFeed:
        {
            auto pixmap = FeedIconCache::icon(index.data(PostFeedIDRole).toULongLong());
            if (!pixmap.isNull())
            {
                painter->drawPixmap(Utilities::centeredSquareInRectangle(option.rect, 0.6f), pixmap, pixmap.rect());
            }
            break;
        }
        case PostColumnFlag:
        {
            QPixmap flagToDraw;
            auto flagColors = index.data(PostAppliedFlagsRole).toList();
            if (flagColors.count() > 0)
            {
                auto mainFlagColor = static_cast<ZapFR::Engine::FlagColor>(flagColors.at(0).toUInt());
                flagToDraw = Utilities::flag(mainFlagColor, Utilities::FlagStyle::Filled);
            }
            else if ((option.state & QStyle::State_MouseOver) == QStyle::State_MouseOver)
            {
                flagToDraw = Utilities::flag(ZapFR::Engine::FlagColor::Gray, Utilities::FlagStyle::Unfilled);
            }

            if (!flagToDraw.isNull())
            {
                painter->drawPixmap(Utilities::centeredSquareInRectangle(option.rect, 0.75f), flagToDraw, flagToDraw.rect());
            }
            break;
        }
        default:
        {
            auto titleRect = option.rect.adjusted(5, 0, -5, 0);
            auto title = index.data(Qt::DisplayRole).toString();
            auto isRead = index.data(PostIsReadRole).toBool();
            static auto whitespaceRe = QRegularExpression(R"(\s+)");
            title.replace(whitespaceRe, " ");
            painter->setPen(QPen(brushText, 1.0));
            painter->setFont(isRead ? regularFont : boldFont);
            auto fm = QFontMetrics(painter->font());
            auto elidedTitle = fm.elidedText(title, Qt::ElideRight, titleRect.width());
            painter->drawText(titleRect, elidedTitle, titleTextOptions);
            break;
        }
    }
}
