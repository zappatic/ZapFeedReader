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

#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "DialogAddFeed.h"
#include "DialogAddFolder.h"
#include "DialogEditScript.h"
#include "DialogImportOPML.h"
#include "DialogJumpToPage.h"
#include "FeedIconCache.h"
#include "ItemDelegateLog.h"
#include "ItemDelegatePost.h"
#include "ItemDelegateScript.h"
#include "ItemDelegateSource.h"
#include "StandardItemModelSources.h"
#include "Utilities.h"
#include "WebEnginePagePost.h"
#include "ZapFR/Agent.h"
#include "ZapFR/Database.h"
#include "ZapFR/FeedLocal.h"
#include "ZapFR/Flag.h"
#include "ZapFR/Folder.h"
#include "ZapFR/Log.h"
#include "ZapFR/Post.h"
#include "ZapFR/ScriptFolderLocal.h"
#include "ZapFR/ScriptLocal.h"

ZapFR::Client::MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ZapFR::Engine::ScriptLocal::setScriptDir(QDir::cleanPath(dataDir() + QDir::separator() + "scripts").toStdString());
    ZapFR::Engine::FeedLocal::setIconDir(QDir::cleanPath(dataDir() + QDir::separator() + "icons").toStdString());
    ZapFR::Engine::Database::setDatabasePath(QDir::cleanPath(dataDir() + QDir::separator() + "zapfeedreader-client.db").toStdString());
    mPostWebEnginePage = std::make_unique<WebEnginePagePost>(this);

    ui->setupUi(this);
    configureConnects();
    createContextMenus();
    configureIcons();
    restoreSettings();
    reloadSources();
    ui->treeViewSources->setItemDelegate(new ItemDelegateSource(ui->treeViewSources));
    ui->tableViewPosts->setItemDelegate(new ItemDelegatePost(ui->tableViewPosts));
    ui->tableViewLogs->setItemDelegate(new ItemDelegateLog(ui->tableViewLogs));
    ui->tableViewScripts->setItemDelegate(new ItemDelegateScript(ui->tableViewScripts));
    ui->webViewPost->setPage(mPostWebEnginePage.get());

    // prevent the left splitter from resizing while the window resizes
    ui->splitterLeft->setStretchFactor(1, 100);

    // add a spacer in the toolbar
    auto spacerWidget = new QWidget();
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto action = ui->toolBar->insertWidget(ui->action_View_logs, spacerWidget);
    action->setProperty("postPaneToolbarSpacer", true);

    ui->stackedWidgetRight->setCurrentIndex(StackedPanePosts);

    reloadCurrentPost();
}

ZapFR::Client::MainWindow::~MainWindow()
{
    delete ui;
}

void ZapFR::Client::MainWindow::closeEvent(QCloseEvent* /*event*/)
{
    saveSettings();
}

void ZapFR::Client::MainWindow::saveSettings() const
{
    QJsonObject root;
    root.insert(SETTING_MAINWINDOW_STATE, QString::fromUtf8(saveState().toBase64()));
    root.insert(SETTING_MAINWINDOW_GEOMETRY, QString::fromUtf8(saveGeometry().toBase64()));
    root.insert(SETTING_SPLITTERLEFT_STATE, QString::fromUtf8(ui->splitterLeft->saveState().toBase64()));
    root.insert(SETTING_SPLITTERLEFTINNER_STATE, QString::fromUtf8(ui->splitterLeftInner->saveState().toBase64()));
    root.insert(SETTING_SPLITTERRIGHT_STATE, QString::fromUtf8(ui->splitterRight->saveState().toBase64()));
    root.insert(SETTING_SOURCETREEVIEW_EXPANSION, expandedSourceTreeItems());

    auto sf = QFile(settingsFile());
    sf.open(QIODeviceBase::WriteOnly);
    sf.write(QJsonDocument(root).toJson());
    sf.close();
}

