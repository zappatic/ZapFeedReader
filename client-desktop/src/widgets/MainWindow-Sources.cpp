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
#include "ZapFR/base/Folder.h"
#include "ZapFR/base/Source.h"
#include "delegates/ItemDelegateSource.h"
#include "dialogs/DialogAddSource.h"
#include "models/SortFilterProxyModelSources.h"
#include "models/StandardItemModelSources.h"
#include "widgets/MainWindow.h"

void ZapFR::Client::MainWindow::reloadSources()
{
    preserveSourceTreeExpansionSelectionState();

    mItemModelSources->clear();
    mItemModelSources->setHorizontalHeaderItem(0, new QStandardItem(tr("Sources & Feeds")));

    // get the trees of all the sources
    auto sources = ZapFR::Engine::Source::getSources({});
    mInitialSourceCount = sources.size();
    for (const auto& source : sources)
    {
        ZapFR::Engine::Agent::getInstance()->queueGetSourceTree(
            source->id(),
            [&](ZapFR::Engine::Source* retrievedSource, const std::vector<ZapFR::Engine::Folder*>& rootFolders, const std::vector<ZapFR::Engine::Feed*>& feeds)
            {
                // lambda to recursively create folder items
                std::unordered_map<uint64_t, QStandardItem*> folderIDToItemMapping; // a map to quickly look up folder items when adding feed items
                std::function<void(ZapFR::Engine::Folder*, uint64_t, QStandardItem*)> createFolderItems;
                createFolderItems = [&](ZapFR::Engine::Folder* folder, uint64_t currentSourceID, QStandardItem* parentItem)
                {
                    auto folderItem = new QStandardItem(QString::fromUtf8(folder->title()));
                    folderItem->setData(SOURCETREE_ENTRY_TYPE_FOLDER, SourceTreeEntryTypeRole);
                    folderItem->setData(QVariant::fromValue<uint64_t>(folder->id()), SourceTreeEntryIDRole);
                    folderItem->setData(QVariant::fromValue<uint64_t>(folder->parentID()), SourceTreeEntryParentFolderIDRole);
                    folderItem->setData(QVariant::fromValue<uint64_t>(currentSourceID), SourceTreeEntryParentSourceIDRole);
                    folderItem->setData(QVariant::fromValue<uint64_t>(folder->sortOrder()), SourceTreeEntrySortOrderRole);
                    parentItem->appendRow(folderItem);

                    for (const auto& subfolder : folder->subfolders())
                    {
                        createFolderItems(subfolder.get(), currentSourceID, folderItem);
                    }
                    folderIDToItemMapping[folder->id()] = folderItem;
                };
                // create the parent source item
                auto sourceItem = new QStandardItem(QString::fromUtf8(retrievedSource->title()));
                sourceItem->setData(SOURCETREE_ENTRY_TYPE_SOURCE, SourceTreeEntryTypeRole);
                sourceItem->setData(QVariant::fromValue<uint64_t>(retrievedSource->id()), SourceTreeEntryIDRole);
                sourceItem->setData(QVariant::fromValue<uint64_t>(retrievedSource->id()), SourceTreeEntryParentSourceIDRole);
                sourceItem->setData(QVariant::fromValue<uint64_t>(retrievedSource->sortOrder()), SourceTreeEntrySortOrderRole);
                sourceItem->setData(QString::fromUtf8(retrievedSource->type()), SourceTreeEntrySourceTypeRole);

                // add all the folders and subfolders
                for (const auto& folder : rootFolders)
                {
                    createFolderItems(folder, retrievedSource->id(), sourceItem);
                }

                // add all the feeds
                for (const auto& feed : feeds)
                {
                    // look up the folder to which this feed belongs, default to source item
                    auto parentItem = sourceItem;
                    auto folderID = feed->folder();
                    if (folderIDToItemMapping.contains(folderID))
                    {
                        parentItem = folderIDToItemMapping.at(folderID);
                    }

                    // create the feed item
                    auto feedItem = new QStandardItem(QString::fromUtf8(feed->title()));
                    feedItem->setData(SOURCETREE_ENTRY_TYPE_FEED, SourceTreeEntryTypeRole);
                    feedItem->setData(QVariant::fromValue<uint64_t>(feed->id()), SourceTreeEntryIDRole);
                    feedItem->setData(QVariant::fromValue<uint64_t>(retrievedSource->id()), SourceTreeEntryParentSourceIDRole);
                    feedItem->setData(QVariant::fromValue<uint64_t>(folderID), SourceTreeEntryParentFolderIDRole);
                    auto unreadCount = feed->unreadCount();
                    feedItem->setData(QVariant::fromValue<uint64_t>(unreadCount), SourceTreeEntryUnreadCount);
                    if (unreadCount >= 999)
                    {
                        feedItem->setToolTip(tr("%1 unread").arg(unreadCount));
                    }
                    feedItem->setData(true, SourceTreeEntryDisplayUnreadCountBadge);
                    auto feedError = feed->lastRefreshError();
                    if (feedError.has_value())
                    {
                        feedItem->setData(QString::fromUtf8(feedError.value()), SourceTreeEntryFeedErrorRole);
                        feedItem->setData(QString::fromUtf8(feedError.value()), Qt::ToolTipRole);
                    }
                    feedItem->setData(QString::fromUtf8(feed->url()), SourceTreeEntryFeedURLRole);
                    feedItem->setData(QVariant::fromValue<uint64_t>(feed->sortOrder()), SourceTreeEntrySortOrderRole);

                    if (!FeedIconCache::isCached(retrievedSource->id(), feed->id()) || !FeedIconCache::isSameHash(retrievedSource->id(), feed->id(), feed->iconHash()))
                    {
                        auto iconData = feed->iconData();
                        if (!iconData.empty())
                        {
                            QPixmap icon;
                            icon.loadFromData(QByteArray(iconData.c_str(), static_cast<int64_t>(iconData.length())));
                            if (!icon.isNull())
                            {
                                Poco::MD5Engine md5;
                                Poco::DigestOutputStream ds(md5);
                                ds << iconData;
                                ds.close();
                                auto iconHash = Poco::DigestEngine::digestToHex(md5.digest());

                                FeedIconCache::cache(retrievedSource->id(), feed->id(), iconHash, icon);
                            }
                        }
                    }
                    parentItem->appendRow(feedItem);
                }
                QMetaObject::invokeMethod(this, "populateSources", Qt::AutoConnection, retrievedSource->id(), sourceItem);
            });
    }
}

