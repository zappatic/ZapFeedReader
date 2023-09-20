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
#include "FeedIconCache.h"
#include "ZapFR/Agent.h"
#include "ZapFR/base/Feed.h"
#include "dialogs/DialogAddFeed.h"
#include "models/SortFilterProxyModelSources.h"
#include "models/StandardItemModelSources.h"
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
                        auto sourceID = mDialogAddFeed->selectedSourceID();
                        auto url = mDialogAddFeed->url().toStdString();
                        auto folderID = mDialogAddFeed->selectedFolderID();
                        if (!url.empty())
                        {
                            ZapFR::Engine::Agent::getInstance()->queueAddFeed(sourceID, url, folderID,
                                                                              [&](uint64_t affectedSourceID, uint64_t newFeedID)
                                                                              { QMetaObject::invokeMethod(this, [=, this]() { feedAdded(affectedSourceID, newFeedID); }); });
                        }
                    }
                });
    }

    auto [sourceID, folderID] = getCurrentlySelectedSourceAndFolderID();
    mDialogAddFeed->reset(sourceID, folderID);
    mDialogAddFeed->open();
}

void ZapFR::Client::MainWindow::removeFeed()
{
    QMessageBox messageBox(this);
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
            ZapFR::Engine::Agent::getInstance()->queueRemoveFeed(sourceID, feedID, [&]() { QMetaObject::invokeMethod(this, [=, this]() { feedRemoved(); }); });
        }
    }
}

void ZapFR::Client::MainWindow::refreshViaToolbarButton()
{
    auto index = ui->treeViewSources->currentIndex();
    if (index.isValid())
    {
        auto type = index.data(SourceTreeEntryTypeRole).toULongLong();
        if (mPreferenceRefreshBehaviour == RefreshBehaviour::EntireSource)
        {
            type = SOURCETREE_ENTRY_TYPE_SOURCE;
        }
        refreshSourceEntryType(index, type);
    }
}

void ZapFR::Client::MainWindow::refreshViaContextMenu()
{
    auto index = ui->treeViewSources->currentIndex();
    if (index.isValid())
    {
        auto type = index.data(SourceTreeEntryTypeRole).toULongLong();
        refreshSourceEntryType(index, type);
    }
}

void ZapFR::Client::MainWindow::refreshSourceEntryType(const QModelIndex& index, uint64_t type)
{
    auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
    ui->statusbar->showMessage(tr("Refreshing '%1'").arg(index.data(Qt::DisplayRole).toString()), StatusBarDefaultTimeout);
    switch (type)
    {
        case SOURCETREE_ENTRY_TYPE_FEED:
        {
            auto feedID = index.data(SourceTreeEntryIDRole).toULongLong();
            ZapFR::Engine::Agent::getInstance()->queueRefreshFeed(
                sourceID, feedID,
                [&](uint64_t affectedSourceID, ZapFR::Engine::Feed* refreshedFeed)
                {
                    auto id = refreshedFeed->id();
                    auto unreadCount = refreshedFeed->unreadCount();
                    auto error = refreshedFeed->lastRefreshError();
                    auto title = refreshedFeed->title();
                    auto iconHash = refreshedFeed->iconHash();
                    auto iconData = refreshedFeed->iconData();
                    QMetaObject::invokeMethod(this, [=, this]()
                                              { feedRefreshed(affectedSourceID, id, unreadCount, error.has_value() ? error.value() : "", title, iconHash, iconData); });
                });
            break;
        }
        case SOURCETREE_ENTRY_TYPE_FOLDER:
        {
            auto folderID = index.data(SourceTreeEntryIDRole).toULongLong();
            ZapFR::Engine::Agent::getInstance()->queueRefreshFolder(
                sourceID, folderID,
                [&](uint64_t affectedSourceID, ZapFR::Engine::Feed* refreshedFeed)
                {
                    auto id = refreshedFeed->id();
                    auto unreadCount = refreshedFeed->unreadCount();
                    auto error = refreshedFeed->lastRefreshError();
                    auto title = refreshedFeed->title();
                    auto iconHash = refreshedFeed->iconHash();
                    auto iconData = refreshedFeed->iconData();
                    QMetaObject::invokeMethod(this, [=, this]()
                                              { feedRefreshed(affectedSourceID, id, unreadCount, error.has_value() ? error.value() : "", title, iconHash, iconData); });
                });
            break;
        }
        case SOURCETREE_ENTRY_TYPE_SOURCE:
        {
            ZapFR::Engine::Agent::getInstance()->queueRefreshSource(
                sourceID,
                [&](uint64_t affectedSourceID, ZapFR::Engine::Feed* refreshedFeed)
                {
                    auto id = refreshedFeed->id();
                    auto unreadCount = refreshedFeed->unreadCount();
                    auto error = refreshedFeed->lastRefreshError();
                    auto title = refreshedFeed->title();
                    auto iconHash = refreshedFeed->iconHash();
                    auto iconData = refreshedFeed->iconData();
                    QMetaObject::invokeMethod(this, [=, this]()
                                              { feedRefreshed(affectedSourceID, id, unreadCount, error.has_value() ? error.value() : "", title, iconHash, iconData); });
                });

            break;
        }
    }
}

