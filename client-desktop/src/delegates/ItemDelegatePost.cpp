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

#include <QPainter>
#include <QPainterPath>

#include "FeedIconCache.h"
#include "Utilities.h"
#include "delegates/ItemDelegatePost.h"
#include "widgets/TableViewPosts.h"

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
        case TableViewPosts::Column::UnreadCol:
        {
            if (!index.data(TableViewPosts::Role::IsRead).toBool())
            {
                QPainterPath p;
                p.addEllipse(Utilities::centeredSquareInRectangle(option.rect, 0.4f));
                painter->setBrush(brushUnreadIndicator);
                painter->setPen(Qt::PenStyle::NoPen);
                painter->drawPath(p);
            }
            break;
        }
        case TableViewPosts::Column::FeedCol:
        {
            auto pixmap = FeedIconCache::icon(index.data(TableViewPosts::Role::SourceID).toULongLong(), index.data(TableViewPosts::Role::FeedID).toULongLong());
            if (!pixmap.isNull())
            {
                painter->drawPixmap(Utilities::centeredSquareInRectangle(option.rect, 0.6f), pixmap, pixmap.rect());
            }
            break;
        }
        case TableViewPosts::Column::FlagCol:
        {
            auto isHovering = ((option.state & QStyle::State_MouseOver) == QStyle::State_MouseOver);
            auto flagColors = index.data(TableViewPosts::Role::AppliedFlags).toList();
            renderFlags(painter, isHovering, flagColors, option.rect);
            break;
        }
        default:
        {
            auto titleRect = option.rect.adjusted(5, 0, -5, 0);
            auto title = index.data(Qt::DisplayRole).toString();
            auto isRead = index.data(TableViewPosts::Role::IsRead).toBool();
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

void ZapFR::Client::ItemDelegatePost::renderFlags(QPainter* painter, bool isHovering, QVariantList& flagsToRender, const QRect& destinationRect) const
{
    auto flagCount = flagsToRender.size();
    if (flagCount == 0 && isHovering)
    {
        auto flag = Utilities::flag(ZapFR::Engine::FlagColor::Gray, Utilities::FlagStyle::Unfilled);
        painter->drawPixmap(Utilities::centeredSquareInRectangle(destinationRect, FlagHeightFactor), flag, flag.rect());
    }

    if (flagCount > 0)
    {
        // sort the flags so they are always in the same color order
        std::sort(flagsToRender.begin(), flagsToRender.end(), [](const QVariant& a, const QVariant& b) { return a.toUInt() < b.toUInt(); });

        // determine whether to draw the flags centered in the column (if there is room to spare and all can fit in the available space)
        // or to just draw the from the left boundary onwards
        auto flagWidth = FlagHeightFactor * static_cast<float>(destinationRect.height());
        auto visibleFlagCount = static_cast<int64_t>(std::floor(static_cast<float>(destinationRect.width()) / flagWidth));
        auto shouldCenter = (flagCount <= visibleFlagCount);

        // calculate the starting x-coord and y coord of the flags
        float top = static_cast<float>(destinationRect.top()) + (static_cast<float>(destinationRect.height()) / 2.0f) - (flagWidth / 2.0f);
        float left = static_cast<float>(destinationRect.left());
        if (shouldCenter)
        {
            left = static_cast<float>(destinationRect.left()) + (static_cast<float>(destinationRect.width()) / 2.0f) - ((static_cast<float>(flagCount) * flagWidth) / 2.0f);
        }

        painter->save();
        painter->setClipRect(destinationRect);
        for (const auto& flagColorVariant : flagsToRender)
        {
            auto flagColor = static_cast<ZapFR::Engine::FlagColor>(flagColorVariant.toUInt());
            auto flag = Utilities::flag(flagColor, Utilities::FlagStyle::Filled);
            auto r = QRectF(left, top, flagWidth, flagWidth);
            painter->drawPixmap(r, flag, flag.rect());

            left += flagWidth;
        }
        painter->restore();
    }
}
