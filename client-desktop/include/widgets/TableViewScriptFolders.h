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

#ifndef ZAPFR_CLIENT_TABLEVIEWSCRIPTFOLDERS_H
#define ZAPFR_CLIENT_TABLEVIEWSCRIPTFOLDERS_H

#include <QStandardItemModel>

#include "widgets/TableViewPaletteCorrected.h"

namespace ZapFR
{
    namespace Client
    {
        class MainWindow;
        class DialogEditScriptFolder;

        class TableViewScriptFolders : public TableViewPaletteCorrected
        {
            Q_OBJECT

          public:
            TableViewScriptFolders(QWidget* parent = nullptr);
            ~TableViewScriptFolders() = default;
            TableViewScriptFolders(const TableViewScriptFolders& e) = delete;
            TableViewScriptFolders& operator=(const TableViewScriptFolders&) = delete;
            TableViewScriptFolders(TableViewScriptFolders&&) = delete;
            TableViewScriptFolders& operator=(TableViewScriptFolders&&) = delete;

            void setMainWindow(MainWindow* mw) noexcept { mMainWindow = mw; }
            void reload(bool forceReload = false);
            void refreshBadges();
            QAction* actionAddScriptFolder() const noexcept { return mActionAddScriptFolder.get(); }
            QAction* actionEditScriptFolder() const noexcept { return mActionEditScriptFolder.get(); }
            QAction* actionRemoveScriptFolder() const noexcept { return mActionRemoveScriptFolder.get(); }
            DialogEditScriptFolder* editScriptFolderDialog();

            std::unordered_map<uint64_t, QString> getIDToTitleMapping() const;

            enum Column
            {
                TitleCol = 0,
            };

            enum Role
            {
                ID = Qt::ItemDataRole::UserRole + 1,
                SourceID = Qt::ItemDataRole::UserRole + 2,
                ShowTotal = Qt::ItemDataRole::UserRole + 3,
                ShowUnread = Qt::ItemDataRole::UserRole + 4,
                TotalPostCount = Qt::ItemDataRole::UserRole + 5,
                TotalUnreadCount = Qt::ItemDataRole::UserRole + 6,
            };

          protected:
            void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;
            void keyPressEvent(QKeyEvent* event) override;

          private slots:
            void addScriptFolder();
            void editScriptFolder();
            void removeScriptFolder();

          private:
            uint64_t mPreviouslySelectedScriptFolderID{0};
            MainWindow* mMainWindow{nullptr};
            std::unique_ptr<QStandardItemModel> mItemModelScriptFolders{nullptr};
            std::unique_ptr<QMenu> mScriptFolderContextMenu{nullptr};
            std::unique_ptr<DialogEditScriptFolder> mDialogEditScriptFolder{nullptr};

            std::unique_ptr<QAction> mActionAddScriptFolder{nullptr};
            std::unique_ptr<QAction> mActionEditScriptFolder{nullptr};
            std::unique_ptr<QAction> mActionRemoveScriptFolder{nullptr};

            void populateScriptFolders(uint64_t sourceID, const QList<QList<QStandardItem*>>& scriptFolders);
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_TABLEVIEWSCRIPTFOLDERS_H
