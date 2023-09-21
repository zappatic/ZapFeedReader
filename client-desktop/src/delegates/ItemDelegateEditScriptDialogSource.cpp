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

#include "delegates/ItemDelegateEditScriptDialogSource.h"
#include "FeedIconCache.h"
#include "widgets/TreeViewSources.h"

ZapFR::Client::ItemDelegateEditScriptDialogSource::ItemDelegateEditScriptDialogSource(QObject* parent) : QStyledItemDelegate(parent)
{
}

void ZapFR::Client::ItemDelegateEditScriptDialogSource::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    static bool initDone{false};
    static auto titleTextOptions = QTextOption(Qt::AlignLeft | Qt::AlignVCenter);
    if (!initDone)
    {
        titleTextOptions.setWrapMode(QTextOption::NoWrap);
        initDone = true;
    }

    auto parentTreeView = qobject_cast<QTreeView*>(parent());

    // the area where the title is drawn
    auto titleRect = option.rect;

    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->setRenderHint(QPainter::Antialiasing, true);

    // determine colors to use
    auto palette = parentTreeView->palette();
    auto brushText = palette.text();
    palette.setCurrentColorGroup((option.state & QStyle::State_Active) == QStyle::State_Active ? QPalette::ColorGroup::Active : QPalette::ColorGroup::Inactive);
    if ((option.state & QStyle::State_Selected) == QStyle::State_Selected)
    {
        brushText = palette.highlightedText();
    }

    // draw the checkbox
    if (index.data(TreeViewSources::Role::Type).toULongLong() == TreeViewSources::EntryType::Feed)
    {
        auto r = option.rect;
        auto w = std::floor(static_cast<float>(r.height()) * 0.75f);
        auto x = static_cast<float>(r.left());
        auto y = static_cast<float>(r.center().y()) - (w / 2.0f);
        auto cbRect = QRectF(x, y, w, w);

        QStyleOptionButton checkbox;
        checkbox.rect = cbRect.toRect();
        checkbox.text = "";
        checkbox.state |= QStyle::State_Enabled;
        checkbox.state |= ((index.data(Qt::CheckStateRole) == Qt::Checked || !parentTreeView->isEnabled()) ? QStyle::State_On : QStyle::State_Off);
        QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkbox, painter);
        titleRect.adjust(static_cast<int32_t>(w) + 9, 0, 0, 0);
    }

    // draw the icon
    if (index.data(TreeViewSources::Role::Type) == TreeViewSources::EntryType::Feed)
    {
        QPixmap icon;
        if (parentTreeView->isEnabled())
        {
            icon = FeedIconCache::icon(index.data(TreeViewSources::Role::ParentSourceID).toULongLong(), index.data(TreeViewSources::Role::ID).toULongLong());
        }
        else
        {
            icon = FeedIconCache::iconGrayscale(index.data(TreeViewSources::Role::ParentSourceID).toULongLong(), index.data(TreeViewSources::Role::ID).toULongLong());
        }
        auto iconSize = option.rect.height() * .75;
        auto iconTargetRect = QRectF(titleRect.left(), option.rect.top() + ((option.rect.height() - iconSize) / 2.0), iconSize, iconSize);
        painter->drawPixmap(iconTargetRect, icon, icon.rect());
        titleRect.adjust(static_cast<int32_t>(iconSize) + 9, 0, 0, 0);
    }

    // draw the title
    auto title = index.data(Qt::DisplayRole).toString();
    auto fm = QFontMetrics(painter->font());
    auto elidedTitle = fm.elidedText(title, Qt::ElideRight, titleRect.width());
    painter->setPen(QPen(brushText, 1.0));
    painter->drawText(titleRect, elidedTitle, titleTextOptions);
}

QSize ZapFR::Client::ItemDelegateEditScriptDialogSource::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto s = QStyledItemDelegate::sizeHint(option, index);
    s.setHeight(s.height() + 8);
    return s;
}
