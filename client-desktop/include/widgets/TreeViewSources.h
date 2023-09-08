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

#ifndef ZAPFR_CLIENT_TREEVIEWSOURCES_H
#define ZAPFR_CLIENT_TREEVIEWSOURCES_H

#include "ClientGlobal.h"
#include "TreeViewPaletteCorrected.h"

namespace ZapFR
{
    namespace Client
    {
        class TreeViewSources : public TreeViewPaletteCorrected
        {
            Q_OBJECT

          public:
            TreeViewSources(QWidget* parent = nullptr);
            ~TreeViewSources() = default;

          signals:
            void currentSourceChanged(const QModelIndex&);
            void deletePressed();
            void folderDoubleClicked();

          protected:
            void currentChanged(const QModelIndex& current, const QModelIndex& previous) override;
            void keyPressEvent(QKeyEvent* event) override;
            void mouseDoubleClickEvent(QMouseEvent* event) override;
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_TREEVIEWSOURCES_H