void ZapFR::Client::MainWindow::populateSources(uint64_t /*sourceID*/, QStandardItem* sourceItem)
{
    mItemModelSources->appendRow(sourceItem);
    restoreSourceTreeExpansionSelectionState(sourceItem);
}

std::tuple<uint64_t, uint64_t> ZapFR::Client::MainWindow::getCurrentlySelectedSourceAndFolderID() const
{
    uint64_t sourceID{0};
    uint64_t folderID{0};
    auto currentIndex = ui->treeViewSources->currentIndex();
    if (currentIndex.isValid())
    {
        sourceID = currentIndex.data(SourceTreeEntryParentSourceIDRole).toULongLong();
        auto type = currentIndex.data(SourceTreeEntryTypeRole);
        if (type == SOURCETREE_ENTRY_TYPE_FOLDER)
        {
            folderID = currentIndex.data(SourceTreeEntryIDRole).toULongLong();
        }
        else if (type == SOURCETREE_ENTRY_TYPE_FEED)
        {
            folderID = currentIndex.data(SourceTreeEntryParentFolderIDRole).toULongLong();
        }
    }
    return std::make_tuple(sourceID, folderID);
}

void ZapFR::Client::MainWindow::preserveSourceTreeExpansionSelectionState()
{
    if (mProxyModelSources != nullptr && mProxyModelSources->displayMode() == SortFilterProxyModelSources::SourceTreeDisplayMode::ShowAll)
    {
        mReloadSourcesExpansionSelectionState = std::make_unique<QJsonObject>();
        mReloadSourcesExpansionSelectionState->insert("expanded", expandedSourceTreeItems());
        uint64_t selectedSourceID = 0;
        uint64_t selectedID = 0;
        uint32_t selectedType = 0;
        auto index = ui->treeViewSources->currentIndex();
        if (index.isValid())
        {
            selectedSourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
            selectedID = index.data(SourceTreeEntryIDRole).toULongLong();
            selectedType = index.data(SourceTreeEntryTypeRole).toUInt();
        }
        mReloadSourcesExpansionSelectionState->insert("selectedSourceID", static_cast<qint64>(selectedSourceID));
        mReloadSourcesExpansionSelectionState->insert("selectedID", static_cast<qint64>(selectedID));
        mReloadSourcesExpansionSelectionState->insert("selectedType", static_cast<int32_t>(selectedType));
    }
}

