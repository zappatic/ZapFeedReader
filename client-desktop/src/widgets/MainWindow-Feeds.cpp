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
#include "ZapFR/Agent.h"
#include "dialogs/DialogAddFeed.h"
#include "widgets/MainWindow.h"

void ZapFR::Client::MainWindow::addFeed()
{
    if (mDialogAddFeed == nullptr)
    {
        mDialogAddFeed = std::make_unique<DialogAddFeed>(this);
        connect(mDialogAddFeed.get(), &QDialog::finished,
                [&](int result)
                {
                    if (result == QDialog::DialogCode::Accepted)
                    {
                        ZapFR::Engine::Agent::getInstance()->queueSubscribeFeed(mDialogAddFeed->selectedSourceID(), mDialogAddFeed->url().toStdString(),
                                                                                mDialogAddFeed->selectedFolderID(), {},
                                                                                [&]() { QMetaObject::invokeMethod(this, "feedAdded", Qt::AutoConnection); });
                    }
                });
    }

    auto [sourceID, folderID] = getCurrentlySelectedSourceAndFolderID();
    mDialogAddFeed->reset(sourceID, folderID);
    mDialogAddFeed->open();
}

void ZapFR::Client::MainWindow::removeFeed()
{
    QMessageBox messageBox;
    messageBox.setText(tr("Remove feed"));
    messageBox.setWindowTitle(tr("Remove feed"));
    messageBox.setInformativeText(tr("Are you sure you want to remove this feed? All associated posts will be removed!"));
    messageBox.setIcon(QMessageBox::Warning);
    auto yesButton = messageBox.addButton(QMessageBox::StandardButton::Yes);
    yesButton->setText(tr("Remove"));
    messageBox.addButton(QMessageBox::StandardButton::Cancel);
    auto messageBoxLayout = qobject_cast<QGridLayout*>(messageBox.layout());
    messageBoxLayout->addItem(new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), messageBoxLayout->rowCount(), 0, 1, messageBoxLayout->columnCount());
    messageBox.exec();
    if (messageBox.buttonRole(messageBox.clickedButton()) == QMessageBox::YesRole)
    {
        auto index = ui->treeViewSources->currentIndex();
        if (index.isValid())
        {
            auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
            auto feedID = index.data(SourceTreeEntryIDRole).toULongLong();
            ZapFR::Engine::Agent::getInstance()->queueRemoveFeed(sourceID, feedID, [&]() { QMetaObject::invokeMethod(this, "feedRemoved", Qt::AutoConnection); });
        }
    }
}

void ZapFR::Client::MainWindow::refreshFeeds()
{
    auto index = ui->treeViewSources->currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
        auto type = index.data(SourceTreeEntryTypeRole).toULongLong();
        switch (type)
        {
            case SOURCETREE_ENTRY_TYPE_FEED:
            {
                auto feedID = index.data(SourceTreeEntryIDRole).toULongLong();
                ZapFR::Engine::Agent::getInstance()->queueRefreshFeed(
                    sourceID, feedID,
                    [&](uint64_t affectedSourceID, uint64_t affectedFeedID)
                    { QMetaObject::invokeMethod(this, "feedRefreshed", Qt::AutoConnection, affectedSourceID, affectedFeedID); });
                break;
            }
            case SOURCETREE_ENTRY_TYPE_FOLDER:
            {
                auto folderID = index.data(SourceTreeEntryIDRole).toULongLong();
                ZapFR::Engine::Agent::getInstance()->queueRefreshFolder(
                    sourceID, folderID,
                    [&](uint64_t affectedSourceID, uint64_t affectedFeedID)
                    { QMetaObject::invokeMethod(this, "feedRefreshed", Qt::AutoConnection, affectedSourceID, affectedFeedID); });
                break;
            }
            case SOURCETREE_ENTRY_TYPE_SOURCE:
            {
                ZapFR::Engine::Agent::getInstance()->queueRefreshSource(
                    sourceID, [&](uint64_t affectedSourceID, uint64_t affectedFeedID)
                    { QMetaObject::invokeMethod(this, "feedRefreshed", Qt::AutoConnection, affectedSourceID, affectedFeedID); });

                break;
            }
        }
    }
}

void ZapFR::Client::MainWindow::feedAdded()
{
    reloadSources();
}

void ZapFR::Client::MainWindow::feedRemoved()
{
    reloadSources();
    populatePosts();
}

void ZapFR::Client::MainWindow::feedMoved()
{
    reloadSources();
}

void ZapFR::Client::MainWindow::feedMarkedRead(uint64_t sourceID, uint64_t feedID)
{
    updateFeedUnreadCountBadge(sourceID, {feedID}, false, 0);
    mCurrentPostPage = 1;
    reloadPosts();
}

void ZapFR::Client::MainWindow::feedRefreshed(uint64_t sourceID, uint64_t feedID)
{
    ZapFR::Engine::Agent::getInstance()->queueGetFeedUnreadCount(sourceID, feedID,
                                                                 [&](uint64_t affectedSourceID, uint64_t affectedFeedID, uint64_t unreadCount)
                                                                 {
                                                                     std::unordered_set<uint64_t> feedIDs;
                                                                     feedIDs.insert(affectedFeedID);
                                                                     QMetaObject::invokeMethod(this, "updateFeedUnreadCountBadge", Qt::AutoConnection, affectedSourceID,
                                                                                               feedIDs, false, unreadCount);
                                                                 });

    // if the feed is currently selected, then refresh the posts so the new unread posts are shown
    auto index = ui->treeViewSources->currentIndex();
    if (index.isValid())
    {
        if (index.data(SourceTreeEntryTypeRole).toULongLong() == SOURCETREE_ENTRY_TYPE_FEED)
        {
            auto selectedFeedID = index.data(SourceTreeEntryIDRole).toULongLong();
            if (selectedFeedID == feedID)
            {
                mCurrentPostPage = 1;
                reloadPosts();
            }
        }
    }

    reloadUsedFlagColors(true);
}

void ZapFR::Client::MainWindow::connectFeedStuff()
{
    connect(ui->action_Add_feed, &QAction::triggered, this, &MainWindow::addFeed);
    connect(ui->action_Refresh_feeds, &QAction::triggered, this, &MainWindow::refreshFeeds);
    connect(ui->action_Remove_feed, &QAction::triggered, this, &MainWindow::removeFeed);
}

void ZapFR::Client::MainWindow::createFeedContextMenus()
{
    mSourceContextMenuFeed = std::make_unique<QMenu>(nullptr);
    mSourceContextMenuFeed->addAction(ui->action_Refresh_feeds);
    mSourceContextMenuFeed->addAction(ui->action_Mark_as_read);
    mSourceContextMenuFeed->addAction(ui->action_View_logs);
    mSourceContextMenuFeed->addSeparator();
    mSourceContextMenuFeed->addAction(ui->action_Remove_feed);
}
