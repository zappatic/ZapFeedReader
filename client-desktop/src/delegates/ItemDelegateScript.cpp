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

#include <QApplication>
#include <QPainter>
#include <cmath>

#include "Utilities.h"
#include "delegates/ItemDelegateScript.h"
#include "widgets/TableViewScripts.h"

ZapFR::Client::ItemDelegateScript::ItemDelegateScript(QObject* parent) : QStyledItemDelegate(parent)
{
}

void ZapFR::Client::ItemDelegateScript::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    static bool initDone{false};
    static auto titleTextOptions = QTextOption(Qt::AlignLeft | Qt::AlignVCenter);
    if (!initDone)
    {
        titleTextOptions.setWrapMode(QTextOption::NoWrap);
        initDone = true;
    }

    auto parentTableView = qobject_cast<TableViewScripts*>(parent());

    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->setRenderHint(QPainter::Antialiasing, true);
    auto regularFont = parentTableView->font();

    // determine colors to use
    auto palette = parentTableView->palette();
    QBrush brushBackground;
    QBrush brushText = palette.text();
    bool paintBackground{false};
    palette.setCurrentColorGroup((option.state & QStyle::State_Active) == QStyle::State_Active ? QPalette::ColorGroup::Active : QPalette::ColorGroup::Inactive);
    if ((option.state & QStyle::State_Selected) == QStyle::State_Selected)
    {
        paintBackground = true;
        brushBackground = palette.highlight();
        brushText = palette.highlightedText();
    }

    // paint the background
    if (paintBackground)
    {
        painter->fillRect(option.rect, brushBackground);
    }

    auto currentColumn = index.column();
    if (currentColumn == TableViewScripts::Column::RunOnEventsCol && index.data(TableViewScripts::Role::EventCount).toULongLong() == 0)
    {
        brushText = Qt::darkRed;
    }
    else if (currentColumn == TableViewScripts::Column::TitleCol && index.data(TableViewScripts::Role::Script).toString().isEmpty())
    {
        brushText = Qt::darkRed;
    }

    switch (currentColumn)
    {
        case TableViewScripts::Column::IsEnabledCol:
        {
            auto r = option.rect;
            auto w = std::floor(static_cast<float>(r.height()) * 0.75f);
            auto x = static_cast<float>(r.center().x()) - (w / 2.0f);
            auto y = static_cast<float>(r.center().y()) - (w / 2.0f);
            auto cbRect = QRectF(x, y, w, w);

            QStyleOptionButton checkbox;
            checkbox.rect = cbRect.toRect();
            checkbox.text = "";
            checkbox.state |= QStyle::State_Enabled;
            checkbox.state |= (index.data(TableViewScripts::Role::IsEnabled).toBool() ? QStyle::State_On : QStyle::State_Off);
            QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkbox, painter);
            break;
        }
        default:
        {
            auto titleRect = option.rect.adjusted(5, 0, -5, 0);
            auto title = index.data(Qt::DisplayRole).toString();
            painter->setPen(QPen(brushText, 1.0));
            auto fm = QFontMetrics(painter->font());
            auto elidedTitle = fm.elidedText(title, Qt::ElideRight, titleRect.width());
            painter->drawText(titleRect, elidedTitle, titleTextOptions);
            break;
        }
    }
}