void ZapFR::Client::MainWindow::restoreSettings()
{
    try
    {
        auto sf = QFile(settingsFile());
        if (sf.exists())
        {
            sf.open(QIODeviceBase::ReadOnly);
            auto json = QJsonDocument::fromJson(sf.readAll());
            sf.close();
            if (json.isObject())
            {
                auto root = json.object();
                if (root.contains(SETTING_MAINWINDOW_STATE))
                {
                    restoreState(QByteArray::fromBase64(root.value(SETTING_MAINWINDOW_STATE).toVariant().toByteArray()));
                }
                if (root.contains(SETTING_MAINWINDOW_GEOMETRY))
                {
                    restoreGeometry(QByteArray::fromBase64(root.value(SETTING_MAINWINDOW_GEOMETRY).toVariant().toByteArray()));
                }
                if (root.contains(SETTING_SPLITTERLEFT_STATE))
                {
                    ui->splitterLeft->restoreState(QByteArray::fromBase64(root.value(SETTING_SPLITTERLEFT_STATE).toVariant().toByteArray()));
                }
                if (root.contains(SETTING_SPLITTERLEFTINNER_STATE))
                {
                    ui->splitterLeftInner->restoreState(QByteArray::fromBase64(root.value(SETTING_SPLITTERLEFTINNER_STATE).toVariant().toByteArray()));
                }
                if (root.contains(SETTING_SPLITTERRIGHT_STATE))
                {
                    ui->splitterRight->restoreState(QByteArray::fromBase64(root.value(SETTING_SPLITTERRIGHT_STATE).toVariant().toByteArray()));
                }
                if (root.contains(SETTING_SOURCETREEVIEW_EXPANSION))
                {
                    // instead of doing this immediately, write the array to mReloadSourcesExpansionSelectionState
                    // so that it will get picked up by reloadSources(), which happens after restoring the settings
                    mReloadSourcesExpansionSelectionState = std::make_unique<QJsonObject>();
                    mReloadSourcesExpansionSelectionState->insert("expanded", root.value(SETTING_SOURCETREEVIEW_EXPANSION).toArray());
                    mReloadSourcesExpansionSelectionState->insert("selectedSourceID", 0);
                    mReloadSourcesExpansionSelectionState->insert("selectedID", 0);
                    mReloadSourcesExpansionSelectionState->insert("selectedType", 0);
                }
            }
        }
    }
    catch (...)
    {
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
            auto index = mItemModelSources->indexFromItem(parent);
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
            auto idToMatch = parent->data(SourceTreeEntryIDRole).toULongLong();
            auto sourceIDToMatch = parent->data(SourceTreeEntryParentSourceIDRole).toULongLong();
            uint64_t folderIDToMatch = 0;
            auto typeToMatch = QString("");
            if (parent->data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_SOURCE)
            {
                typeToMatch = "source";
            }
            else if (parent->data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FOLDER)
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
                        auto index = mItemModelSources->indexFromItem(parent);
                        if (index.isValid())
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

void ZapFR::Client::MainWindow::addSource()
{
    std::cout << "tst\n";
}

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

void ZapFR::Client::MainWindow::addFolder()
{
    if (mDialogAddFolder == nullptr)
    {
        mDialogAddFolder = std::make_unique<DialogAddFolder>(this);
        connect(mDialogAddFolder.get(), &QDialog::finished,
                [&](int result)
                {
                    if (result == QDialog::DialogCode::Accepted)
                    {
                        ZapFR::Engine::Agent::getInstance()->queueAddFolder(mDialogAddFolder->selectedSourceID(), mDialogAddFolder->selectedFolderID(),
                                                                            mDialogAddFolder->title().toStdString(),
                                                                            [&]() { QMetaObject::invokeMethod(this, "folderAdded", Qt::AutoConnection); });
                    }
                });
    }

    auto [sourceID, folderID] = getCurrentlySelectedSourceAndFolderID();
    mDialogAddFolder->reset(sourceID, folderID);
    mDialogAddFolder->open();
}

void ZapFR::Client::MainWindow::importOPML()
{
    if (mDialogImportOPML == nullptr)
    {
        mDialogImportOPML = std::make_unique<DialogImportOPML>(this);
        connect(mDialogImportOPML.get(), &QDialog::finished,
                [&](int result)
                {
                    if (result == QDialog::DialogCode::Accepted)
                    {
                        for (const auto& feed : mDialogImportOPML->importedFeeds())
                        {
                            ZapFR::Engine::Agent::getInstance()->queueSubscribeFeed(mDialogImportOPML->selectedSourceID(), feed.url, mDialogImportOPML->selectedFolderID(),
                                                                                    feed.folderHierarchy,
                                                                                    [&]() { QMetaObject::invokeMethod(this, "feedAdded", Qt::AutoConnection); });
                        }
                    }
                });
    }

    auto [sourceID, folderID] = getCurrentlySelectedSourceAndFolderID();
    mDialogImportOPML->reset(sourceID, folderID);
    mDialogImportOPML->open();
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

void ZapFR::Client::MainWindow::reloadUsedFlagColors()
{
    auto index = ui->treeViewSources->currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
        if (sourceID != mPreviouslySelectedSourceID)
        {
            ZapFR::Engine::Agent::getInstance()->queueGetUsedFlagColors(sourceID,
                                                                        [&](uint64_t affectedSourceID, const std::unordered_set<ZapFR::Engine::FlagColor>& flagColors)
                                                                        {
                                                                            QMetaObject::invokeMethod(this, "populateUsedFlags", Qt::AutoConnection, affectedSourceID,
                                                                                                      flagColors);
                                                                            mPreviouslySelectedSourceID = affectedSourceID;
                                                                        });
        }
    }
}

void ZapFR::Client::MainWindow::populateUsedFlags(uint64_t /*sourceID*/, const std::unordered_set<ZapFR::Engine::FlagColor>& flagColors)
{
    ui->widgetFilterFlagBlue->setHidden(!flagColors.contains(ZapFR::Engine::FlagColor::Blue));
    ui->widgetFilterFlagGreen->setHidden(!flagColors.contains(ZapFR::Engine::FlagColor::Green));
    ui->widgetFilterFlagYellow->setHidden(!flagColors.contains(ZapFR::Engine::FlagColor::Yellow));
    ui->widgetFilterFlagOrange->setHidden(!flagColors.contains(ZapFR::Engine::FlagColor::Orange));
    ui->widgetFilterFlagRed->setHidden(!flagColors.contains(ZapFR::Engine::FlagColor::Red));
    ui->widgetFilterFlagPurple->setHidden(!flagColors.contains(ZapFR::Engine::FlagColor::Purple));

    if (!flagColors.contains(mFlagFilter))
    {
        mFlagFilter = ZapFR::Engine::FlagColor::Gray;
    }
}

void ZapFR::Client::MainWindow::reloadScriptFolders()
{
    // lambda for the callback, retrieving the script folders
    auto processScriptFolders = [&](uint64_t sourceID, const std::vector<ZapFR::Engine::ScriptFolder*>& scriptFolders)
    {
        QList<QList<QStandardItem*>> rows;
        for (const auto& scriptFolder : scriptFolders)
        {
            auto titleItem = new QStandardItem(QString::fromUtf8(scriptFolder->title()));
            titleItem->setData(QVariant::fromValue<uint64_t>(scriptFolder->id()), ScriptFolderIDRole);
            titleItem->setData(QVariant::fromValue<uint64_t>(sourceID), ScriptFolderSourceIDRole);

            QList<QStandardItem*> rowData;
            rowData << titleItem;
            rows << rowData;
        }
        QMetaObject::invokeMethod(this, "populateScriptFolders", Qt::AutoConnection, sourceID, rows);
    };

    auto index = ui->treeViewSources->currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
        if (sourceID != mPreviouslySelectedSourceID)
        {
            ZapFR::Engine::Agent::getInstance()->queueGetScriptFolders(sourceID, processScriptFolders);
        }
    }
}

void ZapFR::Client::MainWindow::populateScriptFolders(uint64_t sourceID, const QList<QList<QStandardItem*>>& scriptFolders)
{
    mPreviouslySelectedSourceID = sourceID;
    mItemModelScriptFolders = std::make_unique<QStandardItemModel>(this);
    ui->tableViewScriptFolders->setModel(mItemModelScriptFolders.get());
    auto headerItem = new QStandardItem(tr("Script folders"));
    headerItem->setTextAlignment(Qt::AlignLeft);
    mItemModelScriptFolders->setHorizontalHeaderItem(ScriptFolderColumnTitle, headerItem);
    for (const auto& scriptFolder : scriptFolders)
    {
        mItemModelScriptFolders->appendRow(scriptFolder);
    }
    ui->tableViewScriptFolders->horizontalHeader()->setSectionResizeMode(ScriptFolderColumnTitle, QHeaderView::Stretch);
}

void ZapFR::Client::MainWindow::reloadSources()
{
    // preserve the expansion of the source items and selected item data
    if (mReloadSourcesExpansionSelectionState == nullptr)
    {
        // we only do this if this state hasn't been set before (which it has after app launch, when the settings are loaded from disk)
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

    // recreate the model
    mItemModelSources = std::make_unique<StandardItemModelSources>(this, this);
    ui->treeViewSources->setModel(mItemModelSources.get());
    mItemModelSources->setHorizontalHeaderItem(0, new QStandardItem(tr("Sources & Feeds")));

    // get the trees of all the sources
    auto sources = ZapFR::Engine::Source::getSources({});
    for (const auto& source : sources)
    {
        ZapFR::Engine::Agent::getInstance()->queueGetSourceTree(
            source->id(),
            [&](uint64_t sourceID, const std::string& sourceTitle, const std::vector<ZapFR::Engine::Folder*>& rootFolders, const std::vector<ZapFR::Engine::Feed*>& feeds)
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
                    parentItem->appendRow(folderItem);

                    if (folder->hasSubfolders())
                    {
                        for (const auto& subfolder : folder->subfolders())
                        {
                            createFolderItems(subfolder, currentSourceID, folderItem);
                        }
                    }
                    folderIDToItemMapping[folder->id()] = folderItem;
                };
                // create the parent source item
                auto sourceItem = new QStandardItem(QString::fromUtf8(sourceTitle));
                sourceItem->setData(SOURCETREE_ENTRY_TYPE_SOURCE, SourceTreeEntryTypeRole);
                sourceItem->setData(QVariant::fromValue<uint64_t>(sourceID), SourceTreeEntryIDRole);
                sourceItem->setData(QVariant::fromValue<uint64_t>(sourceID), SourceTreeEntryParentSourceIDRole);

                // add all the folders and subfolders
                for (const auto& folder : rootFolders)
                {
                    createFolderItems(folder, sourceID, sourceItem);
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
                    feedItem->setData(QVariant::fromValue<uint64_t>(sourceID), SourceTreeEntryParentSourceIDRole);
                    feedItem->setData(QVariant::fromValue<uint64_t>(folderID), SourceTreeEntryParentFolderIDRole);
                    auto unreadCount = feed->unreadCount();
                    feedItem->setData(QVariant::fromValue<uint64_t>(unreadCount), SourceTreeEntryUnreadCount);
                    if (unreadCount >= 999)
                    {
                        feedItem->setToolTip(tr("%1 unread").arg(unreadCount));
                    }
                    feedItem->setData(true, SourceTreeEntryDisplayUnreadCountBadge);

                    if (!FeedIconCache::isCached(feed->id()) || !FeedIconCache::isSameHash(feed->id(), feed->iconHash()))
                    {
                        auto iconData = feed->icon();
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

                                FeedIconCache::cache(feed->id(), iconHash, icon);
                            }
                        }
                    }
                    parentItem->appendRow(feedItem);
                }
                QMetaObject::invokeMethod(this, "populateSources", Qt::AutoConnection, sourceID, sourceItem);
            });
    }
}

