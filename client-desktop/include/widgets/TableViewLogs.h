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

#ifndef ZAPFR_CLIENT_TABLEVIEWLOGS_H
#define ZAPFR_CLIENT_TABLEVIEWLOGS_H

#include <QStandardItemModel>

#include "widgets/TableViewPaletteCorrected.h"

namespace ZapFR
{
    namespace Client
    {
        class MainWindow;

        class TableViewLogs : public TableViewPaletteCorrected
        {
            Q_OBJECT

          public:
            TableViewLogs(QWidget* parent = nullptr);
            ~TableViewLogs() = default;
            TableViewLogs(const TableViewLogs& e) = delete;
            TableViewLogs& operator=(const TableViewLogs&) = delete;
            TableViewLogs(TableViewLogs&&) = delete;
            TableViewLogs& operator=(TableViewLogs&&) = delete;

            void setMainWindow(MainWindow* mw) noexcept;

            void reload();
            void setCurrentLogPage(uint64_t page) noexcept { mCurrentLogPage = page; }
            QAction* actionViewLogs() const noexcept { return mActionViewLogs.get(); }
            QAction* actionClearLogs() const noexcept { return mActionClearLogs.get(); }

            enum Column
            {
                LogLevelCol = 0,
                FeedCol = 1,
                TimestampCol = 2,
                MessageCol = 3,
            };

            enum Role
            {
                ID = Qt::ItemDataRole::UserRole + 1,
                FeedID = Qt::ItemDataRole::UserRole + 2,
                Level = Qt::ItemDataRole::UserRole + 3,
                ParentSourceID = Qt::ItemDataRole::UserRole + 4,
                Message = Qt::ItemDataRole::UserRole + 5,
                Timestamp = Qt::ItemDataRole::UserRole + 6,
            };

          protected:
            void keyPressEvent(QKeyEvent* event) override;

          private slots:
            void copyMessages();

          private:
            MainWindow* mMainWindow{nullptr};
            std::unique_ptr<QStandardItemModel> mItemModelLogs{nullptr};
            std::unique_ptr<QMenu> mLogsContextMenu{nullptr};

            std::unique_ptr<QAction> mActionViewLogs{nullptr};
            std::unique_ptr<QAction> mActionClearLogs{nullptr};
            std::unique_ptr<QAction> mActionCopyMessages{nullptr};

            uint64_t mCurrentLogPage{1};
            uint64_t mCurrentLogCount{0};
            uint64_t mCurrentLogPageCount{1};

            void populateLogs(const QList<QList<QStandardItem*>>& logs = {}, uint64_t pageNumber = 1, uint64_t totalLogCount = 0);
            void connectStuff();
            void createContextMenu();

            static constexpr uint64_t msLogsPerPage{100};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_TABLEVIEWLOGS_H
