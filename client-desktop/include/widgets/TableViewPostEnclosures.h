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
#include "ZapFR/base/Post.h"
#include "widgets/TableViewPaletteCorrected.h"

namespace ZapFR
{
    namespace Client
    {
        class PopupFlagChooser;

        class TableViewPostEnclosures : public TableViewPaletteCorrected
        {
            Q_OBJECT

          public:
            TableViewPostEnclosures(QWidget* parent = nullptr);
            ~TableViewPostEnclosures() = default;

            void clear();
            void loadEnclosures(const std::vector<ZapFR::Engine::Post::Enclosure>& enclosures);

            enum Column
            {
                IconCol = 0,
                URLCol = 1,
                MimetypeCol = 2,
                FilesizeCol = 3,
            };

            enum Role
            {
                Link = Qt::ItemDataRole::UserRole + 1,
            };

          private slots:
            void openEnclosureInExternalBrowser();
            void copyLink();

          private:
            std::unique_ptr<QStandardItemModel> mItemModelPostEnclosures{nullptr};

            std::unique_ptr<QMenu> mContextMenu{nullptr};
            std::unique_ptr<QAction> mCopyLinkAction{nullptr};
            std::unique_ptr<QAction> mOpenInBrowserAction{nullptr};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_TABLEVIEWPOSTSENCLOSURES_H
