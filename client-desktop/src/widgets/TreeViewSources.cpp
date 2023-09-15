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

#include "widgets/TreeViewSources.h"

ZapFR::Client::TreeViewSources::TreeViewSources(QWidget* parent) : TreeViewPaletteCorrected(parent)
{
    connect(this, &QTreeView::expanded,
            [&](const QModelIndex& index)
            {
                if ((QGuiApplication::keyboardModifiers() & Qt::ControlModifier) == Qt::ControlModifier)
                {
                    expandRecursively(index);
                }
            });
}

void ZapFR::Client::TreeViewSources::currentChanged(const QModelIndex& current, const QModelIndex& /*previous*/)
{
    emit currentSourceChanged(current);
}

void ZapFR::Client::TreeViewSources::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
        emit deletePressed();
    }
    else
    {
        QTreeView::keyPressEvent(event);
    }
}

void ZapFR::Client::TreeViewSources::mouseDoubleClickEvent(QMouseEvent* event)
{
    auto index = currentIndex();
    if (index.isValid())
    {
        auto type = index.data(SourceTreeEntryTypeRole).toULongLong();
        if (type == SOURCETREE_ENTRY_TYPE_FOLDER)
        {
            emit folderDoubleClicked();
            return;
        }
    }

    TreeViewPaletteCorrected::mouseDoubleClickEvent(event);
}
