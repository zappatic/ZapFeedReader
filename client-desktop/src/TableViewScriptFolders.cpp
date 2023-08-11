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

#include "TableViewScriptFolders.h"

ZapFR::Client::TableViewScriptFolders::TableViewScriptFolders(QWidget* parent) : QTableView(parent)
{
    // overwrite the inactive palette with the active palette colors to get rid of the stupid unreadable gray on blue text when focus is lost
    auto p = palette();
    p.setColor(QPalette::Inactive, QPalette::Highlight, p.color(QPalette::Active, QPalette::Highlight));
    p.setColor(QPalette::Inactive, QPalette::HighlightedText, p.color(QPalette::Active, QPalette::HighlightedText));
    p.setColor(QPalette::Inactive, QPalette::Button, p.color(QPalette::Active, QPalette::Button));
    p.setColor(QPalette::Inactive, QPalette::ButtonText, p.color(QPalette::Active, QPalette::ButtonText));
    setPalette(p);
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
