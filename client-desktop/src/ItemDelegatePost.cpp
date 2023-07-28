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

    painter->setRenderHint(QPainter::Antialiasing);
    auto regularFont = qobject_cast<QWidget*>(parent())->font();
    auto boldFont = regularFont;
    boldFont.setBold(true);

    // determine colors to use
    auto palette = qobject_cast<QWidget*>(parent())->palette();
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

    if (index.column() == 0)
    {
        if (!index.data(PostIsReadRole).toBool())
        {
            auto targetWidth = static_cast<float>(option.rect.height()) * 0.4f;
            auto targetX = static_cast<float>(option.rect.left()) + ((static_cast<float>(option.rect.width()) / 2.0f) - (targetWidth / 2.0f));
            auto targetY = option.rect.top() + ((option.rect.height() / 2.0) - (targetWidth / 2.0f));
            auto target = QRectF(targetX, targetY, targetWidth, targetWidth);
            QPainterPath p;
            p.addEllipse(target);
            painter->setBrush(brushUnreadIndicator);
            painter->setPen(Qt::PenStyle::NoPen);
            painter->drawPath(p);
        }
    }
    else
    {
        // draw the title
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
    }
}