void ZapFR::Client::MainWindow::populateSources(uint64_t /*sourceID*/, QStandardItem* sourceItem)
{
    mItemModelSources->appendRow(sourceItem);

    // restore source item expansion and selection
    if (mReloadSourcesExpansionSelectionState != nullptr)
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
                    auto indexToSelect = mItemModelSources->indexFromItem(parent);
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
        mReloadSourcesExpansionSelectionState = nullptr;

        if (!ui->treeViewSources->currentIndex().isValid())
        {
            ui->treeViewSources->setCurrentIndex(mItemModelSources->indexFromItem(sourceItem));
        }
    }
}

void ZapFR::Client::MainWindow::updateFeedUnreadCountBadge(uint64_t sourceID, std::unordered_set<uint64_t> feedIDs, bool markEntireSourceAsRead, uint64_t unreadCount)
{
    // find the correct source
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
    if (sourceItem == nullptr)
    {
        return;
    }

    // recursively seek for the correct feed, and update its unread badge value
    std::function<void(QStandardItem*)> updateBadgeInSource;
    updateBadgeInSource = [&](QStandardItem* parent)
    {
        auto index = mItemModelSources->indexFromItem(parent);
        if (index.isValid())
        {
            auto type = index.data(SourceTreeEntryTypeRole).toULongLong();
            switch (type)
            {
                case SOURCETREE_ENTRY_TYPE_SOURCE:
                case SOURCETREE_ENTRY_TYPE_FOLDER:
                {
                    for (int32_t i = 0; i < parent->rowCount(); ++i)
                    {
                        updateBadgeInSource(parent->child(i, 0));
                    }

                    break;
                }
                case SOURCETREE_ENTRY_TYPE_FEED:
                {
                    auto id = index.data(SourceTreeEntryIDRole).toULongLong();
                    if (markEntireSourceAsRead || feedIDs.contains(id))
                    {
                        parent->setData(QVariant::fromValue<uint64_t>(unreadCount), SourceTreeEntryUnreadCount);
                    }
                    break;
                }
            }
        }
    };

    updateBadgeInSource(sourceItem);
}

QString ZapFR::Client::MainWindow::dataDir() const
{
    auto dataLocation = QStandardPaths::locate(QStandardPaths::StandardLocation::GenericDataLocation, "", QStandardPaths::LocateDirectory);

    auto dir = QDir(QDir::cleanPath(dataLocation + QDir::separator() + "ZapFeedReader"));
    if (!dir.exists())
    {
        dir.mkpath("ZapFeedReader");
    }
    return dir.path();
}

QString ZapFR::Client::MainWindow::configDir() const
{
    auto dataLocation = QStandardPaths::locate(QStandardPaths::StandardLocation::ConfigLocation, "", QStandardPaths::LocateDirectory);

    auto dir = QDir(QDir::cleanPath(dataLocation + QDir::separator() + "ZapFeedReader"));
    if (!dir.exists())
    {
        dir.mkpath("ZapFeedReader");
    }
    return dir.path();
}

QString ZapFR::Client::MainWindow::settingsFile() const
{
    return QDir::cleanPath(configDir() + QDir::separator() + "zapfeedreader-client.conf");
}

void ZapFR::Client::MainWindow::reloadPosts()
{
    // lambda to assign the correct role data to the table entries
    auto setItemData = [&](QStandardItem* item, ZapFR::Engine::Post* post, uint64_t sourceID)
    {
        item->setData(QVariant::fromValue<uint64_t>(post->id()), PostIDRole);
        item->setData(QVariant::fromValue<uint64_t>(sourceID), PostSourceIDRole);
        item->setData(QVariant::fromValue<uint64_t>(post->feedID()), PostFeedIDRole);
        item->setData(QVariant::fromValue<bool>(post->isRead()), PostIsReadRole);
        item->setData(QString::fromUtf8(post->link()), PostLinkRole);
    };

    // lambda for the callback, retrieving the posts
    auto processPosts = [&](uint64_t sourceID, const std::vector<ZapFR::Engine::Post*>& posts, uint64_t pageNumber, uint64_t totalPostCount)
    {
        QList<QList<QStandardItem*>> rows;
        for (const auto& post : posts)
        {
            auto unreadItem = new QStandardItem("");
            setItemData(unreadItem, post, sourceID);

            auto flagItem = new QStandardItem("");
            setItemData(flagItem, post, sourceID);
            QList<QVariant> flagColors;
            for (const auto& flag : post->flagColors())
            {
                flagColors << QVariant(static_cast<std::underlying_type_t<ZapFR::Engine::FlagColor>>(flag));
            }
            flagItem->setData(flagColors, PostAppliedFlagsRole);

            auto feedItem = new QStandardItem("");
            setItemData(feedItem, post, sourceID);
            feedItem->setData(QString::fromUtf8(post->feedTitle()), Qt::ToolTipRole);

            auto titleItem = new QStandardItem(QString::fromUtf8(post->title()));
            setItemData(titleItem, post, sourceID);

            auto datePublished = QString::fromUtf8(post->datePublished());
            auto dateItem = new QStandardItem(Utilities::prettyDate(datePublished));
            dateItem->setData(datePublished, PostISODateRole);
            setItemData(dateItem, post, sourceID);

            QList<QStandardItem*> rowData;
            rowData << unreadItem << flagItem << feedItem << titleItem << dateItem;
            rows << rowData;
        }

        QMetaObject::invokeMethod(this, "populatePosts", Qt::AutoConnection, rows, pageNumber, totalPostCount);
    };

    auto index = ui->tableViewScriptFolders->currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(ScriptFolderSourceIDRole).toULongLong();
        auto scriptFolderID = index.data(ScriptFolderIDRole).toULongLong();
        if (mFlagFilter == ZapFR::Engine::FlagColor::Gray)
        {
            ZapFR::Engine::Agent::getInstance()->queueGetScriptFolderPosts(sourceID, scriptFolderID, msPostsPerPage, mCurrentPostPage, mShowOnlyUnreadPosts, processPosts);
        }
        else
        {
            ZapFR::Engine::Agent::getInstance()->queueGetScriptFolderFlaggedPosts(mFlagFilter, sourceID, scriptFolderID, msPostsPerPage, mCurrentPostPage,
                                                                                  mShowOnlyUnreadPosts, processPosts);
        }
    }
    else
    {
        index = ui->treeViewSources->currentIndex();
        if (index.isValid())
        {
            if (index.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FEED)
            {
                auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
                auto feedID = index.data(SourceTreeEntryIDRole).toULongLong();
                if (mFlagFilter == ZapFR::Engine::FlagColor::Gray)
                {
                    ZapFR::Engine::Agent::getInstance()->queueGetFeedPosts(sourceID, feedID, msPostsPerPage, mCurrentPostPage, mShowOnlyUnreadPosts, processPosts);
                }
                else
                {
                    ZapFR::Engine::Agent::getInstance()->queueGetFeedFlaggedPosts(mFlagFilter, sourceID, feedID, msPostsPerPage, mCurrentPostPage, mShowOnlyUnreadPosts,
                                                                                  processPosts);
                }
            }
            else if (index.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FOLDER)
            {
                auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
                auto folderID = index.data(SourceTreeEntryIDRole).toULongLong();
                if (mFlagFilter == ZapFR::Engine::FlagColor::Gray)
                {
                    ZapFR::Engine::Agent::getInstance()->queueGetFolderPosts(sourceID, folderID, msPostsPerPage, mCurrentPostPage, mShowOnlyUnreadPosts, processPosts);
                }
                else
                {
                    ZapFR::Engine::Agent::getInstance()->queueGetFolderFlaggedPosts(mFlagFilter, sourceID, folderID, msPostsPerPage, mCurrentPostPage, mShowOnlyUnreadPosts,
                                                                                    processPosts);
                }
            }
            else if (index.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_SOURCE)
            {
                auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
                if (mFlagFilter == ZapFR::Engine::FlagColor::Gray)
                {
                    ZapFR::Engine::Agent::getInstance()->queueGetSourcePosts(sourceID, msPostsPerPage, mCurrentPostPage, mShowOnlyUnreadPosts, processPosts);
                }
                else
                {
                    ZapFR::Engine::Agent::getInstance()->queueGetSourceFlaggedPosts(mFlagFilter, sourceID, msPostsPerPage, mCurrentPostPage, mShowOnlyUnreadPosts,
                                                                                    processPosts);
                }
            }
            else
            {
                populatePosts();
            }
            updateToolbar();
        }
    }
}

