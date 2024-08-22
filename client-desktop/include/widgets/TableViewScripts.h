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

#ifndef ZAPFR_CLIENT_TABLEVIEWSCRIPTS_H
#define ZAPFR_CLIENT_TABLEVIEWSCRIPTS_H

#include <QKeyEvent>
#include <QStandardItemModel>

#include "ZapFR/base/Script.h"
#include "widgets/TableViewPaletteCorrected.h"

Q_DECLARE_METATYPE(ZapFR::Engine::Script::Event)

namespace ZapFR
{
    namespace Client
    {
        class MainWindow;
        class DialogEditScript;

        class TableViewScripts : public TableViewPaletteCorrected
        {
            Q_OBJECT

          public:
            TableViewScripts(QWidget* parent = nullptr);
            ~TableViewScripts() = default;
            TableViewScripts(const TableViewScripts& e) = delete;
            TableViewScripts& operator=(const TableViewScripts&) = delete;
            TableViewScripts(TableViewScripts&&) = delete;
            TableViewScripts& operator=(TableViewScripts&&) = delete;

            void setMainWindow(MainWindow* mw) noexcept { mMainWindow = mw; }
            void reload(bool forceReload = false);
            QAction* actionViewScripts() const noexcept { return mActionViewScripts.get(); }
            QAction* actionAddScript() const noexcept { return mActionAddScript.get(); }
            QAction* actionEditScript() const noexcept { return mActionEditScript.get(); }
            QAction* actionRemoveScript() const noexcept { return mActionRemoveScript.get(); }

            enum Column
            {
                TypeCol = 0,
                TitleCol = 1,
                IsEnabledCol = 2,
                RunOnEventsCol = 3,
                RunOnFeedIDsCol = 4,
            };

            enum Role
            {
                ID = Qt::ItemDataRole::UserRole + 1,
                IsEnabled = Qt::ItemDataRole::UserRole + 2,
                SourceID = Qt::ItemDataRole::UserRole + 3,
                RunOnEvents = Qt::ItemDataRole::UserRole + 5,
                RunOnFeedIDs = Qt::ItemDataRole::UserRole + 6,
                EventCount = Qt::ItemDataRole::UserRole + 7,
                Title = Qt::ItemDataRole::UserRole + 8,
                Script = Qt::ItemDataRole::UserRole + 9,
            };

          protected:
            void keyPressEvent(QKeyEvent* event) override;

          private slots:
            void editScript();
            void removeScript();
            void addScript();

          private:
            MainWindow* mMainWindow{nullptr};
            std::unique_ptr<QStandardItemModel> mItemModelScripts{nullptr};
            std::unique_ptr<DialogEditScript> mDialogEditScript{nullptr};
            std::unique_ptr<QMenu> mScriptContextMenu{nullptr};

            std::unique_ptr<QAction> mActionViewScripts{nullptr};
            std::unique_ptr<QAction> mActionAddScript{nullptr};
            std::unique_ptr<QAction> mActionEditScript{nullptr};
            std::unique_ptr<QAction> mActionRemoveScript{nullptr};

            void populateScripts(const QList<QList<QStandardItem*>>& scripts = {});
            DialogEditScript* editScriptDialog();
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_TABLEVIEWSCRIPTS_H
