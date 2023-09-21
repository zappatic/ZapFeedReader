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

#include "widgets/TreeViewEditScriptDialogSources.h"
#include "widgets/TreeViewSources.h"

ZapFR::Client::TreeViewEditScriptDialogSources::TreeViewEditScriptDialogSources(QWidget* parent) : TreeViewPaletteCorrected(parent)
{
}

void ZapFR::Client::TreeViewEditScriptDialogSources::mousePressEvent(QMouseEvent* event)
{
    auto index = indexAt(event->pos());
    if (index.isValid() && index.data(TreeViewSources::Role::Type).toULongLong() == TreeViewSources::EntryType::Feed)
    {
        emit feedClicked(index);
    }
    else
    {
        TreeViewPaletteCorrected::mousePressEvent(event);
    }
}