void ZapFR::Client::MainWindow::feedAdded(uint64_t sourceID, uint64_t feedID)
{
    reloadSources();
    ZapFR::Engine::Agent::getInstance()->queueRefreshFeed(
        sourceID, feedID,
        [&](uint64_t affectedSourceID, ZapFR::Engine::Feed* refreshedFeed)
        {
            auto id = refreshedFeed->id();
            auto unreadCount = refreshedFeed->unreadCount();
            auto error = refreshedFeed->lastRefreshError();
            auto title = refreshedFeed->title();
            auto iconHash = refreshedFeed->iconHash();
            auto iconData = refreshedFeed->iconData();
            QMetaObject::invokeMethod(this,
                                      [=, this]() { feedRefreshed(affectedSourceID, id, unreadCount, error.has_value() ? error.value() : "", title, iconHash, iconData); });
        });
}

void ZapFR::Client::MainWindow::feedRemoved()
{
    reloadSources();
    populatePosts();
    ui->statusbar->showMessage(tr("Feed removed"), StatusBarDefaultTimeout);
}

void ZapFR::Client::MainWindow::feedMarkedRead(uint64_t sourceID, uint64_t feedID)
{
    updateFeedUnreadCountBadge(sourceID, {feedID}, false, 0);
    mCurrentPostPage = 1;
    reloadPosts();
    ui->tableViewScriptFolders->reload(true);
    ui->statusbar->showMessage(tr("Feed marked as read"), StatusBarDefaultTimeout);
}

void ZapFR::Client::MainWindow::feedRefreshed(uint64_t sourceID, uint64_t feedID, uint64_t feedUnreadCount, const std::string& error, const std::string& feedTitle,
                                              const std::string& iconHash, const std::string& iconData)
{
    ui->statusbar->showMessage(tr("Feed '%1' refreshed").arg(QString::fromUtf8(feedTitle)), StatusBarDefaultTimeout);
    auto sourceItem = findSourceStandardItem(sourceID);
    if (sourceItem != nullptr)
    {
        auto feedItems = findFeedStandardItems(sourceItem, {{feedID}});
        for (const auto& feedItem : feedItems)
        {
            if (!iconData.empty() && (!FeedIconCache::isCached(sourceID, feedID) || !FeedIconCache::isSameHash(sourceID, feedID, iconHash)))
            {
                QPixmap icon;
                icon.loadFromData(QByteArray(iconData.c_str(), static_cast<int64_t>(iconData.length())));
                if (!icon.isNull())
                {
                    FeedIconCache::cache(sourceID, feedID, iconHash, icon);
                }
            }

            feedItem->setData(QString::fromUtf8(feedTitle), Qt::DisplayRole);
            feedItem->setData(QVariant::fromValue<uint64_t>(feedUnreadCount), SourceTreeEntryUnreadCount);
            if (!error.empty())
            {
                feedItem->setData(QString::fromUtf8(error), SourceTreeEntryErrorRole);
                feedItem->setData(QString::fromUtf8(error), Qt::ToolTipRole);
            }
            else
            {
                feedItem->setData(QVariant(), SourceTreeEntryErrorRole);
                feedItem->setData("", Qt::ToolTipRole);
            }

            // if the feed is currently selected, then refresh the posts so the new unread posts are shown
            // only do that when we're in the posts pane
            if (ui->stackedWidgetContentPanes->currentIndex() == StackedPanePosts)
            {
                auto currentIndex = ui->treeViewSources->currentIndex();
                if (currentIndex.isValid() && mItemModelSources->itemFromIndex(mProxyModelSources->mapToSource(currentIndex)) == feedItem)
                {
                    mCurrentPostPage = 1;
                    reloadPosts();
                }
            }
        }
    }
    ui->tableViewScriptFolders->reload(true);
    reloadUsedFlagColors(true);
}