void ZapFR::Client::MainWindow::populatePosts(const QList<QList<QStandardItem*>>& posts, uint64_t pageNumber, uint64_t totalPostCount)
{
    ui->stackedWidgetRight->setCurrentIndex(StackedPanePosts);

    int32_t columnWidthUnread = 50;
    int32_t columnWidthFlag = 40;
    int32_t columnWidthFeed = 40;
    int32_t columnWidthDate = 180;
    // restore the previous column widths
    if (mItemModelPosts != nullptr)
    {
        columnWidthUnread = ui->tableViewPosts->horizontalHeader()->sectionSize(PostColumnUnread);
        columnWidthFlag = ui->tableViewPosts->horizontalHeader()->sectionSize(PostColumnFlag);
        columnWidthFeed = ui->tableViewPosts->horizontalHeader()->sectionSize(PostColumnFeed);
        columnWidthDate = ui->tableViewPosts->horizontalHeader()->sectionSize(PostColumnDate);
    }

    mItemModelPosts = std::make_unique<QStandardItemModel>(this);
    ui->tableViewPosts->setModel(mItemModelPosts.get());
    mItemModelPosts->setHorizontalHeaderItem(PostColumnUnread, new QStandardItem(tr("Unread")));
    mItemModelPosts->setHorizontalHeaderItem(PostColumnFlag, new QStandardItem(tr("Flag")));
    mItemModelPosts->setHorizontalHeaderItem(PostColumnFeed, new QStandardItem(tr("Feed")));
    switch (mFlagFilter)
    {
        case ZapFR::Engine::FlagColor::Gray:
        {
            mItemModelPosts->setHorizontalHeaderItem(PostColumnTitle, new QStandardItem(tr("Title")));
            break;
        }
        case ZapFR::Engine::FlagColor::Blue:
        {
            mItemModelPosts->setHorizontalHeaderItem(PostColumnTitle, new QStandardItem(tr("Title (Blue flag filter active)")));
            break;
        }
        case ZapFR::Engine::FlagColor::Green:
        {
            mItemModelPosts->setHorizontalHeaderItem(PostColumnTitle, new QStandardItem(tr("Title (Green flag filter active)")));
            break;
        }
        case ZapFR::Engine::FlagColor::Yellow:
        {
            mItemModelPosts->setHorizontalHeaderItem(PostColumnTitle, new QStandardItem(tr("Title (Yellow flag filter active)")));
            break;
        }
        case ZapFR::Engine::FlagColor::Orange:
        {
            mItemModelPosts->setHorizontalHeaderItem(PostColumnTitle, new QStandardItem(tr("Title (Orange flag filter active)")));
            break;
        }
        case ZapFR::Engine::FlagColor::Red:
        {
            mItemModelPosts->setHorizontalHeaderItem(PostColumnTitle, new QStandardItem(tr("Title (Red flag filter active)")));
            break;
        }
        case ZapFR::Engine::FlagColor::Purple:
        {
            mItemModelPosts->setHorizontalHeaderItem(PostColumnTitle, new QStandardItem(tr("Title (Purple flag filter active)")));
            break;
        }
        default:
        {
            break;
        }
    }
    mItemModelPosts->setHorizontalHeaderItem(PostColumnDate, new QStandardItem(tr("Date")));
    for (const auto& post : posts)
    {
        mItemModelPosts->appendRow(post);
    }
    ui->tableViewPosts->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableViewPosts->horizontalHeader()->setSectionResizeMode(PostColumnTitle, QHeaderView::Stretch);
    ui->tableViewPosts->horizontalHeader()->resizeSection(PostColumnUnread, columnWidthUnread);
    ui->tableViewPosts->horizontalHeader()->resizeSection(PostColumnFlag, columnWidthFlag);
    ui->tableViewPosts->horizontalHeader()->resizeSection(PostColumnFeed, columnWidthFeed);
    ui->tableViewPosts->horizontalHeader()->resizeSection(PostColumnDate, columnWidthDate);
    postsTableViewSelectionChanged({});

    // in case we have just 1 feed selected, hide the feed column in the posts table
    // if we have a script folder selected, always show the feed column
    ui->tableViewPosts->setColumnHidden(PostColumnFeed, false);
    auto treeViewSourcesIndex = ui->treeViewSources->currentIndex();
    auto tableViewScriptFoldersIndex = ui->tableViewScriptFolders->currentIndex();
    if (!tableViewScriptFoldersIndex.isValid() && treeViewSourcesIndex.isValid() && treeViewSourcesIndex.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FEED)
    {
        ui->tableViewPosts->setColumnHidden(PostColumnFeed, true);
    }

    mCurrentPostCount = totalPostCount;
    mCurrentPostPage = pageNumber;
    mCurrentPostPageCount = 1;
    if (mCurrentPostCount > 0)
    {
        mCurrentPostPageCount = static_cast<uint64_t>(std::ceil(static_cast<float>(mCurrentPostCount) / static_cast<float>(msPostsPerPage)));
    }

    ui->pushButtonPostPageNumber->setText(QString("%1 %2 / %3").arg(tr("Page")).arg(mCurrentPostPage).arg(mCurrentPostPageCount));
    ui->labelTotalPostCount->setText(tr("%n post(s)", "", static_cast<int32_t>(mCurrentPostCount)));
}

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

void ZapFR::Client::MainWindow::postsTableViewSelectionChanged(const QModelIndexList& selected)
{
    mCurrentPostID = 0;
    mCurrentPostSourceID = 0;
    mCurrentPostFeedID = 0;

    if (selected.count() == 1)
    {
        auto index = selected.at(0);
        if (index.isValid())
        {
            mCurrentPostID = index.data(PostIDRole).toULongLong();
            mCurrentPostSourceID = index.data(PostSourceIDRole).toULongLong();
            mCurrentPostFeedID = index.data(PostFeedIDRole).toULongLong();

            auto isRead = index.data(PostIsReadRole).toBool();
            if (!isRead)
            {
                ZapFR::Engine::Agent::getInstance()->queueMarkPostRead(mCurrentPostSourceID, mCurrentPostFeedID, mCurrentPostID,
                                                                       [&](uint64_t sourceID, uint64_t feedID, uint64_t postID)
                                                                       { QMetaObject::invokeMethod(this, "postMarkedRead", Qt::AutoConnection, sourceID, feedID, postID); });
            }
            reloadCurrentPost();
        }
    }
    else if (selected.count() == 0)
    {
        setPostHTML(textMessageHTML(tr("No post selected")));
    }
    else
    {
        setPostHTML(textMessageHTML(tr("%1 posts selected").arg(selected.count())));
    }
}

