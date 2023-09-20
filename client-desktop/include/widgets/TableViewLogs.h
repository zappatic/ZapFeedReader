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

#include "ClientGlobal.h"
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

            void setMainWindow(MainWindow* mw) noexcept { mMainWindow = mw; }
            void connectStuff();

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
            };

          private:
            MainWindow* mMainWindow{nullptr};
            std::unique_ptr<QStandardItemModel> mItemModelLogs{nullptr};

            std::unique_ptr<QAction> mActionViewLogs{nullptr};
            std::unique_ptr<QAction> mActionClearLogs{nullptr};

            uint64_t mCurrentLogPage{1};
            uint64_t mCurrentLogCount{0};
            uint64_t mCurrentLogPageCount{1};

            void populateLogs(const QList<QList<QStandardItem*>>& logs = {}, uint64_t pageNumber = 1, uint64_t totalLogCount = 0);

            static constexpr uint64_t msLogsPerPage{100};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_TABLEVIEWLOGS_H
