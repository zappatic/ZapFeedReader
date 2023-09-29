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

#include <QClipboard>
#include <QMessageBox>

#include "./ui_MainWindow.h"
#include "Utilities.h"
#include "ZapFR/Agent.h"
#include "ZapFR/Log.h"
#include "delegates/ItemDelegateLog.h"
#include "widgets/MainWindow.h"
#include "widgets/TableViewLogs.h"
#include "widgets/TreeViewSources.h"

ZapFR::Client::TableViewLogs::TableViewLogs(QWidget* parent) : TableViewPaletteCorrected(parent)
{
    setItemDelegate(new ItemDelegateLog(this));
    mItemModelLogs = std::make_unique<QStandardItemModel>(this);
    setModel(mItemModelLogs.get());

    mActionViewLogs = std::make_unique<QAction>(tr("View logs"), this);
    mActionClearLogs = std::make_unique<QAction>(tr("Clear logs"), this);
    mActionCopyMessages = std::make_unique<QAction>(tr("Copy"), this);
}

void ZapFR::Client::TableViewLogs::setMainWindow(MainWindow* mw) noexcept
{
    mMainWindow = mw;
    connectStuff();
    createContextMenu();
}

void ZapFR::Client::TableViewLogs::reload()
{
    static auto whitespaceRe = QRegularExpression(R"(\s+)");

    // lambda for the callback, retrieving the logs
    auto processLogs = [&](uint64_t sourceID, const std::vector<ZapFR::Engine::Log*> logs, uint64_t page, uint64_t totalRecordCount)
    {
        const auto setItemData = [&](QStandardItem* item, ZapFR::Engine::Log* log)
        {
            auto message = QString::fromUtf8(log->message());
            message.replace(whitespaceRe, " ");

            item->setData(message, Role::Message);
            item->setData(QVariant::fromValue<uint64_t>(log->id()), Role::ID);
            item->setData(QVariant::fromValue<uint64_t>(log->level()), Role::Level);
            item->setData(QString::fromUtf8(log->timestamp()), Role::Timestamp);
            if (log->feedID().has_value())
            {
                item->setData(QVariant::fromValue<uint64_t>(log->feedID().value()), Role::FeedID);
                item->setData(QVariant::fromValue<uint64_t>(sourceID), Role::ParentSourceID);
            }
        };

        QList<QList<QStandardItem*>> rows;
        for (const auto& log : logs)
        {
            auto logLevelItem = new QStandardItem("");
            setItemData(logLevelItem, log);
            auto logLevel = log->level();
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
            setItemData(dateItem, log);

            auto feedItem = new QStandardItem("");
            setItemData(feedItem, log);
            if (log->feedTitle().has_value())
            {
                feedItem->setData(QString::fromUtf8(log->feedTitle().value()), Qt::ToolTipRole);
            }

            auto message = QString::fromUtf8(log->message());
            message.replace(whitespaceRe, " ");
            auto titleItem = new QStandardItem(message);
            setItemData(titleItem, log);

            QList<QStandardItem*> rowData;
            rowData << logLevelItem << feedItem << dateItem << titleItem;
            rows << rowData;
        }

        QMetaObject::invokeMethod(this, [=, this]() { populateLogs(rows, page, totalRecordCount); });
    };

    auto index = mMainWindow->treeViewSources()->currentIndex();
    if (index.isValid())
    {
        if (index.data(TreeViewSources::Role::Type) == TreeViewSources::EntryType::Feed)
        {
            auto sourceID = index.data(TreeViewSources::Role::ParentSourceID).toULongLong();
            auto feedID = index.data(TreeViewSources::Role::ID).toULongLong();
            ZapFR::Engine::Agent::getInstance()->queueGetFeedLogs(sourceID, feedID, msLogsPerPage, mCurrentLogPage, processLogs);
        }
        else if (index.data(TreeViewSources::Role::Type) == TreeViewSources::EntryType::Folder)
        {
            auto sourceID = index.data(TreeViewSources::Role::ParentSourceID).toULongLong();
            auto folderID = index.data(TreeViewSources::Role::ID).toULongLong();
            ZapFR::Engine::Agent::getInstance()->queueGetFolderLogs(sourceID, folderID, msLogsPerPage, mCurrentLogPage, processLogs);
        }
        else if (index.data(TreeViewSources::Role::Type) == TreeViewSources::EntryType::Source)
        {
            auto sourceID = index.data(TreeViewSources::Role::ParentSourceID).toULongLong();
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
    mMainWindow->setContentPane(ContentPane::Logs);

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
    connect(mActionCopyMessages.get(), &QAction::triggered, this, &TableViewLogs::copyMessages);

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

                auto index = mMainWindow->treeViewSources()->currentIndex();
                if (index.isValid())
                {
                    auto type = index.data(TreeViewSources::Role::Type).toULongLong();
                    auto sourceID = index.data(TreeViewSources::Role::ParentSourceID).toULongLong();
                    switch (type)
                    {
                        case TreeViewSources::EntryType::Source:
                        {
                            ZapFR::Engine::Agent::getInstance()->queueClearSourceLogs(sourceID, [&]() { reload(); });
                            break;
                        }
                        case TreeViewSources::EntryType::Folder:
                        {
                            auto folderID = index.data(TreeViewSources::Role::ID).toULongLong();
                            ZapFR::Engine::Agent::getInstance()->queueClearFolderLogs(sourceID, folderID, [&]() { reload(); });
                            break;
                        }
                        case TreeViewSources::EntryType::Feed:
                        {
                            auto feedID = index.data(TreeViewSources::Role::ID).toULongLong();
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

    connect(this, &TableViewLogs::customContextMenuRequested, [&](const QPoint& p) { mLogsContextMenu->popup(viewport()->mapToGlobal(p)); });
}

void ZapFR::Client::TableViewLogs::createContextMenu()
{
    mLogsContextMenu = std::make_unique<QMenu>(nullptr);
    mLogsContextMenu->addAction(mActionCopyMessages.get());
}

void ZapFR::Client::TableViewLogs::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_C && ((event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier))
    {
        copyMessages();
        return;
    }
    QTableView::keyPressEvent(event);
}

void ZapFR::Client::TableViewLogs::copyMessages()
{
    QString stringToCopy;
    QTextStream ss(&stringToCopy);

    auto selection = selectionModel()->selectedIndexes();
    for (const auto& selectedRow : selection)
    {
        if (selectedRow.column() == Column::MessageCol)
        {
            ss << "At: " << selectedRow.data(Role::Timestamp).toString() << "\n";
            ss << "Level: " << selectedRow.data(Role::Level).toULongLong() << "\n";
            ss << "Message: " << selectedRow.data(Role::Message).toString() << "\n\n";
        }
    }

    QApplication::clipboard()->setText(stringToCopy);
    mMainWindow->setStatusBarMessage(tr("Log message(s) copied to clipboard"));
}