void ZapFR::Client::MainWindow::updateFeedUnreadCountBadge(uint64_t sourceID, std::unordered_set<uint64_t> feedIDs, bool markEntireSourceAsRead, uint64_t unreadCount)
{
    auto sourceItem = findSourceStandardItem(sourceID);
    if (sourceItem == nullptr)
    {
        return;
    }

    std::unordered_set<QStandardItem*> feedItems;
    if (markEntireSourceAsRead)
    {
        feedItems = findFeedStandardItems(sourceItem, {});
    }
    else
    {
        feedItems = findFeedStandardItems(sourceItem, feedIDs);
    }

    for (const auto& feedItem : feedItems)
    {
        feedItem->setData(QVariant::fromValue<uint64_t>(unreadCount), SourceTreeEntryUnreadCount);
    }
}

std::unordered_set<QStandardItem*> ZapFR::Client::MainWindow::findFeedStandardItems(QStandardItem* sourceItem, const std::optional<std::unordered_set<uint64_t>>& feedIDs)
{
    std::unordered_set<QStandardItem*> feedItems;

    std::function<void(QStandardItem*)> findFeeds;
    findFeeds = [&](QStandardItem* parent)
    {
        auto type = parent->data(SourceTreeEntryTypeRole).toULongLong();
        switch (type)
        {
            case SOURCETREE_ENTRY_TYPE_SOURCE:
            case SOURCETREE_ENTRY_TYPE_FOLDER:
            {
                for (int32_t i = 0; i < parent->rowCount(); ++i)
                {
                    findFeeds(parent->child(i, 0));
                }
                break;
            }
            case SOURCETREE_ENTRY_TYPE_FEED:
            {
                auto id = parent->data(SourceTreeEntryIDRole).toULongLong();
                if (feedIDs.has_value())
                {
                    if (feedIDs.value().contains(id))
                    {
                        feedItems.insert(parent);
                    }
                }
                else // empty feedIDs means add all feed items
                {
                    feedItems.insert(parent);
                }
                break;
            }
        }
    };

    findFeeds(sourceItem);

    return feedItems;
}

void ZapFR::Client::MainWindow::connectFeedStuff()
{
    connect(ui->action_Add_feed, &QAction::triggered, this, &MainWindow::addFeed);
    connect(ui->action_Refresh, &QAction::triggered, this, &MainWindow::refreshViaContextMenu);
    connect(ui->action_Toolbar_refresh, &QAction::triggered, this, &MainWindow::refreshViaToolbarButton);
    connect(ui->action_Remove_feed, &QAction::triggered, this, &MainWindow::removeFeed);
}

void ZapFR::Client::MainWindow::createFeedContextMenus()
{
    mSourceContextMenuFeed = std::make_unique<QMenu>(nullptr);
    mSourceContextMenuFeed->addAction(ui->action_Refresh);
    mSourceContextMenuFeed->addAction(ui->action_Mark_as_read);
    mSourceContextMenuFeed->addSeparator();
    mSourceContextMenuFeed->addAction(ui->action_Remove_feed);
    mSourceContextMenuFeed->addSeparator();
    mSourceContextMenuFeed->addAction(ui->tableViewLogs->actionViewLogs());
    mSourceContextMenuFeed->addAction(ui->action_View_properties);
}
