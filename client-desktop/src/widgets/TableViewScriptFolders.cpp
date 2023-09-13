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

#include "widgets/TableViewScriptFolders.h"

ZapFR::Client::TableViewScriptFolders::TableViewScriptFolders(QWidget* parent) : TableViewPaletteCorrected(parent)
{
}

void ZapFR::Client::TableViewScriptFolders::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    QTableView::selectionChanged(selected, deselected);
    for (const auto& index : selectedIndexes())
    {
        if (index.column() == ScriptFolderColumnTitle)
        {
            emit selectedScriptFolderChanged(index);
        }
    }
}

void ZapFR::Client::TableViewScriptFolders::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
        emit deletePressed();
    }
    else
    {
        QTableView::keyPressEvent(event);
    }
}