void ZapFR::Client::MainWindow::reloadCurrentPost()
{
    if (mCurrentPostSourceID > 0 && mCurrentPostFeedID > 0 && mCurrentPostID > 0)
    {
        ZapFR::Engine::Agent::getInstance()->queueGetPost(mCurrentPostSourceID, mCurrentPostFeedID, mCurrentPostID,
                                                          [&](std::unique_ptr<ZapFR::Engine::Post> post)
                                                          {
                                                              QString htmlStr;
                                                              QTextStream html(&htmlStr, QIODeviceBase::ReadWrite);

                                                              auto postTitle = QString::fromUtf8(post->title());
                                                              auto postLink = QString::fromUtf8(post->link());

                                                              html << "<!DOCTYPE html>\n"
                                                                   << "<html>\n"
                                                                   << " <head>\n"
                                                                   << "     <style type='text/css'>\n"
                                                                   << postStyles() << "\n"
                                                                   << "     </style>\n"
                                                                   << " </head>\n"
                                                                   << " <body>\n";

                                                              if (postLink.isEmpty())
                                                              {
                                                                  html << R"(<h1 class="zapfr_title">)" << postTitle << "</h1>\n";
                                                              }
                                                              else
                                                              {
                                                                  html << R"(<a class="zapfr_title" href=")" << postLink << R"(">)" << postTitle << "</a>\n";
                                                              }

                                                              html << QString::fromUtf8(post->description()) << "\n"
                                                                   << " </body>\n"
                                                                   << "</html>";

                                                              QMetaObject::invokeMethod(this, "setPostHTML", Qt::AutoConnection, htmlStr);
                                                          });
    }
    else
    {
        setPostHTML(textMessageHTML(tr("No post selected")));
    }
}

void ZapFR::Client::MainWindow::setPostHTML(const QString& html)
{
    ui->webViewPost->setHtml(html);
}

QString ZapFR::Client::MainWindow::postStyles() const
{
    auto font = ui->treeViewSources->font();
    auto palette = QPalette(ui->treeViewSources->palette());

    QString overrideFilename;
    QString backgroundColor;
    QString textColor;
    QColor highlightColor = palette.color(QPalette::Active, QPalette::Highlight);

    auto currentColorScheme = QGuiApplication::styleHints()->colorScheme();
    if (currentColorScheme == Qt::ColorScheme::Dark)
    {
        overrideFilename = "posttheme.dark.css";
        backgroundColor = "#2a2a2a";
        textColor = "#fff";
    }
    else
    {
        overrideFilename = "posttheme.light.css";
        backgroundColor = "#fff";
        textColor = "#000";
    }

    auto override = QFile(QDir::cleanPath(configDir() + QDir::separator() + overrideFilename));
    if (override.exists())
    {
        override.open(QIODeviceBase::ReadOnly);
        auto styles = QString::fromUtf8(override.readAll());
        override.close();
        return styles;
    }

    return QString(R"(body { font-family: "%1", sans-serif; background-color: %2; color: %3; })"
                   "\n"
                   "a { color: %4; }\n"
                   ".zapfr_title { color: %3; font-size: 36px; font-weight: bold; text-decoration: none; display: block; margin: 25px 0; user-select:none; }\n")
        .arg(font.family())
        .arg(backgroundColor)
        .arg(textColor)
        .arg(highlightColor.name());
}

