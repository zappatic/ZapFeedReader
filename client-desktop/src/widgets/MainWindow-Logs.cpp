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

#include "./ui_MainWindow.h"
#include "widgets/MainWindow.h"
#include "ZapFR/Agent.h"
#include "ZapFR/Log.h"

void ZapFR::Client::MainWindow::reloadLogs()
{
    // lambda for the callback, retrieving the logs
    auto processLogs = [&](uint64_t /*sourceID*/, const std::vector<ZapFR::Engine::Log*> logs, uint64_t page, uint64_t totalRecordCount)
    {
        QList<QList<QStandardItem*>> rows;
        for (const auto& log : logs)
        {
            auto logLevelItem = new QStandardItem("");
            auto logLevel = log->level();
            logLevelItem->setData(QVariant::fromValue<uint64_t>(logLevel), LogLevelRole);
            switch (logLevel)
            {
                case ZapFR::Engine::LogLevel::Debug:
                {
                    logLevelItem->setData(tr("Debug"), Qt::ToolTipRole);
                    break;
                }
                case ZapFR::Engine::LogLevel::Info:
                {
                    logLevelItem->setData(tr("Info"), Qt::ToolTipRole);
                    break;
                }
                case ZapFR::Engine::LogLevel::Warning:
                {
                    logLevelItem->setData(tr("Warning"), Qt::ToolTipRole);
                    break;
                }
                case ZapFR::Engine::LogLevel::Error:
                {
                    logLevelItem->setData(tr("Error"), Qt::ToolTipRole);
                    break;
                }
            }

            auto dateLog = QString::fromUtf8(log->timestamp());
            auto dateItem = new QStandardItem(Utilities::prettyDate(dateLog));
            dateItem->setData(QVariant::fromValue<uint64_t>(log->id()), LogIDRole);

            auto feedItem = new QStandardItem("");
            if (log->feedID().has_value())
            {
                feedItem->setData(QVariant::fromValue<uint64_t>(log->feedID().value()), LogFeedIDRole);
            }
            if (log->feedTitle().has_value())
            {
                feedItem->setData(QString::fromUtf8(log->feedTitle().value()), Qt::ToolTipRole);
            }

            auto message = QString::fromUtf8(log->message());
            static auto whitespaceRe = QRegularExpression(R"(\s+)");
            message.replace(whitespaceRe, " ");
            auto titleItem = new QStandardItem(message);

            QList<QStandardItem*> rowData;
            rowData << logLevelItem << feedItem << dateItem << titleItem;
            rows << rowData;
        }

        QMetaObject::invokeMethod(this, "populateLogs", Qt::AutoConnection, rows, page, totalRecordCount);
    };

    auto index = ui->treeViewSources->currentIndex();
    if (index.isValid())
    {
        if (index.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FEED)
        {
            auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
            auto feedID = index.data(SourceTreeEntryIDRole).toULongLong();
            ZapFR::Engine::Agent::getInstance()->queueGetFeedLogs(sourceID, feedID, msLogsPerPage, mCurrentLogPage, processLogs);
        }
        else if (index.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FOLDER)
        {
            auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
            auto folderID = index.data(SourceTreeEntryIDRole).toULongLong();
            ZapFR::Engine::Agent::getInstance()->queueGetFolderLogs(sourceID, folderID, msLogsPerPage, mCurrentLogPage, processLogs);
        }
        else if (index.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_SOURCE)
        {
            auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
            ZapFR::Engine::Agent::getInstance()->queueGetSourceLogs(sourceID, msLogsPerPage, mCurrentLogPage, processLogs);
        }
        else
        {
            populateLogs();
        }
    }
}

void ZapFR::Client::MainWindow::populateLogs(const QList<QList<QStandardItem*>>& logs, uint64_t pageNumber, uint64_t totalLogCount)
{
    ui->stackedWidgetRight->setCurrentIndex(StackedPaneLogs);

    mItemModelLogs = std::make_unique<QStandardItemModel>(this);
    ui->tableViewLogs->setModel(mItemModelLogs.get());
    mItemModelLogs->setHorizontalHeaderItem(LogsColumnLogLevel, new QStandardItem(tr("Level")));
    mItemModelLogs->setHorizontalHeaderItem(LogsColumnTimestamp, new QStandardItem(tr("Timestamp")));
    mItemModelLogs->setHorizontalHeaderItem(LogsColumnFeed, new QStandardItem(tr("Feed")));
    mItemModelLogs->setHorizontalHeaderItem(LogsColumnMessage, new QStandardItem(tr("Message")));
    for (const auto& log : logs)
    {
        mItemModelLogs->appendRow(log);
    }
    ui->tableViewLogs->horizontalHeader()->setSectionResizeMode(LogsColumnMessage, QHeaderView::Stretch);
    ui->tableViewLogs->horizontalHeader()->resizeSection(LogsColumnTimestamp, 200);
    ui->tableViewLogs->horizontalHeader()->resizeSection(LogsColumnFeed, 40);
    ui->tableViewLogs->horizontalHeader()->resizeSection(LogsColumnLogLevel, 40);

    mCurrentLogCount = totalLogCount;
    mCurrentLogPage = pageNumber;
    mCurrentLogPageCount = 1;
    if (mCurrentLogCount > 0)
    {
        mCurrentLogPageCount = static_cast<uint64_t>(std::ceil(static_cast<float>(mCurrentLogCount) / static_cast<float>(msLogsPerPage)));
    }

    ui->pushButtonLogPageNumber->setText(QString("%1 %2 / %3").arg(tr("Page")).arg(mCurrentLogPage).arg(mCurrentLogPageCount));
}

void ZapFR::Client::MainWindow::connectLogsStuff()
{
    connect(ui->action_View_logs, &QAction::triggered,
            [&]()
            {
                mCurrentLogPage = 1;
                reloadLogs();
            });

    connect(ui->pushButtonLogPreviousPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentLogPage = std::max(1ul, mCurrentLogPage - 1);
                reloadLogs();
            });

    connect(ui->pushButtonLogNextPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentLogPage = std::min(mCurrentLogPageCount, mCurrentLogPage + 1);
                reloadLogs();
            });

    connect(ui->pushButtonLogFirstPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentLogPage = 1;
                reloadLogs();
            });

    connect(ui->pushButtonLogLastPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentLogPage = mCurrentLogPageCount;
                reloadLogs();
            });

    connect(ui->pushButtonLogPageNumber, &QPushButton::clicked,
            [&]()
            {
                showJumpToPageDialog(mCurrentLogPage, mCurrentLogPageCount,
                                     [&](uint64_t page)
                                     {
                                         mCurrentLogPage = page;
                                         reloadLogs();
                                     });
            });
}
