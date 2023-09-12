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

#ifndef ZAPFR_CLIENT_TABLEVIEWPOSTSENCLOSURES_H
#define ZAPFR_CLIENT_TABLEVIEWPOSTSENCLOSURES_H

#include "ClientGlobal.h"

namespace ZapFR
{
    namespace Client
    {
        class PopupFlagChooser;

        class TableViewPostEnclosures : public QTableView
        {
            Q_OBJECT

          public:
            TableViewPostEnclosures(QWidget* parent = nullptr);
            ~TableViewPostEnclosures() = default;

          private slots:
            void openEnclosureInExternalBrowser();
            void copyLink();

          private:
            std::unique_ptr<QMenu> mContextMenu{nullptr};
            std::unique_ptr<QAction> mCopyLinkAction{nullptr};
            std::unique_ptr<QAction> mOpenInBrowserAction{nullptr};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_TABLEVIEWPOSTSENCLOSURES_H