QString ZapFR::Client::MainWindow::textMessageHTML(const QString& message) const
{
    auto currentColorScheme = QGuiApplication::styleHints()->colorScheme();
    QString textColor = (currentColorScheme == Qt::ColorScheme::Dark ? "#444" : "#aaa");

    QString htmlStr;
    QTextStream html(&htmlStr, QIODeviceBase::ReadWrite);

    html << "<!DOCTYPE html>\n<html><head><style type='text/css'>\n" << postStyles();
    html << QString("h1 { color: %1; text-align: center; margin-top: 50px; }\n").arg(textColor);
    html << "</style></head><body>";
    html << "<h1>" << message << "</h1>";
    html << "</body></html>";

    return htmlStr;
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

void ZapFR::Client::MainWindow::folderMoved()
{
    reloadSources();
}

void ZapFR::Client::MainWindow::folderAdded()
{
    reloadSources();
}

void ZapFR::Client::MainWindow::folderRemoved()
{
    reloadSources();
    populatePosts();
}

void ZapFR::Client::MainWindow::postMarkedRead(uint64_t sourceID, uint64_t feedID, uint64_t postID)
{
    for (int32_t i = 0; i < mItemModelPosts->rowCount(); ++i)
    {
        auto index = mItemModelPosts->index(i, 0);
        if (index.data(PostIDRole).toULongLong() == postID)
        {
            for (int32_t col = 0; col < mItemModelPosts->columnCount(); ++col)
            {
                auto item = mItemModelPosts->item(i, col);
                item->setData(QVariant::fromValue<bool>(true), PostIsReadRole);
            }
            ui->tableViewPosts->scrollTo(index);
        }
    }

    ZapFR::Engine::Agent::getInstance()->queueGetFeedUnreadCount(sourceID, feedID,
                                                                 [&](uint64_t affectedSourceID, uint64_t affectedFeedID, uint64_t unreadCount)
                                                                 {
                                                                     std::unordered_set<uint64_t> feedIDs;
                                                                     feedIDs.insert(affectedFeedID);
                                                                     QMetaObject::invokeMethod(this, "updateFeedUnreadCountBadge", Qt::AutoConnection, affectedSourceID,
                                                                                               feedIDs, false, unreadCount);
                                                                 });
}

void ZapFR::Client::MainWindow::postsMarkedUnread(uint64_t sourceID, std::vector<std::tuple<uint64_t, uint64_t>> postIDs)
{
    std::unordered_set<uint64_t> uniqueFeedIDs{};

    for (const auto& [feedID, postID] : postIDs)
    {
        uniqueFeedIDs.insert(feedID);
        for (int32_t i = 0; i < mItemModelPosts->rowCount(); ++i)
        {
            auto index = mItemModelPosts->index(i, 0);
            if (index.data(PostIDRole).toULongLong() == postID)
            {
                for (int32_t col = 0; col < mItemModelPosts->columnCount(); ++col)
                {
                    auto item = mItemModelPosts->item(i, col);
                    item->setData(QVariant::fromValue<bool>(false), PostIsReadRole);
                }
            }
        }
    }

    for (const auto& feedID : uniqueFeedIDs)
    {
        ZapFR::Engine::Agent::getInstance()->queueGetFeedUnreadCount(sourceID, feedID,
                                                                     [&](uint64_t affectedSourceID, uint64_t affectedFeedID, uint64_t unreadCount)
                                                                     {
                                                                         std::unordered_set<uint64_t> feedIDs;
                                                                         feedIDs.insert(affectedFeedID);
                                                                         QMetaObject::invokeMethod(this, "updateFeedUnreadCountBadge", Qt::AutoConnection, affectedSourceID,
                                                                                                   feedIDs, false, unreadCount);
                                                                     });
    }
}

void ZapFR::Client::MainWindow::postMarkedFlagged(uint64_t /*sourceID*/, uint64_t /*feedID*/, uint64_t /*postID*/, ZapFR::Engine::FlagColor /*flagColor*/)
{
    mPreviouslySelectedSourceID = 0;
    reloadUsedFlagColors();
}

void ZapFR::Client::MainWindow::postMarkedUnflagged(uint64_t /*sourceID*/, uint64_t /*feedID*/, uint64_t /*postID*/,
                                                    const std::unordered_set<ZapFR::Engine::FlagColor>& /*flagColors*/)
{
    mPreviouslySelectedSourceID = 0;
    reloadUsedFlagColors();
}

void ZapFR::Client::MainWindow::feedMarkedRead(uint64_t sourceID, uint64_t feedID)
{
    updateFeedUnreadCountBadge(sourceID, {feedID}, false, 0);
    mCurrentPostPage = 1;
    reloadPosts();
}

void ZapFR::Client::MainWindow::folderMarkedRead(uint64_t sourceID, std::unordered_set<uint64_t> feedIDs)
{
    updateFeedUnreadCountBadge(sourceID, feedIDs, false, 0);
    mCurrentPostPage = 1;
    reloadPosts();
}

void ZapFR::Client::MainWindow::sourceMarkedRead(uint64_t sourceID)
{
    updateFeedUnreadCountBadge(sourceID, {}, true, 0);
    mCurrentPostPage = 1;
    reloadPosts();
}

void ZapFR::Client::MainWindow::configureIcons()
{
    // the defaults are for the light theme
    auto color = QString("#000");
    auto colorDisabled = QString("#aaa");
    auto currentColorScheme = QGuiApplication::styleHints()->colorScheme();
    if (currentColorScheme == Qt::ColorScheme::Dark)
    {
        color = "#fff";
        colorDisabled = "#555";
    }

    const auto configureIcon = [&](const QString& svgResource)
    {
        auto svgFile = QFile(svgResource);
        svgFile.open(QIODeviceBase::ReadOnly);
        auto svgContents = QString(svgFile.readAll());
        svgFile.close();

        QIcon icon;

        // Normal On
        QImage normalOn;
        auto svg = svgContents;
        svg.replace("{#color}", color);
        normalOn.loadFromData(svg.toUtf8());
        icon.addPixmap(QPixmap::fromImage(normalOn), QIcon::Normal, QIcon::On);

        // Disabled On
        QImage disabledOn;
        svg = svgContents;
        svg.replace("{#color}", colorDisabled);
        disabledOn.loadFromData(svg.toUtf8());
        icon.addPixmap(QPixmap::fromImage(disabledOn), QIcon::Disabled, QIcon::On);
        return icon;
    };

    ui->action_Refresh_feeds->setIcon(configureIcon(":/refreshFeed.svg"));
    ui->action_Mark_as_read->setIcon(configureIcon(":/markAsRead.svg"));
    ui->action_Add_feed->setIcon(configureIcon(":/addFeed.svg"));
    ui->action_Add_folder->setIcon(configureIcon(":/addFolder.svg"));
    ui->action_View_logs->setIcon(configureIcon(":/viewLogs.svg"));
    ui->action_View_scripts->setIcon(configureIcon(":/script.svg"));
    ui->action_Back_to_posts->setIcon(configureIcon(":/back.svg"));
    ui->action_Edit_script->setIcon(configureIcon(":/edit.svg"));
    ui->action_Remove_script->setIcon(configureIcon(":/remove.svg"));
    ui->action_Add_script->setIcon(configureIcon(":/addFeed.svg"));
    ui->pushButtonPostPreviousPage->setIcon(configureIcon(":/previousPage.svg"));
    ui->pushButtonPostFirstPage->setIcon(configureIcon(":/firstPage.svg"));
    ui->pushButtonPostNextPage->setIcon(configureIcon(":/nextPage.svg"));
    ui->pushButtonPostLastPage->setIcon(configureIcon(":/lastPage.svg"));
    ui->pushButtonLogPreviousPage->setIcon(configureIcon(":/previousPage.svg"));
    ui->pushButtonLogFirstPage->setIcon(configureIcon(":/firstPage.svg"));
    ui->pushButtonLogNextPage->setIcon(configureIcon(":/nextPage.svg"));
    ui->pushButtonLogLastPage->setIcon(configureIcon(":/lastPage.svg"));

    auto labelFont = ui->labelTotalPostCount->font();
    labelFont.setPointSizeF(10.0f);
    ui->pushButtonPostPageNumber->setFont(labelFont);
    ui->pushButtonLogPageNumber->setFont(labelFont);
    ui->labelTotalPostCount->setFont(labelFont);
    ui->pushButtonToggleShowUnread->setFont(labelFont);

    ui->toolBar->setStyleSheet(QString("QToolBar { border-bottom-style: none; }\n"
                                       "QToolButton:disabled { color:%1; }\n")
                                   .arg(colorDisabled));

    auto palette = ui->frameFlagFilters->palette();
    ui->frameFlagFilters->setStyleSheet(QString("QFrame { border-top: 0px; border-left: 0px; border-right: 1px solid %1; border-bottom: 1px solid %1;}")
                                            .arg(palette.color(QPalette::Active, QPalette::Dark).name()));
}

void ZapFR::Client::MainWindow::updateToolbar()
{
    // hide all actions
    for (const auto& action : ui->toolBar->actions())
    {
        action->setVisible(false);
    }

    switch (ui->stackedWidgetRight->currentIndex())
    {
        case StackedPanePosts:
        {
            bool anythingSelected{false};
            QString markAsReadCaption;
            QString refreshFeedsCaption;

            auto index = ui->treeViewSources->currentIndex();
            if (index.isValid())
            {
                anythingSelected = true;

                auto type = index.data(SourceTreeEntryTypeRole).toULongLong();
                switch (type)
                {
                    case SOURCETREE_ENTRY_TYPE_FEED:
                    {
                        markAsReadCaption = tr("Mark feed as read");
                        refreshFeedsCaption = tr("Refresh feed");
                        break;
                    }
                    case SOURCETREE_ENTRY_TYPE_FOLDER:
                    {
                        markAsReadCaption = tr("Mark folder as read");
                        refreshFeedsCaption = tr("Refresh folder");
                        break;
                    }
                    case SOURCETREE_ENTRY_TYPE_SOURCE:
                    {
                        markAsReadCaption = tr("Mark source as read");
                        refreshFeedsCaption = tr("Refresh source");
                        break;
                    }
                }
            }

            ui->action_Add_feed->setVisible(true);
            ui->action_Add_folder->setVisible(true);
            ui->action_Refresh_feeds->setVisible(true);
            ui->action_Mark_as_read->setVisible(true);
            ui->action_View_logs->setVisible(true);
            ui->action_View_scripts->setVisible(true);

            ui->action_Add_feed->setEnabled(anythingSelected);
            ui->action_Add_folder->setEnabled(anythingSelected);
            ui->action_Mark_as_read->setEnabled(anythingSelected);
            ui->action_Mark_as_read->setText(markAsReadCaption);
            ui->action_View_logs->setEnabled(anythingSelected);
            ui->action_Refresh_feeds->setEnabled(anythingSelected);
            ui->action_Refresh_feeds->setText(refreshFeedsCaption);

            for (const auto& action : ui->toolBar->actions())
            {
                if (action->property("postPaneToolbarSpacer").isValid())
                {
                    action->setVisible(true);
                    break;
                }
            }
            break;
        }
        case StackedPaneLogs:
        {
            ui->action_Back_to_posts->setVisible(true);
            break;
        }
        case StackedPaneScripts:
        {
            ui->action_Back_to_posts->setVisible(true);
            ui->action_Edit_script->setVisible(true);
            ui->action_Remove_script->setVisible(true);
            ui->action_Add_script->setVisible(true);
            break;
        }
    }
}

void ZapFR::Client::MainWindow::markAsRead()
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
                ZapFR::Engine::Agent::getInstance()->queueMarkFeedRead(
                    sourceID, feedID,
                    [&](uint64_t affectedSourceID, uint64_t affectedFeedID)
                    { QMetaObject::invokeMethod(this, "feedMarkedRead", Qt::AutoConnection, affectedSourceID, affectedFeedID); });
                break;
            }
            case SOURCETREE_ENTRY_TYPE_FOLDER:
            {
                auto folderID = index.data(SourceTreeEntryIDRole).toULongLong();
                ZapFR::Engine::Agent::getInstance()->queueMarkFolderRead(
                    sourceID, folderID,
                    [&](uint64_t affectedSourceID, std::unordered_set<uint64_t> feedIDs)
                    { QMetaObject::invokeMethod(this, "folderMarkedRead", Qt::AutoConnection, affectedSourceID, feedIDs); });
                break;
            }
            case SOURCETREE_ENTRY_TYPE_SOURCE:
            {
                ZapFR::Engine::Agent::getInstance()->queueMarkSourceRead(sourceID, [&](uint64_t affectedSourceID)
                                                                         { QMetaObject::invokeMethod(this, "sourceMarkedRead", Qt::AutoConnection, affectedSourceID); });
                break;
            }
        }
    }
}

