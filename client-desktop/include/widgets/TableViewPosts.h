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

#ifndef ZAPFR_CLIENT_TABLEVIEWPOSTS_H
#define ZAPFR_CLIENT_TABLEVIEWPOSTS_H

#include "ClientGlobal.h"
#include "TableViewPaletteCorrected.h"
#include "Utilities.h"
#include "ZapFR/Flag.h"

namespace ZapFR
{
    namespace Client
    {
        class PopupFlagChooser;

        class TableViewPosts : public TableViewPaletteCorrected
        {
            Q_OBJECT

          public:
            TableViewPosts(QWidget* parent = nullptr);
            ~TableViewPosts() = default;

          signals:
            void selectedPostsChanged(const QModelIndexList&);
            void postMarkedFlagged(uint64_t sourceID, uint64_t feedID, uint64_t postID, ZapFR::Engine::FlagColor flagColor);
            void postMarkedUnflagged(uint64_t sourceID, uint64_t feedID, uint64_t postID, ZapFR::Engine::FlagColor flagColor);
            void clearAllFlagsRequested(uint64_t sourceID, uint64_t feedID, uint64_t postID);

          private slots:
            void processFlagToggle(ZapFR::Engine::FlagColor flagColor, Utilities::FlagStyle flagStyle);
            void openPostInExternalBrowser();

          protected:
            void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;
            void mouseMoveEvent(QMouseEvent* event) override;
            void mouseReleaseEvent(QMouseEvent* event) override;
            void keyPressEvent(QKeyEvent* event) override;
            void paintEvent(QPaintEvent* event) override;

          private:
            std::unique_ptr<PopupFlagChooser> mPopupFlagChooser{nullptr};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_TABLEVIEWPOSTS_H
