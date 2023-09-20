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

#include "widgets/TableViewLogs.h"
#include "./ui_MainWindow.h"
#include "Utilities.h"
#include "ZapFR/Agent.h"
#include "ZapFR/Log.h"
#include "delegates/ItemDelegateLog.h"
#include "widgets/MainWindow.h"
#include "widgets/TreeViewSources.h"

ZapFR::Client::TableViewLogs::TableViewLogs(QWidget* parent) : TableViewPaletteCorrected(parent)
{
    setItemDelegate(new ItemDelegateLog(this));
    mItemModelLogs = std::make_unique<QStandardItemModel>(this);
    setModel(mItemModelLogs.get());

    mActionViewLogs = std::make_unique<QAction>(tr("View logs"), this);
    mActionClearLogs = std::make_unique<QAction>(tr("Clear logs"), this);
}

void ZapFR::Client::TableViewLogs::reload()
{
    // lambda for the callback, retrieving the logs
    auto processLogs = [&](uint64_t sourceID, const std::vector<ZapFR::Engine::Log*> logs, uint64_t page, uint64_t totalRecordCount)
    {
        QList<QList<QStandardItem*>> rows;
        for (const auto& log : logs)
        {
            auto logLevelItem = new QStandardItem("");
            auto logLevel = log->level();
            logLevelItem->setData(QVariant::fromValue<uint64_t>(logLevel), Role::Level);
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
            dateItem->setData(QVariant::fromValue<uint64_t>(log->id()), Role::ID);

            auto feedItem = new QStandardItem("");
            if (log->feedID().has_value())
            {
                feedItem->setData(QVariant::fromValue<uint64_t>(log->feedID().value()), Role::FeedID);
                feedItem->setData(QVariant::fromValue<uint64_t>(sourceID), Role::ParentSourceID);
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

        QMetaObject::invokeMethod(this, [=, this]() { populateLogs(rows, page, totalRecordCount); });
    };

    auto index = mMainWindow->getUI()->treeViewSources->currentIndex();
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

void ZapFR::Client::TableViewLogs::populateLogs(const QList<QList<QStandardItem*>>& logs, uint64_t pageNumber, uint64_t totalLogCount)
{
    mMainWindow->setContentPane(StackedPaneLogs);

    mItemModelLogs->clear();
    mItemModelLogs->setHorizontalHeaderItem(Column::LogLevelCol, new QStandardItem(tr("Level")));
    mItemModelLogs->setHorizontalHeaderItem(Column::TimestampCol, new QStandardItem(tr("Timestamp")));
    mItemModelLogs->setHorizontalHeaderItem(Column::FeedCol, new QStandardItem(tr("Feed")));
    mItemModelLogs->setHorizontalHeaderItem(Column::MessageCol, new QStandardItem(tr("Message")));
    for (const auto& log : logs)
    {
        mItemModelLogs->appendRow(log);
    }
    horizontalHeader()->setSectionResizeMode(Column::MessageCol, QHeaderView::Stretch);
    horizontalHeader()->resizeSection(Column::TimestampCol, 200);
    horizontalHeader()->resizeSection(Column::FeedCol, 40);
    horizontalHeader()->resizeSection(Column::LogLevelCol, 40);

    mCurrentLogCount = totalLogCount;
    mCurrentLogPage = pageNumber;
    mCurrentLogPageCount = 1;
    if (mCurrentLogCount > 0)
    {
        mCurrentLogPageCount = static_cast<uint64_t>(std::ceil(static_cast<float>(mCurrentLogCount) / static_cast<float>(msLogsPerPage)));
    }

    mMainWindow->getUI()->pushButtonLogPageNumber->setText(QString("%1 %2 / %3").arg(tr("Page")).arg(mCurrentLogPage).arg(mCurrentLogPageCount));
}

void ZapFR::Client::TableViewLogs::connectStuff()
{
    connect(mActionViewLogs.get(), &QAction::triggered,
            [&]()
            {
                mCurrentLogPage = 1;
                reload();
            });

    connect(mActionClearLogs.get(), &QAction::triggered,
            [&]()
            {
                if (QMessageBox::question(this, tr("Clear logs"), tr("Are you sure you wish to clear these logs?")) == QMessageBox::No)
                {
                    return;
                }

                auto index = mMainWindow->getUI()->treeViewSources->currentIndex();
                if (index.isValid())
                {
                    auto type = index.data(SourceTreeEntryTypeRole).toULongLong();
                    auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
                    switch (type)
                    {
                        case SOURCETREE_ENTRY_TYPE_SOURCE:
                        {
                            ZapFR::Engine::Agent::getInstance()->queueClearSourceLogs(sourceID, [&]() { reload(); });
                            break;
                        }
                        case SOURCETREE_ENTRY_TYPE_FOLDER:
                        {
                            auto folderID = index.data(SourceTreeEntryIDRole).toULongLong();
                            ZapFR::Engine::Agent::getInstance()->queueClearFolderLogs(sourceID, folderID, [&]() { reload(); });
                            break;
                        }
                        case SOURCETREE_ENTRY_TYPE_FEED:
                        {
                            auto feedID = index.data(SourceTreeEntryIDRole).toULongLong();
                            ZapFR::Engine::Agent::getInstance()->queueClearFeedLogs(sourceID, feedID, [&]() { reload(); });
                            break;
                        }
                    }
                }
            });

    connect(mMainWindow->getUI()->pushButtonLogPreviousPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentLogPage = std::max(1ul, mCurrentLogPage - 1);
                reload();
            });

    connect(mMainWindow->getUI()->pushButtonLogNextPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentLogPage = std::min(mCurrentLogPageCount, mCurrentLogPage + 1);
                reload();
            });

    connect(mMainWindow->getUI()->pushButtonLogFirstPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentLogPage = 1;
                reload();
            });

    connect(mMainWindow->getUI()->pushButtonLogLastPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentLogPage = mCurrentLogPageCount;
                reload();
            });

    connect(mMainWindow->getUI()->pushButtonLogPageNumber, &QPushButton::clicked,
            [&]()
            {
                mMainWindow->showJumpToPageDialog(mCurrentLogPage, mCurrentLogPageCount,
                                                  [&](uint64_t page)
                                                  {
                                                      mCurrentLogPage = page;
                                                      reload();
                                                  });
            });
}