void ZapFR::Client::MainWindow::markAsUnread()
{
    auto selectionModel = ui->tableViewPosts->selectionModel();
    if (selectionModel != nullptr)
    {
        uint64_t sourceID{0};
        std::vector<std::tuple<uint64_t, uint64_t>> feedAndPostIDs;
        auto selectedIndexes = selectionModel->selectedIndexes();
        for (const auto& index : selectedIndexes)
        {
            if (index.column() == PostColumnUnread)
            {
                sourceID = index.data(PostSourceIDRole).toULongLong();
                auto feedID = index.data(PostFeedIDRole).toULongLong();
                auto postID = index.data(PostIDRole).toULongLong();

                feedAndPostIDs.emplace_back(std::make_tuple(feedID, postID));
            }
        }

        if (feedAndPostIDs.size() > 0)
        {
            ZapFR::Engine::Agent::getInstance()->queueMarkPostsUnread(sourceID, feedAndPostIDs,
                                                                      [&](uint64_t affectedSourceID, std::vector<std::tuple<uint64_t, uint64_t>> postIDs) {
                                                                          QMetaObject::invokeMethod(this, "postsMarkedUnread", Qt::AutoConnection, affectedSourceID, postIDs);
                                                                      });
        }
    }
}

void ZapFR::Client::MainWindow::removeFolder()
{
    QMessageBox messageBox;
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
        auto index = ui->treeViewSources->currentIndex();
        if (index.isValid())
        {
            auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
            auto folder = index.data(SourceTreeEntryIDRole).toULongLong();
            ZapFR::Engine::Agent::getInstance()->queueRemoveFolder(sourceID, folder, [&]() { QMetaObject::invokeMethod(this, "folderRemoved", Qt::AutoConnection); });
        }
    }
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

void ZapFR::Client::MainWindow::showJumpToPageDialog(uint64_t currentPage, uint64_t pageCount, std::function<void(uint64_t)> callback)
{
    if (mDialogJumpToPage == nullptr)
    {
        mDialogJumpToPage = std::make_unique<DialogJumpToPage>(this);
        connect(mDialogJumpToPage.get(), &QDialog::finished,
                [&](int result)
                {
                    if (result == QDialog::DialogCode::Accepted)
                    {
                        mDialogJumpToPage->callback()(mDialogJumpToPage->pageToJumpTo());
                    }
                });
    }
    mDialogJumpToPage->reset(currentPage, pageCount, callback);
    mDialogJumpToPage->open();
}

void ZapFR::Client::MainWindow::createContextMenus()
{
    // SOURCE
    mSourceContextMenuSource = std::make_unique<QMenu>(nullptr);
    mSourceContextMenuSource->addAction(ui->action_Refresh_feeds);
    mSourceContextMenuSource->addAction(ui->action_Mark_as_read);
    mSourceContextMenuSource->addSeparator();
    mSourceContextMenuSource->addAction(ui->action_Add_feed);
    mSourceContextMenuSource->addAction(ui->action_Add_folder);
    mSourceContextMenuSource->addAction(ui->action_View_logs);

    // FOLDER
    mSourceContextMenuFolder = std::make_unique<QMenu>(nullptr);
    mSourceContextMenuFolder->addAction(ui->action_Refresh_feeds);
    mSourceContextMenuFolder->addAction(ui->action_Mark_as_read);
    mSourceContextMenuFolder->addSeparator();
    mSourceContextMenuFolder->addAction(ui->action_Add_feed);
    mSourceContextMenuFolder->addAction(ui->action_Add_folder);
    mSourceContextMenuFolder->addAction(ui->action_View_logs);
    mSourceContextMenuFolder->addSeparator();
    mSourceContextMenuFolder->addAction(ui->action_Remove_folder);

    // FEED
    mSourceContextMenuFeed = std::make_unique<QMenu>(nullptr);
    mSourceContextMenuFeed->addAction(ui->action_Refresh_feeds);
    mSourceContextMenuFeed->addAction(ui->action_Mark_as_read);
    mSourceContextMenuFeed->addAction(ui->action_View_logs);
    mSourceContextMenuFeed->addSeparator();
    mSourceContextMenuFeed->addAction(ui->action_Remove_feed);

    // POST
    mPostContextMenu = std::make_unique<QMenu>(nullptr);
    mPostContextMenu->addAction(ui->action_Mark_as_unread);

    // SCRIPT
    mScriptContextMenu = std::make_unique<QMenu>(nullptr);
    mScriptContextMenu->addAction(ui->action_Add_script);
    mScriptContextMenu->addAction(ui->action_Edit_script);
    mScriptContextMenu->addSeparator();
    mScriptContextMenu->addAction(ui->action_Remove_script);
}

