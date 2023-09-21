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

#include "widgets/TreeViewSources.h"
#include "./ui_MainWindow.h"
#include "FeedIconCache.h"
#include "ZapFR/Agent.h"
#include "ZapFR/base/Feed.h"
#include "ZapFR/base/Folder.h"
#include "ZapFR/base/Source.h"
#include "delegates/ItemDelegateSource.h"
#include "dialogs/DialogAddFeed.h"
#include "dialogs/DialogAddFolder.h"
#include "dialogs/DialogAddSource.h"
#include "dialogs/DialogEditFolder.h"
#include "dialogs/DialogImportOPML.h"
#include "models/SortFilterProxyModelSources.h"
#include "models/StandardItemModelSources.h"
#include "widgets/MainWindow.h"
#include "widgets/WidgetPropertiesPaneFeed.h"
#include "widgets/WidgetPropertiesPaneFolder.h"
#include "widgets/WidgetPropertiesPaneSource.h"

ZapFR::Client::TreeViewSources::TreeViewSources(QWidget* parent) : TreeViewPaletteCorrected(parent)
{
    setItemDelegate(new ItemDelegateSource(this));

    mActionAddSource = std::make_unique<QAction>(tr("Add source"), this);
    mActionRemoveSource = std::make_unique<QAction>(tr("Remove source"), this);
    mActionAddFeed = std::make_unique<QAction>(tr("Add feed"), this);
    mActionRemoveFeed = std::make_unique<QAction>(tr("Remove feed"), this);
    mActionAddFolder = std::make_unique<QAction>(tr("Add folder"), this);
    mActionEditFolder = std::make_unique<QAction>(tr("Edit folder"), this);
    mActionRemoveFolder = std::make_unique<QAction>(tr("Remove folder"), this);
    mActionToolbarRefresh = std::make_unique<QAction>(tr("Refresh"), this);
    mActionRefresh = std::make_unique<QAction>(tr("Refresh"), this);
    mActionReconnectToSource = std::make_unique<QAction>(tr("Reconnect to source"), this);
    mActionImportOPML = std::make_unique<QAction>(tr("Import OPML"), this);
    mActionExportOPML = std::make_unique<QAction>(tr("Export OPML"), this);
    mActionViewProperties = std::make_unique<QAction>(tr("View properties"), this);
}

void ZapFR::Client::TreeViewSources::setMainWindow(MainWindow* mw) noexcept
{
    mMainWindow = mw;

    mItemModelSources = std::make_unique<StandardItemModelSources>(mMainWindow, this);
    mProxyModelSources = std::make_unique<SortFilterProxyModelSources>(this);
    mProxyModelSources->setSourceModel(mItemModelSources.get());
    setModel(mProxyModelSources.get());
    sortByColumn(0, Qt::AscendingOrder);

    connectStuff();
    createContextMenus();
}

void ZapFR::Client::TreeViewSources::currentChanged(const QModelIndex& current, const QModelIndex& /*previous*/)
{
    emit currentSourceChanged(current);
}

void ZapFR::Client::TreeViewSources::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
        emit deletePressed();
    }
    else
    {
        QTreeView::keyPressEvent(event);
    }
}

void ZapFR::Client::TreeViewSources::mouseDoubleClickEvent(QMouseEvent* event)
{
    auto index = currentIndex();
    if (index.isValid())
    {
        auto type = index.data(Role::Type).toULongLong();
        if (type == TreeViewSources::EntryType::Folder)
        {
            emit folderDoubleClicked();
            return;
        }
    }

    TreeViewPaletteCorrected::mouseDoubleClickEvent(event);
}

void ZapFR::Client::TreeViewSources::reload()
{
    preserveExpansionSelectionState();

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
                    folderItem->setData(TreeViewSources::EntryType::Folder, Role::Type);
                    folderItem->setData(QVariant::fromValue<uint64_t>(folder->id()), Role::ID);
                    folderItem->setData(QVariant::fromValue<uint64_t>(folder->parentID()), Role::ParentFolderID);
                    folderItem->setData(QVariant::fromValue<uint64_t>(currentSourceID), Role::ParentSourceID);
                    folderItem->setData(QVariant::fromValue<uint64_t>(folder->sortOrder()), Role::SortOrder);
                    parentItem->appendRow(folderItem);

                    for (const auto& subfolder : folder->subfolders())
                    {
                        createFolderItems(subfolder.get(), currentSourceID, folderItem);
                    }
                    folderIDToItemMapping[folder->id()] = folderItem;
                };
                // create the parent source item
                auto sourceItem = new QStandardItem(QString::fromUtf8(retrievedSource->title()));
                sourceItem->setData(TreeViewSources::EntryType::Source, Role::Type);
                sourceItem->setData(QVariant::fromValue<uint64_t>(retrievedSource->id()), Role::ID);
                sourceItem->setData(QVariant::fromValue<uint64_t>(retrievedSource->id()), Role::ParentSourceID);
                sourceItem->setData(QVariant::fromValue<uint64_t>(retrievedSource->sortOrder()), Role::SortOrder);
                sourceItem->setData(QString::fromUtf8(retrievedSource->type()), Role::SourceType);
                const auto& lastError = retrievedSource->lastError();
                if (!lastError.empty())
                {
                    auto errorQString = QString::fromUtf8(lastError);
                    sourceItem->setData(errorQString, Role::Error);
                    sourceItem->setData(errorQString, Qt::ToolTipRole);
                }

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
                    feedItem->setData(TreeViewSources::EntryType::Feed, Role::Type);
                    feedItem->setData(QVariant::fromValue<uint64_t>(feed->id()), Role::ID);
                    feedItem->setData(QVariant::fromValue<uint64_t>(retrievedSource->id()), Role::ParentSourceID);
                    feedItem->setData(QVariant::fromValue<uint64_t>(folderID), Role::ParentFolderID);
                    auto unreadCount = feed->unreadCount();
                    feedItem->setData(QVariant::fromValue<uint64_t>(unreadCount), Role::UnreadCount);
                    if (unreadCount >= 999)
                    {
                        feedItem->setToolTip(tr("%1 unread").arg(unreadCount));
                    }
                    if (mMainWindow->currentContentPane() == ContentPane::Posts)
                    {
                        feedItem->setData(true, Role::DisplayUnreadCountBadge);
                    }
                    auto feedError = feed->lastRefreshError();
                    if (feedError.has_value())
                    {
                        feedItem->setData(QString::fromUtf8(feedError.value()), Role::Error);
                        feedItem->setData(QString::fromUtf8(feedError.value()), Qt::ToolTipRole);
                    }
                    feedItem->setData(QString::fromUtf8(feed->url()), Role::FeedURL);
                    feedItem->setData(QVariant::fromValue<uint64_t>(feed->sortOrder()), Role::SortOrder);

                    if (!FeedIconCache::isCached(retrievedSource->id(), feed->id()) || !FeedIconCache::isSameHash(retrievedSource->id(), feed->id(), feed->iconHash()))
                    {
                        auto iconData = feed->iconData();
                        if (!iconData.empty())
                        {
                            QPixmap icon;
                            icon.loadFromData(QByteArray(iconData.c_str(), static_cast<int64_t>(iconData.length())));
                            if (!icon.isNull())
                            {
                                FeedIconCache::cache(retrievedSource->id(), feed->id(), feed->iconHash(), icon);
                            }
                        }
                    }
                    parentItem->appendRow(feedItem);
                }
                QMetaObject::invokeMethod(this, [=, this]() { populateSources(retrievedSource->id(), sourceItem); });
            });
    }
}

