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

#include "widgets/TreeViewPaletteCorrected.h"

ZapFR::Client::TreeViewPaletteCorrected::TreeViewPaletteCorrected(QWidget* parent) : QTreeView(parent)
{
}

bool ZapFR::Client::TreeViewPaletteCorrected::viewportEvent(QEvent* event)
{
    if (event->type() == QEvent::ToolTip)
    {
        // workaround for qt bug (?) showing tooltip text as gray on light yellow background in dark mode
        auto tooltipPalette = QToolTip::palette();
        tooltipPalette.setColor(QPalette::Inactive, QPalette::ToolTipText, Qt::black);
        QToolTip::setPalette(tooltipPalette);
    }
    return QTreeView::viewportEvent(event);
}