void ZapFR::Client::MainWindow::configureConnects()
{
    connect(ui->action_Add_source, &QAction::triggered, this, &MainWindow::addSource);
    connect(ui->action_Add_feed, &QAction::triggered, this, &MainWindow::addFeed);
    connect(ui->action_Add_folder, &QAction::triggered, this, &MainWindow::addFolder);
    connect(ui->action_Import_OPML, &QAction::triggered, this, &MainWindow::importOPML);
    connect(ui->action_Mark_as_read, &QAction::triggered, this, &MainWindow::markAsRead);
    connect(ui->action_Refresh_feeds, &QAction::triggered, this, &MainWindow::refreshFeeds);
    connect(ui->action_Mark_as_unread, &QAction::triggered, this, &MainWindow::markAsUnread);
    connect(ui->action_Remove_folder, &QAction::triggered, this, &MainWindow::removeFolder);
    connect(ui->action_Remove_feed, &QAction::triggered, this, &MainWindow::removeFeed);

    connect(ui->action_View_logs, &QAction::triggered,
            [&]()
            {
                mCurrentLogPage = 1;
                reloadLogs();
            });

    connect(ui->action_View_scripts, &QAction::triggered,
            [&]()
            {
                mPreviouslySelectedSourceID = 0;
                reloadScripts();
            });

    connect(ui->action_Back_to_posts, &QAction::triggered, [&]() { ui->stackedWidgetRight->setCurrentIndex(StackedPanePosts); });

    connect(ui->treeViewSources, &TreeViewSources::currentSourceChanged,
            [&](const QModelIndex& index)
            {
                if (index.isValid())
                {
                    switch (ui->stackedWidgetRight->currentIndex())
                    {
                        case StackedPanePosts:
                        {
                            ui->tableViewScriptFolders->setCurrentIndex(QModelIndex());
                            mCurrentPostPage = 1;
                            reloadPosts();
                            reloadUsedFlagColors();
                            reloadScriptFolders();
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
                    }
                }
            });

    connect(ui->tableViewPosts, &TableViewPosts::selectedPostsChanged, this, &MainWindow::postsTableViewSelectionChanged);

    connect(ui->tableViewScriptFolders, &TableViewScriptFolders::selectedScriptFolderChanged,
            [&](const QModelIndex& index)
            {
                if (index.isValid())
                {
                    reloadPosts();
                }
            });

    connect(ui->treeViewSources, &TreeViewSources::customContextMenuRequested,
            [&](const QPoint& p)
            {
                if (ui->stackedWidgetRight->currentIndex() == StackedPanePosts)
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

    connect(ui->tableViewPosts, &TableViewPosts::customContextMenuRequested,
            [&](const QPoint& p) { mPostContextMenu->popup(ui->tableViewPosts->viewport()->mapToGlobal(p)); });

    connect(ui->pushButtonPostPreviousPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentPostPage = std::max(1ul, mCurrentPostPage - 1);
                reloadPosts();
            });

    connect(ui->pushButtonPostNextPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentPostPage = std::min(mCurrentPostPageCount, mCurrentPostPage + 1);
                reloadPosts();
            });

    connect(ui->pushButtonPostFirstPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentPostPage = 1;
                reloadPosts();
            });

    connect(ui->pushButtonPostLastPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentPostPage = mCurrentPostPageCount;
                reloadPosts();
            });

    connect(ui->pushButtonPostPageNumber, &QPushButton::clicked,
            [&]()
            {
                showJumpToPageDialog(mCurrentPostPage, mCurrentPostPageCount,
                                     [&](uint64_t page)
                                     {
                                         mCurrentPostPage = page;
                                         reloadPosts();
                                     });
            });

    connect(ui->pushButtonToggleShowUnread, &QPushButton::clicked,
            [&]()
            {
                mShowOnlyUnreadPosts = !mShowOnlyUnreadPosts;
                if (mShowOnlyUnreadPosts)
                {
                    ui->pushButtonToggleShowUnread->setText(tr("Show all posts"));
                }
                else
                {
                    ui->pushButtonToggleShowUnread->setText(tr("Show only unread posts"));
                }
                reloadPosts();
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

    connect(ui->stackedWidgetRight, &QStackedWidget::currentChanged,
            [&]()
            {
                updateToolbar();
                switch (ui->stackedWidgetRight->currentIndex())
                {
                    case StackedPanePosts:
                    {
                        ui->frameFlagFilters->setVisible(true);
                        ui->tableViewScriptFolders->setVisible(true);
                        setUnreadBadgesShown(true);
                        mCurrentPostPage = 1;
                        mPreviouslySelectedSourceID = 0;
                        reloadUsedFlagColors();
                        reloadPosts();
                        break;
                    }
                    case StackedPaneLogs:
                    {
                        ui->frameFlagFilters->setVisible(false);
                        ui->tableViewScriptFolders->setVisible(false);
                        setUnreadBadgesShown(false);
                        break;
                    }
                    case StackedPaneScripts:
                    {
                        ui->frameFlagFilters->setVisible(false);
                        ui->tableViewScriptFolders->setVisible(false);
                        setUnreadBadgesShown(false);
                        break;
                    }
                }
            });

    connect(mPostWebEnginePage.get(), &QWebEnginePage::linkHovered,
            [&](const QString& url)
            {
                if (!url.isEmpty())
                {
                    ui->statusbar->showMessage(url);
                }
                else
                {
                    ui->statusbar->clearMessage();
                }
            });

    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
            [&](Qt::ColorScheme /*scheme*/)
            {
                reloadCurrentPost();
                configureIcons();
            });

    connect(ui->tableViewPosts, &TableViewPosts::postMarkedFlagged,
            [&](uint64_t sourceID, uint64_t feedID, uint64_t postID, ZapFR::Engine::FlagColor flagColor)
            {
                ZapFR::Engine::Agent::getInstance()->queueMarkPostFlagged(
                    sourceID, feedID, postID, flagColor,
                    [&](uint64_t affectedSourceID, uint64_t affectedFeedID, uint64_t affectedPostID, ZapFR::Engine::FlagColor affectedFlagColor)
                    { QMetaObject::invokeMethod(this, "postMarkedFlagged", Qt::AutoConnection, affectedSourceID, affectedFeedID, affectedPostID, affectedFlagColor); });
            });

    connect(ui->tableViewPosts, &TableViewPosts::postMarkedUnflagged,
            [&](uint64_t sourceID, uint64_t feedID, uint64_t postID, ZapFR::Engine::FlagColor flagColor)
            {
                ZapFR::Engine::Agent::getInstance()->queueMarkPostUnflagged(
                    sourceID, feedID, postID, {flagColor},
                    [&](uint64_t affectedSourceID, uint64_t affectedFeedID, uint64_t affectedPostID, const std::unordered_set<ZapFR::Engine::FlagColor>& affectedFlagColor)
                    { QMetaObject::invokeMethod(this, "postMarkedUnflagged", Qt::AutoConnection, affectedSourceID, affectedFeedID, affectedPostID, affectedFlagColor); });
            });

    connect(ui->tableViewPosts, &TableViewPosts::clearAllFlagsRequested,
            [&](uint64_t sourceID, uint64_t feedID, uint64_t postID)
            {
                ZapFR::Engine::Agent::getInstance()->queueMarkPostUnflagged(
                    sourceID, feedID, postID, ZapFR::Engine::Flag::allFlagColors(),
                    [&](uint64_t affectedSourceID, uint64_t affectedFeedID, uint64_t affectedPostID, const std::unordered_set<ZapFR::Engine::FlagColor>& affectedFlagColor)
                    { QMetaObject::invokeMethod(this, "postMarkedUnflagged", Qt::AutoConnection, affectedSourceID, affectedFeedID, affectedPostID, affectedFlagColor); });
            });

    ui->widgetFilterFlagBlue->setFlagColor(ZapFR::Engine::FlagColor::Blue);
    ui->widgetFilterFlagGreen->setFlagColor(ZapFR::Engine::FlagColor::Green);
    ui->widgetFilterFlagYellow->setFlagColor(ZapFR::Engine::FlagColor::Yellow);
    ui->widgetFilterFlagOrange->setFlagColor(ZapFR::Engine::FlagColor::Orange);
    ui->widgetFilterFlagRed->setFlagColor(ZapFR::Engine::FlagColor::Red);
    ui->widgetFilterFlagPurple->setFlagColor(ZapFR::Engine::FlagColor::Purple);

    static std::vector<PopupFlag*> flags{ui->widgetFilterFlagBlue,   ui->widgetFilterFlagGreen, ui->widgetFilterFlagYellow,
                                         ui->widgetFilterFlagOrange, ui->widgetFilterFlagRed,   ui->widgetFilterFlagPurple};
    for (const auto& flag : flags)
    {
        flag->setFlagStyle(Utilities::FlagStyle::Unfilled);
        connect(flag, &PopupFlag::flagClicked,
                [&](PopupFlag* clickedFlag)
                {
                    for (const auto& f : flags)
                    {
                        if (f != clickedFlag)
                        {
                            f->setFlagStyle(Utilities::FlagStyle::Unfilled);
                        }
                    }

                    switch (clickedFlag->flagStyle())
                    {
                        case Utilities::FlagStyle::Filled:
                        {
                            clickedFlag->setFlagStyle(Utilities::FlagStyle::Unfilled);
                            mFlagFilter = ZapFR::Engine::FlagColor::Gray;
                            break;
                        }
                        case Utilities::FlagStyle::Unfilled:
                        {
                            clickedFlag->setFlagStyle(Utilities::FlagStyle::Filled);
                            mFlagFilter = clickedFlag->flagColor();
                            break;
                        }
                    }
                    mCurrentPostPage = 1;
                    reloadPosts();
                });
    }

    connectScriptStuff();
}