void ZapFR::Client::TreeViewSources::populateSources(uint64_t /*sourceID*/, QStandardItem* sourceItem)
{
    mItemModelSources->appendRow(sourceItem);
    restoreExpansionSelectionState(sourceItem);
}

void ZapFR::Client::TreeViewSources::preserveExpansionSelectionState()
{
    if (mDisplayMode == DisplayMode::ShowAll)
    {
        mReloadExpansionSelectionState = std::make_unique<QJsonObject>();
        mReloadExpansionSelectionState->insert("expanded", expandedItems());
        uint64_t selectedSourceID = 0;
        uint64_t selectedID = 0;
        uint32_t selectedType = 0;
        auto index = currentIndex();
        if (index.isValid())
        {
            selectedSourceID = index.data(Role::ParentSourceID).toULongLong();
            selectedID = index.data(Role::ID).toULongLong();
            selectedType = index.data(Role::Type).toUInt();
        }
        mReloadExpansionSelectionState->insert("selectedSourceID", static_cast<qint64>(selectedSourceID));
        mReloadExpansionSelectionState->insert("selectedID", static_cast<qint64>(selectedID));
        mReloadExpansionSelectionState->insert("selectedType", static_cast<int32_t>(selectedType));
    }
}

void ZapFR::Client::TreeViewSources::restoreExpansionSelectionState(QStandardItem* /*sourceItem*/)
{
    if (mReloadExpansionSelectionState != nullptr && mDisplayMode == DisplayMode::ShowAll)
    {
        auto expandedItems = mReloadExpansionSelectionState->value("expanded").toArray();
        auto selectedSourceID = mReloadExpansionSelectionState->value("selectedSourceID").toInteger();
        auto selectedID = mReloadExpansionSelectionState->value("selectedID").toInteger();
        auto selectedType = mReloadExpansionSelectionState->value("selectedType").toInteger();

        expandItems(expandedItems);
        if (selectedSourceID != 0 && selectedID != 0)
        {
            std::function<void(QStandardItem*)> selectIndex;
            selectIndex = [&](QStandardItem* parent)
            {
                if (parent->data(Role::Type).toUInt() == selectedType && parent->data(Role::ParentSourceID).toLongLong() == selectedSourceID &&
                    parent->data(Role::ID).toLongLong() == selectedID)
                {
                    auto indexToSelect = mProxyModelSources->mapFromSource(mItemModelSources->indexFromItem(parent));
                    setCurrentIndex(indexToSelect);
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
        if (!currentIndex().isValid() && mItemModelSources->invisibleRootItem()->rowCount() == static_cast<int32_t>(mInitialSourceCount))
        {
            QStandardItem* localSource{nullptr};
            QStandardItem* firstRemoteSource{nullptr};
            for (auto i = 0; i < mItemModelSources->invisibleRootItem()->rowCount(); ++i)
            {
                auto child = mItemModelSources->invisibleRootItem()->child(i);
                if (child->data(Role::Type).toULongLong() == TreeViewSources::EntryType::Source)
                {
                    if (child->data(Role::SourceType).toString().toStdString() == ZapFR::Engine::IdentifierLocalServer)
                    {
                        localSource = child;
                    }
                    else if (firstRemoteSource == nullptr && child->data(Role::SourceType).toString().toStdString() == ZapFR::Engine::IdentifierRemoteServer)
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
            setCurrentIndex(mProxyModelSources->mapFromSource(mItemModelSources->indexFromItem(toSelect)));
        }
    }
}

QJsonArray ZapFR::Client::TreeViewSources::expandedItems() const
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
                if (isExpanded(index))
                {
                    if (parent->data(Role::Type) == TreeViewSources::EntryType::Source)
                    {
                        QJsonObject o;
                        o.insert("type", "source");
                        o.insert("id", QJsonValue::fromVariant(parent->data(Role::ID)));
                        expandedSourceTreeItems.append(o);
                    }
                    else if (parent->data(Role::Type) == TreeViewSources::EntryType::Folder)
                    {
                        QJsonObject o;
                        o.insert("type", "folder");
                        o.insert("sourceID", QJsonValue::fromVariant(parent->data(Role::ParentSourceID)));
                        o.insert("id", QJsonValue::fromVariant(parent->data(Role::ID)));
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

void ZapFR::Client::TreeViewSources::expandItems(const QJsonArray& items)
{
    std::function<void(QStandardItem*)> processExpansionStates;
    processExpansionStates = [&](QStandardItem* parent)
    {
        if (parent->hasChildren())
        {
            auto index = mProxyModelSources->mapFromSource(mItemModelSources->indexFromItem(parent));
            if (index.isValid())
            {
                auto idToMatch = parent->data(Role::ID).toULongLong();
                auto sourceIDToMatch = parent->data(Role::ParentSourceID).toULongLong();
                uint64_t folderIDToMatch = 0;
                auto typeToMatch = QString("");
                if (parent->data(Role::Type).toULongLong() == TreeViewSources::EntryType::Source)
                {
                    typeToMatch = "source";
                }
                else if (parent->data(Role::Type).toULongLong() == TreeViewSources::EntryType::Folder)
                {
                    typeToMatch = "folder";
                    folderIDToMatch = parent->data(Role::ID).toULongLong();
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
                            setExpanded(index, true);
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

std::tuple<uint64_t, uint64_t> ZapFR::Client::TreeViewSources::getCurrentlySelectedSourceAndFolderID() const
{
    uint64_t sourceID{0};
    uint64_t folderID{0};
    auto index = currentIndex();
    if (index.isValid())
    {
        sourceID = index.data(Role::ParentSourceID).toULongLong();
        auto type = index.data(Role::Type);
        if (type == TreeViewSources::EntryType::Folder)
        {
            folderID = index.data(Role::ID).toULongLong();
        }
        else if (type == TreeViewSources::EntryType::Feed)
        {
            folderID = index.data(Role::ParentFolderID).toULongLong();
        }
    }
    return std::make_tuple(sourceID, folderID);
}

void ZapFR::Client::TreeViewSources::setUnreadBadgesShown(bool b)
{
    std::function<void(QStandardItem*)> updateShowThreadBadgeState;
    updateShowThreadBadgeState = [&](QStandardItem* parent)
    {
        auto index = mItemModelSources->indexFromItem(parent);
        switch (index.data(Role::Type).toULongLong())
        {
            case TreeViewSources::EntryType::Feed:
            {
                parent->setData(b, Role::DisplayUnreadCountBadge);
                break;
            }
            case TreeViewSources::EntryType::Folder:
            case TreeViewSources::EntryType::Source:
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

void ZapFR::Client::TreeViewSources::setAllowDragAndDrop(bool b)
{
    mItemModelSources->setAllowDragAndDrop(b);
}

void ZapFR::Client::TreeViewSources::setDisplayMode(DisplayMode dm)
{
    mDisplayMode = dm;
    mProxyModelSources->setDisplayMode(dm);
    switch (dm)
    {
        case DisplayMode::ShowAll:
        {
            restoreExpansionSelectionState(nullptr);
            mItemModelSources->setHorizontalHeaderItem(0, new QStandardItem(tr("Sources & Feeds")));
            break;
        }
        case DisplayMode::ShowSourcesOnly:
        {
            preserveExpansionSelectionState();
            uint64_t currentParentSource{0};
            auto index = currentIndex();
            if (index.isValid())
            {
                currentParentSource = index.data(Role::ParentSourceID).toULongLong();
            }

            mItemModelSources->setHorizontalHeaderItem(0, new QStandardItem(tr("Sources")));

            // auto select the source for the item that was currently selected, so the selection isn't empty after
            // only showing sources
            if (currentParentSource != 0)
            {
                auto rootItem = mItemModelSources->invisibleRootItem();
                for (int32_t i = 0; i < rootItem->rowCount(); ++i)
                {
                    auto child = rootItem->child(i);
                    if (child->data(Role::ParentSourceID).toULongLong() == currentParentSource)
                    {
                        setCurrentIndex(mProxyModelSources->mapFromSource(mItemModelSources->indexFromItem(child)));
                        break;
                    }
                }
            }
            break;
        }
    }
}

void ZapFR::Client::TreeViewSources::agentErrorOccurred(uint64_t sourceID, const std::string& errorMessage)
{
    auto rootItem = mItemModelSources->invisibleRootItem();
    for (auto i = 0; i < rootItem->rowCount(); ++i)
    {
        auto child = rootItem->child(i);
        auto type = child->data(Role::Type).toULongLong();
        auto id = child->data(Role::ID).toULongLong();
        if (type == TreeViewSources::EntryType::Source && id == sourceID)
        {
            child->setData(QString::fromUtf8(errorMessage), Role::Error);
            child->setData(QString::fromUtf8(errorMessage), Qt::ToolTipRole);
            break;
        }
    }
}

void ZapFR::Client::TreeViewSources::saveSettings(QJsonObject& root)
{
    switch (mDisplayMode)
    {
        case DisplayMode::ShowAll:
        {
            root.insert(SETTING_SOURCETREEVIEW_EXPANSION, expandedItems());
            break;
        }
        case DisplayMode::ShowSourcesOnly:
        {
            if (mReloadExpansionSelectionState != nullptr)
            {
                auto expandedItems = mReloadExpansionSelectionState->value("expanded").toArray();
                root.insert(SETTING_SOURCETREEVIEW_EXPANSION, expandedItems);
            }
            break;
        }
    }
}

void ZapFR::Client::TreeViewSources::restoreSettings(const QJsonObject& root)
{
    if (root.contains(SETTING_SOURCETREEVIEW_EXPANSION))
    {
        // instead of doing this immediately, write the array to mReloadSourcesExpansionSelectionState
        // so that it will get picked up by reload(), which happens after restoring the settings
        mReloadExpansionSelectionState = std::make_unique<QJsonObject>();
        mReloadExpansionSelectionState->insert("expanded", root.value(SETTING_SOURCETREEVIEW_EXPANSION).toArray());
        mReloadExpansionSelectionState->insert("selectedSourceID", 0);
        mReloadExpansionSelectionState->insert("selectedID", 0);
        mReloadExpansionSelectionState->insert("selectedType", 0);
    }
}

void ZapFR::Client::TreeViewSources::addSource()
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
                            configData[ZapFR::Engine::JSONIdentifierRemoteConfigDataHost] = hostName;
                            configData[ZapFR::Engine::JSONIdentifierRemoteConfigDataPort] = mDialogAddSource->port();
                            configData[ZapFR::Engine::JSONIdentifierRemoteConfigDataLogin] = mDialogAddSource->login();
                            configData[ZapFR::Engine::JSONIdentifierRemoteConfigDataPassword] = mDialogAddSource->password();
                            configData[ZapFR::Engine::JSONIdentifierRemoteConfigDataUseHTTPS] = mDialogAddSource->useHTTPS();
                            auto configDataStr = QJsonDocument(configData).toJson(QJsonDocument::Compact).toStdString();
                            ZapFR::Engine::Source::create(sourceType, sourceTitle, configDataStr);
                            reload();
                        }
                    }
                });
    }

    mDialogAddSource->reset();
    mDialogAddSource->open();
}

void ZapFR::Client::TreeViewSources::removeSource()
{
    auto index = currentIndex();
    if (index.isValid())
    {
        auto sourceType = index.data(Role::SourceType).toString().toStdString();
        if (sourceType == ZapFR::Engine::IdentifierLocalServer)
        {
            QMessageBox::information(this, tr("Can't remove local source"), tr("You cannot remove the local source"));
            return;
        }

        QMessageBox messageBox(this);
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
            auto sourceID = index.data(Role::ParentSourceID).toULongLong();

            // deleting a source does not need to go over an agent, as it's only possible to delete remote sources that are stored in the local database
            ZapFR::Engine::Source::removeSource(sourceID);
            reload();
        }
    }
}

bool ZapFR::Client::TreeViewSources::doesSourceHaveError(uint64_t sourceID)
{
    auto root = mItemModelSources->invisibleRootItem();
    for (int32_t i = 0; i < root->rowCount(); ++i)
    {
        auto child = root->child(i);
        if (child->data(Role::Type).toULongLong() == TreeViewSources::EntryType::Source && child->data(Role::ID).toULongLong() == sourceID)
        {
            auto error = child->data(Role::Error);
            return (!error.isNull() && error.isValid() && !error.toString().isEmpty());
        }
    }
    return false;
}

void ZapFR::Client::TreeViewSources::remoteSourceUnreadCountsReceived(uint64_t affectedSourceID, const std::unordered_map<uint64_t, uint64_t>& unreadCounts)
{
    std::function<void(QStandardItem*)> updateBadges;
    updateBadges = [&](QStandardItem* parent)
    {
        auto type = parent->data(Role::Type).toULongLong();
        auto parentSourceID = parent->data(Role::ParentSourceID).toULongLong();
        if (parentSourceID == affectedSourceID)
        {
            switch (type)
            {
                case TreeViewSources::EntryType::Feed:
                {
                    auto parentFeedID = parent->data(Role::ID).toULongLong();
                    for (const auto& [feedID, unreadCount] : unreadCounts)
                    {
                        if (feedID == parentFeedID)
                        {
                            parent->setData(QVariant::fromValue<uint64_t>(unreadCount), Role::UnreadCount);
                            break;
                        }
                    }
                    break;
                }
                case TreeViewSources::EntryType::Folder:
                case TreeViewSources::EntryType::Source:
                {
                    for (int32_t i = 0; i < parent->rowCount(); ++i)
                    {
                        auto child = parent->child(i);
                        updateBadges(child);
                    }
                    break;
                }
            }
        }
    };

    auto rootParent = mItemModelSources->invisibleRootItem();
    for (int32_t i = 0; i < rootParent->rowCount(); ++i)
    {
        auto rootChild = rootParent->child(i);
        updateBadges(rootChild);
    }
}

QStandardItem* ZapFR::Client::TreeViewSources::findSourceStandardItem(uint64_t sourceID)
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
                auto type = index.data(Role::Type).toULongLong();
                if (type == TreeViewSources::EntryType::Source)
                {
                    auto id = index.data(Role::ID).toULongLong();
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

std::unordered_set<QStandardItem*> ZapFR::Client::TreeViewSources::findFeedStandardItems(QStandardItem* sourceItem, const std::optional<std::unordered_set<uint64_t>>& feedIDs)
{
    std::unordered_set<QStandardItem*> feedItems;

    std::function<void(QStandardItem*)> findFeeds;
    findFeeds = [&](QStandardItem* parent)
    {
        auto type = parent->data(Role::Type).toULongLong();
        switch (type)
        {
            case TreeViewSources::EntryType::Source:
            case TreeViewSources::EntryType::Folder:
            {
                for (int32_t i = 0; i < parent->rowCount(); ++i)
                {
                    findFeeds(parent->child(i, 0));
                }
                break;
            }
            case TreeViewSources::EntryType::Feed:
            {
                auto id = parent->data(Role::ID).toULongLong();
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

void ZapFR::Client::TreeViewSources::cloneSourceTreeContents(uint64_t sourceID, QStandardItemModel* destination,
                                                             const std::optional<std::unordered_set<uint64_t>>& feedIDsToCheck)
{
    std::function<void(const QModelIndex&, QStandardItem*)> cloneItemAndChildren;
    cloneItemAndChildren = [&](const QModelIndex& parent, QStandardItem* destinationParentItem)
    {
        for (int32_t i = 0; i < mItemModelSources->rowCount(parent); ++i)
        {
            auto child = mItemModelSources->index(i, 0, parent);
            if (child.data(Role::ParentSourceID).toULongLong() == sourceID)
            {
                auto clone = mItemModelSources->itemFromIndex(child)->clone();

                if (feedIDsToCheck.has_value() && clone->data(Role::Type).toULongLong() == TreeViewSources::EntryType::Feed &&
                    feedIDsToCheck.value().contains(clone->data(Role::ID).toULongLong()))
                {
                    clone->setData(Qt::Checked, Qt::CheckStateRole);
                }
                else
                {
                    clone->setData(Qt::Unchecked, Qt::CheckStateRole);
                }
                destinationParentItem->appendRow(clone);
                cloneItemAndChildren(child, clone);
            }
        }
    };
    for (int32_t j = 0; j < mItemModelSources->invisibleRootItem()->rowCount(); ++j)
    {
        auto index = mItemModelSources->index(j, 0);
        cloneItemAndChildren(index, destination->invisibleRootItem());
    }
}

void ZapFR::Client::TreeViewSources::addFolder()
{
    if (mDialogAddFolder == nullptr)
    {
        mDialogAddFolder = std::make_unique<DialogAddFolder>(this);
        connect(mDialogAddFolder.get(), &QDialog::finished,
                [&](int result)
                {
                    if (result == QDialog::DialogCode::Accepted)
                    {
                        auto sourceID = mDialogAddFolder->selectedSourceID();
                        auto folderID = mDialogAddFolder->selectedFolderID();
                        auto title = mDialogAddFolder->title().toStdString();
                        if (!title.empty())
                        {
                            ZapFR::Engine::Agent::getInstance()->queueAddFolder(sourceID, folderID, title,
                                                                                [&]()
                                                                                {
                                                                                    QMetaObject::invokeMethod(this,
                                                                                                              [=, this]()
                                                                                                              {
                                                                                                                  reload();
                                                                                                                  mMainWindow->setStatusBarMessage(tr("Folder added"));
                                                                                                              });
                                                                                });
                        }
                    }
                });
    }

    auto [sourceID, folderID] = getCurrentlySelectedSourceAndFolderID();
    mDialogAddFolder->reset(sourceID, folderID);
    mDialogAddFolder->open();
}

void ZapFR::Client::TreeViewSources::editFolder()
{
    if (mDialogEditFolder == nullptr)
    {
        mDialogEditFolder = std::make_unique<DialogEditFolder>(this);
        connect(mDialogEditFolder.get(), &QDialog::finished,
                [&](int result)
                {
                    if (result == QDialog::DialogCode::Accepted)
                    {
                        auto sourceID = mDialogEditFolder->sourceID();
                        auto folderID = mDialogEditFolder->id();
                        auto title = mDialogEditFolder->title().toStdString();
                        if (!title.empty())
                        {
                            ZapFR::Engine::Agent::getInstance()->queueUpdateFolder(
                                sourceID, folderID, title,
                                [&](uint64_t affectedSourceID, uint64_t affectedFolderID, const std::string& updatedTitle)
                                {
                                    QMetaObject::invokeMethod(this,
                                                              [=, this]()
                                                              {
                                                                  std::function<void(QStandardItem*)> updateFolderTitle;
                                                                  updateFolderTitle = [&](QStandardItem* item)
                                                                  {
                                                                      for (int32_t i = 0; i < item->rowCount(); ++i)
                                                                      {
                                                                          auto child = item->child(i);
                                                                          auto childSourceID = child->data(Role::ParentSourceID).toULongLong();
                                                                          if (childSourceID != affectedSourceID)
                                                                          {
                                                                              continue;
                                                                          }

                                                                          auto childType = child->data(Role::Type).toULongLong();
                                                                          if (childType == TreeViewSources::EntryType::Folder)
                                                                          {
                                                                              auto childFolderID = child->data(Role::ID).toULongLong();
                                                                              if (childFolderID == affectedFolderID)
                                                                              {
                                                                                  child->setData(QString::fromUtf8(updatedTitle), Qt::DisplayRole);
                                                                                  return;
                                                                              }
                                                                              else
                                                                              {
                                                                                  updateFolderTitle(child);
                                                                              }
                                                                          }
                                                                          else if (childType == TreeViewSources::EntryType::Source)
                                                                          {
                                                                              updateFolderTitle(child);
                                                                          }
                                                                      }
                                                                  };
                                                                  updateFolderTitle(mItemModelSources->invisibleRootItem());
                                                              });
                                });
                        }
                    }
                });
    }

    auto index = currentIndex();
    if (index.isValid())
    {
        auto type = index.data(Role::Type);
        if (type == TreeViewSources::EntryType::Folder)
        {
            auto sourceID = index.data(Role::ParentSourceID).toULongLong();
            auto folderID = index.data(Role::ID).toULongLong();
            auto title = index.data(Qt::DisplayRole).toString();

            mDialogEditFolder->reset(sourceID, folderID, title);
            mDialogEditFolder->open();
        }
    }
}

void ZapFR::Client::TreeViewSources::removeFolder()
{
    QMessageBox messageBox(this);
    messageBox.setText(tr("Remove folder"));
    messageBox.setWindowTitle(tr("Remove folder"));
    messageBox.setInformativeText(tr("Are you sure you want to remove this folder, all its subfolders, and all feeds they contain? All associated posts will be removed!"));
    messageBox.setIcon(QMessageBox::Warning);
    auto yesButton = messageBox.addButton(QMessageBox::StandardButton::Yes);
    yesButton->setText(tr("Remove"));
    messageBox.addButton(QMessageBox::StandardButton::Cancel);
    auto messageBoxLayout = qobject_cast<QGridLayout*>(messageBox.layout());
    messageBoxLayout->addItem(new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), messageBoxLayout->rowCount(), 0, 1, messageBoxLayout->columnCount());
    messageBox.exec();
    if (messageBox.buttonRole(messageBox.clickedButton()) == QMessageBox::YesRole)
    {
        auto index = currentIndex();
        if (index.isValid())
        {
            auto sourceID = index.data(Role::ParentSourceID).toULongLong();
            auto folder = index.data(Role::ID).toULongLong();
            ZapFR::Engine::Agent::getInstance()->queueRemoveFolder(sourceID, folder,
                                                                   [&]()
                                                                   {
                                                                       QMetaObject::invokeMethod(this,
                                                                                                 [=, this]()
                                                                                                 {
                                                                                                     reload();
                                                                                                     mMainWindow->getUI()->tableViewPosts->clearPosts();
                                                                                                     mMainWindow->setStatusBarMessage(tr("Folder removed"));
                                                                                                 });
                                                                   });
        }
    }
}

void ZapFR::Client::TreeViewSources::addFeed()
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

void ZapFR::Client::TreeViewSources::feedAdded(uint64_t sourceID, uint64_t feedID)
{
    reload();
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

void ZapFR::Client::TreeViewSources::removeFeed()
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
        auto index = currentIndex();
        if (index.isValid())
        {
            auto sourceID = index.data(Role::ParentSourceID).toULongLong();
            auto feedID = index.data(Role::ID).toULongLong();
            ZapFR::Engine::Agent::getInstance()->queueRemoveFeed(sourceID, feedID,
                                                                 [&]()
                                                                 {
                                                                     QMetaObject::invokeMethod(this,
                                                                                               [=, this]()
                                                                                               {
                                                                                                   reload();
                                                                                                   mMainWindow->getUI()->tableViewPosts->clearPosts();
                                                                                                   mMainWindow->setStatusBarMessage(tr("Feed removed"));
                                                                                               });
                                                                 });
        }
    }
}

void ZapFR::Client::TreeViewSources::refreshViaToolbarButton()
{
    auto index = currentIndex();
    if (index.isValid())
    {
        auto type = index.data(Role::Type).toULongLong();
        if (mMainWindow->preferences()->refreshBehaviour == RefreshBehaviour::EntireSource)
        {
            type = TreeViewSources::EntryType::Source;
        }
        refreshSourceEntryType(index, type);
    }
}

void ZapFR::Client::TreeViewSources::refreshViaContextMenu()
{
    auto index = currentIndex();
    if (index.isValid())
    {
        auto type = index.data(Role::Type).toULongLong();
        refreshSourceEntryType(index, type);
    }
}

void ZapFR::Client::TreeViewSources::refreshSourceEntryType(const QModelIndex& index, uint64_t type)
{
    auto sourceID = index.data(Role::ParentSourceID).toULongLong();
    mMainWindow->setStatusBarMessage(tr("Refreshing '%1'").arg(index.data(Qt::DisplayRole).toString()));
    switch (type)
    {
        case TreeViewSources::EntryType::Feed:
        {
            auto feedID = index.data(Role::ID).toULongLong();
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
        case TreeViewSources::EntryType::Folder:
        {
            auto folderID = index.data(Role::ID).toULongLong();
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
        case TreeViewSources::EntryType::Source:
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

void ZapFR::Client::TreeViewSources::feedRefreshed(uint64_t sourceID, uint64_t feedID, uint64_t feedUnreadCount, const std::string& error, const std::string& feedTitle,
                                                   const std::string& iconHash, const std::string& iconData)
{
    mMainWindow->setStatusBarMessage(tr("Feed '%1' refreshed").arg(QString::fromUtf8(feedTitle)));
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
            feedItem->setData(QVariant::fromValue<uint64_t>(feedUnreadCount), Role::UnreadCount);
            if (!error.empty())
            {
                feedItem->setData(QString::fromUtf8(error), Role::Error);
                feedItem->setData(QString::fromUtf8(error), Qt::ToolTipRole);
            }
            else
            {
                feedItem->setData(QVariant(), Role::Error);
                feedItem->setData("", Qt::ToolTipRole);
            }

            // if the feed is currently selected, then refresh the posts so the new unread posts are shown
            // only do that when we're in the posts pane
            if (mMainWindow->currentContentPane() == ContentPane::Posts)
            {
                auto index = currentIndex();
                if (index.isValid() && mItemModelSources->itemFromIndex(mProxyModelSources->mapToSource(index)) == feedItem)
                {
                    mMainWindow->getUI()->tableViewPosts->setPage(1);
                    mMainWindow->getUI()->tableViewPosts->reload();
                }
            }
        }
    }
    mMainWindow->getUI()->tableViewScriptFolders->reload(true);
    mMainWindow->getUI()->frameFlagFilters->reload(true);
}

void ZapFR::Client::TreeViewSources::updateFeedUnreadCountBadge(uint64_t sourceID, std::unordered_set<uint64_t> feedIDs, bool markEntireSourceAsRead, uint64_t unreadCount)
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
        feedItem->setData(QVariant::fromValue<uint64_t>(unreadCount), Role::UnreadCount);
    }
}

void ZapFR::Client::TreeViewSources::importOPML()
{
    if (mDialogImportOPML == nullptr)
    {
        mDialogImportOPML = std::make_unique<DialogImportOPML>(this);
        connect(
            mDialogImportOPML.get(), &QDialog::finished,
            [&](int result)
            {
                if (result == QDialog::DialogCode::Accepted)
                {
                    ZapFR::Engine::Agent::getInstance()->queueImportOPML(
                        mDialogImportOPML->selectedSourceID(), mDialogImportOPML->OPML(), mDialogImportOPML->selectedFolderID(),
                        [&]() { QMetaObject::invokeMethod(this, [=, this]() { reload(); }); },
                        [&](uint64_t affectedSourceID, ZapFR::Engine::Feed* refreshedFeed)
                        {
                            auto id = refreshedFeed->id();
                            auto unreadCount = refreshedFeed->unreadCount();
                            auto error = refreshedFeed->lastRefreshError();
                            auto title = refreshedFeed->title();
                            auto iconHash = refreshedFeed->iconHash();
                            auto iconData = refreshedFeed->iconData();
                            QMetaObject::invokeMethod(
                                this, [=, this]() { feedRefreshed(affectedSourceID, id, unreadCount, error.has_value() ? error.value() : "", title, iconHash, iconData); });
                        });

                    // give it a bit of time to parse the OPML file, then start checking whether the refreshing has completed
                    QTimer::singleShot(
                        2500, [&]()
                        { ZapFR::Engine::Agent::getInstance()->queueMonitorFeedRefreshCompletion([&]() { QMetaObject::invokeMethod(this, [=, this]() { reload(); }); }); });
                }
            });
    }

    auto [sourceID, folderID] = getCurrentlySelectedSourceAndFolderID();
    mDialogImportOPML->reset(sourceID, folderID);
    mDialogImportOPML->open();
}

void ZapFR::Client::TreeViewSources::exportOPML()
{
    auto index = currentIndex();
    if (!index.isValid())
    {
        return;
    }

    auto sourceItem = findSourceStandardItem(index.data(TreeViewSources::Role::ParentSourceID).toULongLong());
    if (sourceItem == nullptr)
    {
        return;
    }

    auto opmlFilePath = QFileDialog::getSaveFileName(this, tr("Export source '%1'").arg(sourceItem->data(Qt::DisplayRole).toString()), QString(), tr("OPML files (*.opml)"));
    if (opmlFilePath.isEmpty())
    {
        return;
    }

    QDomDocument xml;

    // QDomProcessingInstruction documentation specifically states to not use createProcessingInstruction to create
    // the XML declaration, but doesn't bother explaining what should be done instead...
    xml.appendChild(xml.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\""));

    auto root = xml.createElement("opml");
    root.setAttribute("version", "2.0");
    xml.appendChild(root);

    auto head = xml.createElement("head");
    root.appendChild(head);

    auto title = xml.createElement("title");
    title.appendChild(xml.createTextNode(tr("ZapFeedReader export of source '%1'").arg(sourceItem->data(Qt::DisplayRole).toString())));
    head.appendChild(title);

    auto dateCreated = xml.createElement("dateCreated");
    dateCreated.appendChild(xml.createTextNode(QDateTime::currentDateTime().toString(Qt::RFC2822Date)));
    head.appendChild(dateCreated);

    auto body = xml.createElement("body");
    root.appendChild(body);

    // lambda to recursively add outlines
    std::function<void(QStandardItem*, QDomElement&)> addOutlines;
    addOutlines = [&](QStandardItem* parentItem, QDomElement& parentElement)
    {
        for (int32_t i = 0; i < parentItem->rowCount(); ++i)
        {
            auto child = parentItem->child(i);
            auto childType = child->data(TreeViewSources::Role::Type).toULongLong();
            switch (childType)
            {
                case TreeViewSources::EntryType::Folder:
                {
                    auto outline = xml.createElement("outline");
                    outline.setAttribute("text", child->data(Qt::DisplayRole).toString());
                    parentElement.appendChild(outline);
                    addOutlines(child, outline);
                    break;
                }
                case TreeViewSources::EntryType::Feed:
                {
                    auto outline = xml.createElement("outline");
                    outline.setAttribute("type", "rss");
                    outline.setAttribute("text", child->data(Qt::DisplayRole).toString());
                    outline.setAttribute("xmlUrl", child->data(TreeViewSources::Role::FeedURL).toString());
                    parentElement.appendChild(outline);
                    break;
                }
            }
        }
    };
    addOutlines(sourceItem, body);

    QFile opmlFile(opmlFilePath);
    if (opmlFile.open(QIODevice::WriteOnly | QIODeviceBase::Text))
    {
        QTextStream s(&opmlFile);
        s << xml.toString();
        opmlFile.close();
    }
}

void ZapFR::Client::TreeViewSources::reloadPropertiesPane()
{
    auto index = currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(Role::ParentSourceID).toULongLong();
        switch (index.data(Role::Type).toULongLong())
        {
            case EntryType::Source:
            {
                ZapFR::Engine::Agent::getInstance()->queueGetSource(
                    sourceID,
                    [&](ZapFR::Engine::Source* source)
                    {
                        QMap<QString, QVariant> props;
                        props["sourceID"] = QVariant::fromValue<uint64_t>(source->id());
                        props["title"] = QString::fromUtf8(source->title());
                        props["type"] = QString::fromUtf8(source->type());
                        props["configData"] = QString::fromUtf8(source->configData());

                        QMap<uint64_t, QString> stats;
                        for (const auto& [s, v] : source->statistics())
                        {
                            stats[static_cast<std::underlying_type_t<ZapFR::Engine::Folder::Statistic>>(s)] = QString::fromUtf8(v);
                        }
                        props["statistics"] = QVariant::fromValue<QMap<uint64_t, QString>>(stats);

                        QMetaObject::invokeMethod(this,
                                                  [=, this]()
                                                  {
                                                      mMainWindow->setContentPane(ContentPane::Properties);
                                                      auto ui = mMainWindow->getUI();
                                                      ui->stackedWidgetProperties->setCurrentIndex(PropertiesPane::SourceProps);
                                                      ui->widgetPropertiesPaneSource->reset(props);
                                                  });
                    });
                break;
            }
            case EntryType::Folder:
            {
                auto folderID = index.data(Role::ID).toULongLong();
                ZapFR::Engine::Agent::getInstance()->queueGetFolder(
                    sourceID, folderID,
                    [&](ZapFR::Engine::Folder* folder)
                    {
                        QMap<QString, QVariant> props;

                        props["sourceID"] = QVariant::fromValue<uint64_t>(sourceID);
                        props["folderID"] = QVariant::fromValue<uint64_t>(folder->id());
                        props["title"] = QString::fromUtf8(folder->title());

                        QMap<uint64_t, QString> stats;
                        for (const auto& [s, v] : folder->statistics())
                        {
                            stats[static_cast<std::underlying_type_t<ZapFR::Engine::Folder::Statistic>>(s)] = QString::fromUtf8(v);
                        }
                        props["statistics"] = QVariant::fromValue<QMap<uint64_t, QString>>(stats);

                        QMetaObject::invokeMethod(this,
                                                  [=, this]()
                                                  {
                                                      mMainWindow->setContentPane(ContentPane::Properties);
                                                      auto ui = mMainWindow->getUI();
                                                      ui->stackedWidgetProperties->setCurrentIndex(PropertiesPane::FolderProps);
                                                      ui->widgetPropertiesPaneFolder->reset(props);
                                                  });
                    });
                break;
            }
            case EntryType::Feed:
            {
                auto feedID = index.data(Role::ID).toULongLong();
                ZapFR::Engine::Agent::getInstance()->queueGetFeed(sourceID, feedID,
                                                                  [&](uint64_t retrievedSourceID, ZapFR::Engine::Feed* feed)
                                                                  {
                                                                      QMap<QString, QVariant> props;

                                                                      props["sourceID"] = QVariant::fromValue<uint64_t>(retrievedSourceID);
                                                                      props["feedID"] = QVariant::fromValue<uint64_t>(feed->id());
                                                                      props["title"] = QString::fromUtf8(feed->title());
                                                                      props["subtitle"] = QString::fromUtf8(feed->subtitle());
                                                                      props["url"] = QString::fromUtf8(feed->url());
                                                                      props["link"] = QString::fromUtf8(feed->link());
                                                                      props["description"] = QString::fromUtf8(feed->description());
                                                                      props["copyright"] = QString::fromUtf8(feed->copyright());
                                                                      props["lastRefreshed"] = QString::fromUtf8(feed->lastChecked());
                                                                      props["refreshInterval"] = "";
                                                                      if (feed->refreshInterval().has_value())
                                                                      {
                                                                          // convert seconds to minutes for the interface
                                                                          props["refreshInterval"] = QString::number(feed->refreshInterval().value() / 60);
                                                                      }
                                                                      props["lastError"] = "";
                                                                      if (feed->lastRefreshError().has_value())
                                                                      {
                                                                          props["lastError"] = QString::fromUtf8(feed->lastRefreshError().value());
                                                                      }

                                                                      QMap<uint64_t, QString> stats;
                                                                      for (const auto& [s, v] : feed->statistics())
                                                                      {
                                                                          stats[static_cast<std::underlying_type_t<ZapFR::Engine::Feed::Statistic>>(s)] = QString::fromUtf8(v);
                                                                      }
                                                                      props["statistics"] = QVariant::fromValue<QMap<uint64_t, QString>>(stats);

                                                                      QMetaObject::invokeMethod(this,
                                                                                                [=, this]()
                                                                                                {
                                                                                                    mMainWindow->setContentPane(ContentPane::Properties);
                                                                                                    auto ui = mMainWindow->getUI();
                                                                                                    ui->stackedWidgetProperties->setCurrentIndex(PropertiesPane::FeedProps);
                                                                                                    ui->widgetPropertiesPaneFeed->reset(props);
                                                                                                });
                                                                  });
                break;
            }
        }
    }
}

void ZapFR::Client::TreeViewSources::connectStuff()
{
    auto ui = mMainWindow->getUI();

    connect(mActionAddSource.get(), &QAction::triggered, this, &TreeViewSources::addSource);
    connect(mActionRemoveSource.get(), &QAction::triggered, this, &TreeViewSources::removeSource);
    connect(mActionAddFolder.get(), &QAction::triggered, this, &TreeViewSources::addFolder);
    connect(mActionRemoveFolder.get(), &QAction::triggered, this, &TreeViewSources::removeFolder);
    connect(mActionEditFolder.get(), &QAction::triggered, this, &TreeViewSources::editFolder);
    connect(ui->treeViewSources, &TreeViewSources::folderDoubleClicked, this, &TreeViewSources::editFolder);
    connect(mActionAddFeed.get(), &QAction::triggered, this, &TreeViewSources::addFeed);
    connect(mActionRefresh.get(), &QAction::triggered, this, &TreeViewSources::refreshViaContextMenu);
    connect(mActionToolbarRefresh.get(), &QAction::triggered, this, &TreeViewSources::refreshViaToolbarButton);
    connect(mActionRemoveFeed.get(), &QAction::triggered, this, &TreeViewSources::removeFeed);
    connect(mActionImportOPML.get(), &QAction::triggered, this, &TreeViewSources::importOPML);
    connect(mActionExportOPML.get(), &QAction::triggered, this, &TreeViewSources::exportOPML);
    connect(mActionViewProperties.get(), &QAction::triggered, [&]() { reloadPropertiesPane(); });

    connect(ui->widgetPropertiesPaneSource, &WidgetPropertiesPaneSource::sourceUpdated, [&]() { reload(); });

    connect(mActionReconnectToSource.get(), &QAction::triggered,
            [&]()
            {
                if (mMainWindow->currentContentPane() == ContentPane::Posts)
                {
                    auto index = currentIndex();
                    if (index.isValid())
                    {
                        auto sourceID = index.data(Role::ParentSourceID).toULongLong();

                        // clear out the error first, then do a simple (remote) request which might re-trigger the error, or keep it cleared
                        auto root = mItemModelSources->invisibleRootItem();
                        for (int32_t i = 0; i < root->rowCount(); ++i)
                        {
                            auto child = root->child(i);
                            if (child->data(Role::ID) == sourceID)
                            {
                                child->setData(QVariant(), Role::Error);
                                break;
                            }
                        }

                        ZapFR::Engine::Agent::getInstance()->queueGetSourceUnreadCount(
                            sourceID,
                            [&](uint64_t /*affectedSourceID*/, const std::unordered_map<uint64_t, uint64_t>& /*unreadCounts*/)
                            {
                                // nop
                            });
                    }
                }
            });

    connect(this, &QTreeView::expanded,
            [&](const QModelIndex& index)
            {
                if ((QGuiApplication::keyboardModifiers() & Qt::ControlModifier) == Qt::ControlModifier)
                {
                    expandRecursively(index);
                }
            });

    connect(this, &TreeViewSources::deletePressed,
            [&]()
            {
                if (mMainWindow->currentContentPane() == ContentPane::Posts)
                {
                    auto index = currentIndex();
                    if (index.isValid())
                    {
                        auto type = index.data(Role::Type).toULongLong();
                        switch (type)
                        {
                            case TreeViewSources::EntryType::Folder:
                            {
                                removeFolder();
                                break;
                            }
                            case TreeViewSources::EntryType::Feed:
                            {
                                removeFeed();
                                break;
                            }
                            case TreeViewSources::EntryType::Source:
                            {
                                removeSource();
                                break;
                            }
                        }
                    }
                }
            });

    connect(this, &TreeViewSources::customContextMenuRequested,
            [&](const QPoint& p)
            {
                if (mMainWindow->currentContentPane() == ContentPane::Posts)
                {
                    auto index = indexAt(p);
                    if (index.isValid())
                    {
                        auto parentSourceID = index.data(Role::ParentSourceID).toULongLong();
                        if (doesSourceHaveError(parentSourceID))
                        {
                            mContextMenuSourceError->popup(viewport()->mapToGlobal(p));
                        }
                        else
                        {
                            auto type = index.data(Role::Type).toULongLong();
                            switch (type)
                            {
                                case TreeViewSources::EntryType::Feed:
                                {
                                    mContextMenuFeed->popup(viewport()->mapToGlobal(p));
                                    break;
                                }
                                case TreeViewSources::EntryType::Folder:
                                {
                                    mContextMenuFolder->popup(viewport()->mapToGlobal(p));
                                    break;
                                }
                                case TreeViewSources::EntryType::Source:
                                {
                                    mContextMenuSource->popup(viewport()->mapToGlobal(p));
                                    break;
                                }
                            }
                        }
                    }
                }
            });

    connect(this, &TreeViewSources::currentSourceChanged,
            [&](const QModelIndex& index)
            {
                if (index.isValid())
                {
                    auto ui = mMainWindow->getUI();
                    switch (mMainWindow->currentContentPane())
                    {
                        case ContentPane::Posts:
                        {
                            ui->tableViewScriptFolders->setCurrentIndex(QModelIndex());
                            ui->tableViewPosts->setPage(1);
                            ui->tableViewPosts->reload();
                            ui->frameFlagFilters->reload();
                            ui->tableViewScriptFolders->reload();
                            ui->tableViewPosts->updateActivePostFilter();
                            break;
                        }
                        case ContentPane::Logs:
                        {
                            ui->tableViewLogs->setCurrentLogPage(1);
                            ui->tableViewLogs->reload();
                            break;
                        }
                        case ContentPane::Scripts:
                        {
                            ui->tableViewScripts->reload();
                            break;
                        }
                        case ContentPane::Properties:
                        {
                            reloadPropertiesPane();
                            break;
                        }
                    }
                }
            });

    mUpdateRemoteSourceBadgesTimer = std::make_unique<QTimer>(this);
    mUpdateRemoteSourceBadgesTimer->setInterval(30 * 1000);
    connect(mUpdateRemoteSourceBadgesTimer.get(), &QTimer::timeout,
            [&]()
            {
                uint64_t currentlySelectedSourceID{0};
                auto index = currentIndex();
                if (index.isValid())
                {
                    currentlySelectedSourceID = index.data(Role::ParentSourceID).toULongLong();
                }

                auto sources = ZapFR::Engine::Source::getSources(ZapFR::Engine::IdentifierRemoteServer);
                for (const auto& source : sources)
                {
                    ZapFR::Engine::Agent::getInstance()->queueGetSourceUnreadCount(
                        source->id(), [&](uint64_t affectedSourceID, const std::unordered_map<uint64_t, uint64_t>& unreadCounts)
                        { QMetaObject::invokeMethod(this, [=, this]() { remoteSourceUnreadCountsReceived(affectedSourceID, unreadCounts); }); });
                    if (source->id() == currentlySelectedSourceID)
                    {
                        mMainWindow->getUI()->tableViewScriptFolders->reload();
                    }
                }
            });
    mUpdateRemoteSourceBadgesTimer->start();
}

void ZapFR::Client::TreeViewSources::createContextMenus()
{
    auto ui = mMainWindow->getUI();

    mContextMenuSource = std::make_unique<QMenu>(nullptr);
    mContextMenuSource->addAction(mActionRefresh.get());
    mContextMenuSource->addAction(ui->tableViewPosts->actionMarkAsRead());
    mContextMenuSource->addSeparator();
    mContextMenuSource->addAction(mActionAddFeed.get());
    mContextMenuSource->addAction(mActionAddFolder.get());
    mContextMenuSource->addSeparator();
    mContextMenuSource->addAction(mActionRemoveSource.get());
    mContextMenuSource->addSeparator();
    mContextMenuSource->addAction(ui->tableViewLogs->actionViewLogs());
    mContextMenuSource->addAction(ui->treeViewSources->actionViewProperties());

    mContextMenuSourceError = std::make_unique<QMenu>(nullptr);
    mContextMenuSourceError->addAction(mActionReconnectToSource.get());
    mContextMenuSourceError->addSeparator();
    mContextMenuSourceError->addAction(mActionRemoveSource.get());
    mContextMenuSourceError->addSeparator();
    mContextMenuSourceError->addAction(ui->treeViewSources->actionViewProperties());

    mContextMenuFolder = std::make_unique<QMenu>(nullptr);
    mContextMenuFolder->addAction(mActionRefresh.get());
    mContextMenuFolder->addAction(ui->tableViewPosts->actionMarkAsRead());
    mContextMenuFolder->addSeparator();
    mContextMenuFolder->addAction(mActionAddFeed.get());
    mContextMenuFolder->addAction(mActionAddFolder.get());
    mContextMenuFolder->addAction(mActionEditFolder.get());
    mContextMenuFolder->addAction(mActionRemoveFolder.get());
    mContextMenuFolder->addSeparator();
    mContextMenuFolder->addAction(ui->tableViewLogs->actionViewLogs());
    mContextMenuFolder->addAction(ui->treeViewSources->actionViewProperties());

    mContextMenuFeed = std::make_unique<QMenu>(nullptr);
    mContextMenuFeed->addAction(mActionRefresh.get());
    mContextMenuFeed->addAction(ui->tableViewPosts->actionMarkAsRead());
    mContextMenuFeed->addSeparator();
    mContextMenuFeed->addAction(mActionRemoveFeed.get());
    mContextMenuFeed->addSeparator();
    mContextMenuFeed->addAction(ui->tableViewLogs->actionViewLogs());
    mContextMenuFeed->addAction(ui->treeViewSources->actionViewProperties());
}