void ZapFR::Client::MainWindow::restoreSourceTreeExpansionSelectionState(QStandardItem* /*sourceItem*/)
{
    if (mReloadSourcesExpansionSelectionState != nullptr && mProxyModelSources != nullptr &&
        mProxyModelSources->displayMode() == SortFilterProxyModelSources::SourceTreeDisplayMode::ShowAll)
    {
        auto expandedItems = mReloadSourcesExpansionSelectionState->value("expanded").toArray();
        auto selectedSourceID = mReloadSourcesExpansionSelectionState->value("selectedSourceID").toInteger();
        auto selectedID = mReloadSourcesExpansionSelectionState->value("selectedID").toInteger();
        auto selectedType = mReloadSourcesExpansionSelectionState->value("selectedType").toInteger();

        expandSourceTreeItems(expandedItems);
        if (selectedSourceID != 0 && selectedID != 0)
        {
            std::function<void(QStandardItem*)> selectIndex;
            selectIndex = [&](QStandardItem* parent)
            {
                if (parent->data(SourceTreeEntryTypeRole).toUInt() == selectedType && parent->data(SourceTreeEntryParentSourceIDRole).toLongLong() == selectedSourceID &&
                    parent->data(SourceTreeEntryIDRole).toLongLong() == selectedID)
                {
                    auto indexToSelect = mProxyModelSources->mapFromSource(mItemModelSources->indexFromItem(parent));
                    ui->treeViewSources->setCurrentIndex(indexToSelect);
                    return;
                }
                else
                {
                    if (parent->hasChildren())
                    {
                        for (int i = 0; i < parent->rowCount(); ++i)
                        {
                            selectIndex(parent->child(i));
                        }
                    }
                }
            };
            selectIndex(mItemModelSources->invisibleRootItem());
        }

        // if nothing is selected in the source tree (as it is on startup):
        // select the local source if it has children or there is no remote source
        // else select the first remote source
        // only do this when ALL the sources have been loaded (as this function is called from each source retrieval callback)
        if (!ui->treeViewSources->currentIndex().isValid() && mItemModelSources->invisibleRootItem()->rowCount() == static_cast<int32_t>(mInitialSourceCount))
        {
            QStandardItem* localSource{nullptr};
            QStandardItem* firstRemoteSource{nullptr};
            for (auto i = 0; i < mItemModelSources->invisibleRootItem()->rowCount(); ++i)
            {
                auto child = mItemModelSources->invisibleRootItem()->child(i);
                if (child->data(SourceTreeEntryTypeRole).toULongLong() == SOURCETREE_ENTRY_TYPE_SOURCE)
                {
                    if (child->data(SourceTreeEntrySourceTypeRole).toString().toStdString() == ZapFR::Engine::IdentifierLocalServer)
                    {
                        localSource = child;
                    }
                    else if (firstRemoteSource == nullptr && child->data(SourceTreeEntrySourceTypeRole).toString().toStdString() == ZapFR::Engine::IdentifierRemoteServer)
                    {
                        firstRemoteSource = child;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            QStandardItem* toSelect{nullptr};
            if (localSource != nullptr && (localSource->rowCount() > 0 || firstRemoteSource == nullptr))
            {
                toSelect = localSource;
            }
            else if (firstRemoteSource != nullptr)
            {
                toSelect = firstRemoteSource;
            }
            ui->treeViewSources->setCurrentIndex(mProxyModelSources->mapFromSource(mItemModelSources->indexFromItem(toSelect)));
        }
    }
}

QJsonArray ZapFR::Client::MainWindow::expandedSourceTreeItems() const
{
    QJsonArray expandedSourceTreeItems;
    if (mItemModelSources == nullptr)
    {
        return expandedSourceTreeItems;
    }

    std::function<void(QStandardItem*)> processExpansionStates;
    processExpansionStates = [&](QStandardItem* parent)
    {
        if (parent->hasChildren())
        {
            auto index = mProxyModelSources->mapFromSource(mItemModelSources->indexFromItem(parent));
            if (index.isValid())
            {
                if (ui->treeViewSources->isExpanded(index))
                {
                    if (parent->data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_SOURCE)
                    {
                        QJsonObject o;
                        o.insert("type", "source");
                        o.insert("id", QJsonValue::fromVariant(parent->data(SourceTreeEntryIDRole)));
                        expandedSourceTreeItems.append(o);
                    }
                    else if (parent->data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FOLDER)
                    {
                        QJsonObject o;
                        o.insert("type", "folder");
                        o.insert("sourceID", QJsonValue::fromVariant(parent->data(SourceTreeEntryParentSourceIDRole)));
                        o.insert("id", QJsonValue::fromVariant(parent->data(SourceTreeEntryIDRole)));
                        expandedSourceTreeItems.append(o);
                    }
                }
            }

            for (auto i = 0; i < parent->rowCount(); ++i)
            {
                auto child = parent->child(i);
                processExpansionStates(child);
            }
        }
    };
    processExpansionStates(mItemModelSources->invisibleRootItem());
    return expandedSourceTreeItems;
}

void ZapFR::Client::MainWindow::expandSourceTreeItems(const QJsonArray& items) const
{
    std::function<void(QStandardItem*)> processExpansionStates;
    processExpansionStates = [&](QStandardItem* parent)
    {
        if (parent->hasChildren())
        {
            auto index = mProxyModelSources->mapFromSource(mItemModelSources->indexFromItem(parent));
            if (index.isValid())
            {
                auto idToMatch = parent->data(SourceTreeEntryIDRole).toULongLong();
                auto sourceIDToMatch = parent->data(SourceTreeEntryParentSourceIDRole).toULongLong();
                uint64_t folderIDToMatch = 0;
                auto typeToMatch = QString("");
                if (parent->data(SourceTreeEntryTypeRole).toULongLong() == SOURCETREE_ENTRY_TYPE_SOURCE)
                {
                    typeToMatch = "source";
                }
                else if (parent->data(SourceTreeEntryTypeRole).toULongLong() == SOURCETREE_ENTRY_TYPE_FOLDER)
                {
                    typeToMatch = "folder";
                    folderIDToMatch = parent->data(SourceTreeEntryIDRole).toULongLong();
                }

                for (const auto& entry : items)
                {
                    auto o = entry.toObject();
                    auto type = o.value("type").toString();
                    if (type == typeToMatch)
                    {
                        auto shouldExpand{false};

                        if (type == "source")
                        {
                            auto id = o.value("id").toVariant().toULongLong();
                            shouldExpand = id == idToMatch;
                        }
                        else if (type == "folder")
                        {
                            auto sourceID = o.value("sourceID").toVariant().toULongLong();
                            auto folderID = o.value("id").toVariant().toULongLong();
                            shouldExpand = (sourceID == sourceIDToMatch && folderID == folderIDToMatch);
                        }

                        if (shouldExpand)
                        {
                            ui->treeViewSources->setExpanded(index, true);
                        }
                    }
                }
            }
            for (auto i = 0; i < parent->rowCount(); ++i)
            {
                auto child = parent->child(i);
                processExpansionStates(child);
            }
        }
    };
    processExpansionStates(mItemModelSources->invisibleRootItem());
}

void ZapFR::Client::MainWindow::setUnreadBadgesShown(bool b)
{
    std::function<void(QStandardItem*)> updateShowThreadBadgeState;
    updateShowThreadBadgeState = [&](QStandardItem* parent)
    {
        auto index = mItemModelSources->indexFromItem(parent);
        switch (index.data(SourceTreeEntryTypeRole).toULongLong())
        {
            case SOURCETREE_ENTRY_TYPE_FEED:
            {
                parent->setData(b, SourceTreeEntryDisplayUnreadCountBadge);
                break;
            }
            case SOURCETREE_ENTRY_TYPE_FOLDER:
            case SOURCETREE_ENTRY_TYPE_SOURCE:
            {
                for (int32_t i = 0; i < parent->rowCount(); ++i)
                {
                    auto child = parent->child(i, 0);
                    updateShowThreadBadgeState(child);
                }
                break;
            }
        }
    };

    if (mItemModelSources != nullptr)
    {
        for (int32_t i = 0; i < mItemModelSources->rowCount(); ++i)
        {
            auto item = mItemModelSources->item(i, 0);
            updateShowThreadBadgeState(item);
        }
    }
}

void ZapFR::Client::MainWindow::addSource()
{
    if (mDialogAddSource == nullptr)
    {
        mDialogAddSource = std::make_unique<DialogAddSource>(this);
        connect(mDialogAddSource.get(), &QDialog::finished,
                [&](int result)
                {
                    if (result == QDialog::DialogCode::Accepted)
                    {
                        auto sourceType = mDialogAddSource->sourceType().toStdString();
                        auto sourceTitle = mDialogAddSource->serverName().toStdString();
                        if (sourceTitle.empty())
                        {
                            sourceTitle = tr("Unnamed server").toStdString();
                        }
                        auto hostName = mDialogAddSource->hostName();
                        if (!hostName.isEmpty())
                        {
                            auto configData = QJsonObject();
                            configData["host"] = hostName;
                            configData["port"] = mDialogAddSource->port();
                            configData["login"] = mDialogAddSource->login();
                            configData["password"] = mDialogAddSource->password();
                            configData["useHTTPS"] = mDialogAddSource->useHTTPS();
                            auto configDataStr = QJsonDocument(configData).toJson(QJsonDocument::Compact).toStdString();
                            ZapFR::Engine::Source::create(sourceType, sourceTitle, configDataStr);
                            reloadSources();
                        }
                    }
                });
    }

    mDialogAddSource->reset();
    mDialogAddSource->open();
}

void ZapFR::Client::MainWindow::removeSource()
{
    auto index = ui->treeViewSources->currentIndex();
    if (index.isValid())
    {
        auto sourceType = index.data(SourceTreeEntrySourceTypeRole).toString().toStdString();
        if (sourceType == ZapFR::Engine::IdentifierLocalServer)
        {
            QMessageBox::information(this, tr("Can't remove local source"), tr("You cannot remove the local source"));
            return;
        }

        QMessageBox messageBox;
        messageBox.setWindowTitle(tr("Remove source"));
        messageBox.setText(tr("Remove source?"));
        messageBox.setInformativeText(tr("Are you sure you want to remove this remote source?"));
        messageBox.setIcon(QMessageBox::Warning);
        auto yesButton = messageBox.addButton(QMessageBox::StandardButton::Yes);
        yesButton->setText(tr("Remove"));
        messageBox.addButton(QMessageBox::StandardButton::Cancel);
        auto messageBoxLayout = qobject_cast<QGridLayout*>(messageBox.layout());
        messageBoxLayout->addItem(new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), messageBoxLayout->rowCount(), 0, 1, messageBoxLayout->columnCount());
        messageBox.exec();
        if (messageBox.buttonRole(messageBox.clickedButton()) == QMessageBox::YesRole)
        {
            auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();

            // deleting a source does not need to go over an agent, as it's only possible to delete remote sources that are stored in the local database
            ZapFR::Engine::Source::removeSource(sourceID);
            reloadSources();
        }
    }
}

void ZapFR::Client::MainWindow::sourceMarkedRead(uint64_t sourceID)
{
    updateFeedUnreadCountBadge(sourceID, {}, true, 0);
    mCurrentPostPage = 1;
    reloadPosts();
}

QStandardItem* ZapFR::Client::MainWindow::findSourceStandardItem(uint64_t sourceID)
{
    QStandardItem* sourceItem{nullptr};
    auto root = mItemModelSources->invisibleRootItem();
    for (int32_t i = 0; i < root->rowCount(); ++i)
    {
        auto child = root->child(i, 0);
        if (child != nullptr)
        {
            auto index = mItemModelSources->indexFromItem(child);
            if (index.isValid())
            {
                auto type = index.data(SourceTreeEntryTypeRole).toULongLong();
                if (type == SOURCETREE_ENTRY_TYPE_SOURCE)
                {
                    auto id = index.data(SourceTreeEntryIDRole).toULongLong();
                    if (sourceID == id)
                    {
                        sourceItem = child;
                        break;
                    }
                }
            }
        }
    }
    return sourceItem;
}

void ZapFR::Client::MainWindow::connectSourceStuff()
{
    connect(ui->action_Add_source, &QAction::triggered, this, &MainWindow::addSource);
    connect(ui->action_Remove_source, &QAction::triggered, this, &MainWindow::removeSource);

    connect(ui->treeViewSources, &TreeViewSources::deletePressed,
            [&]()
            {
                auto currentIndex = ui->treeViewSources->currentIndex();
                if (currentIndex.isValid())
                {
                    auto type = currentIndex.data(SourceTreeEntryTypeRole).toULongLong();
                    switch (type)
                    {
                        case SOURCETREE_ENTRY_TYPE_FOLDER:
                        {
                            removeFolder();
                            break;
                        }
                        case SOURCETREE_ENTRY_TYPE_FEED:
                        {
                            removeFeed();
                            break;
                        }
                        case SOURCETREE_ENTRY_TYPE_SOURCE:
                        {
                            removeSource();
                            break;
                        }
                    }
                }
            });

    connect(ui->treeViewSources, &TreeViewSources::customContextMenuRequested,
            [&](const QPoint& p)
            {
                if (ui->stackedWidgetContentPanes->currentIndex() == StackedPanePosts)
                {
                    auto index = ui->treeViewSources->indexAt(p);
                    if (index.isValid())
                    {
                        auto type = index.data(SourceTreeEntryTypeRole).toULongLong();
                        switch (type)
                        {
                            case SOURCETREE_ENTRY_TYPE_FEED:
                            {
                                mSourceContextMenuFeed->popup(ui->treeViewSources->viewport()->mapToGlobal(p));
                                break;
                            }
                            case SOURCETREE_ENTRY_TYPE_FOLDER:
                            {
                                mSourceContextMenuFolder->popup(ui->treeViewSources->viewport()->mapToGlobal(p));
                                break;
                            }
                            case SOURCETREE_ENTRY_TYPE_SOURCE:
                            {
                                mSourceContextMenuSource->popup(ui->treeViewSources->viewport()->mapToGlobal(p));
                                break;
                            }
                        }
                    }
                }
            });

    connect(ui->treeViewSources, &TreeViewSources::currentSourceChanged,
            [&](const QModelIndex& index)
            {
                if (index.isValid())
                {
                    switch (ui->stackedWidgetContentPanes->currentIndex())
                    {
                        case StackedPanePosts:
                        {
                            ui->tableViewScriptFolders->setCurrentIndex(QModelIndex());
                            mCurrentPostPage = 1;
                            reloadPosts();
                            reloadUsedFlagColors();
                            reloadScriptFolders();
                            updateActivePostFilter();
                            break;
                        }
                        case StackedPaneLogs:
                        {
                            mCurrentLogPage = 1;
                            reloadLogs();
                            break;
                        }
                        case StackedPaneScripts:
                        {
                            reloadScripts();
                            break;
                        }
                        case StackedPaneProperties:
                        {
                            reloadPropertiesPane();
                            break;
                        }
                    }
                }
            });
}

void ZapFR::Client::MainWindow::createSourceContextMenus()
{
    mSourceContextMenuSource = std::make_unique<QMenu>(nullptr);
    mSourceContextMenuSource->addAction(ui->action_Refresh_feeds);
    mSourceContextMenuSource->addAction(ui->action_Mark_as_read);
    mSourceContextMenuSource->addSeparator();
    mSourceContextMenuSource->addAction(ui->action_Add_feed);
    mSourceContextMenuSource->addAction(ui->action_Add_folder);
    mSourceContextMenuSource->addSeparator();
    mSourceContextMenuSource->addAction(ui->action_Remove_source);
    mSourceContextMenuSource->addSeparator();
    mSourceContextMenuSource->addAction(ui->action_View_logs);
    mSourceContextMenuSource->addAction(ui->action_View_properties);
}

void ZapFR::Client::MainWindow::initializeUISources()
{
    ui->treeViewSources->setItemDelegate(new ItemDelegateSource(ui->treeViewSources));

    // prevent the left splitter from resizing while the window resizes
    ui->splitterSourcesAndContentPanes->setStretchFactor(1, 100);

    mItemModelSources = std::make_unique<StandardItemModelSources>(this, this);
    mProxyModelSources = std::make_unique<SortFilterProxyModelSources>(this);
    mProxyModelSources->setSourceModel(mItemModelSources.get());
    ui->treeViewSources->setModel(mProxyModelSources.get());
    ui->treeViewSources->sortByColumn(0, Qt::AscendingOrder);
}
