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
#include "Agent.h"
#include "FeedIconCache.h"
#include "FeedLocal.h"
#include "Folder.h"
#include "ItemDelegatePost.h"
#include "ItemDelegateSource.h"
#include "StandardItemModelSources.h"
#include "Utilities.h"
#include "WebEnginePagePost.h"

using namespace std::placeholders;

ZapFR::Client::MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ZapFR::Engine::FeedLocal::setIconDir(QDir::cleanPath(dataDir() + QDir::separator() + "icons").toStdString());
    mDatabase = std::make_unique<ZapFR::Engine::Database>(QDir::cleanPath(dataDir() + QDir::separator() + "zapfeedreader-client.db").toStdString());
    ZapFR::Engine::Source::registerDatabaseInstance(mDatabase.get());
    ZapFR::Engine::Feed::registerDatabaseInstance(mDatabase.get());
    ZapFR::Engine::Post::registerDatabaseInstance(mDatabase.get());
    ZapFR::Engine::Folder::registerDatabaseInstance(mDatabase.get());

    ui->setupUi(this);
    connect(ui->action_Add_source, &QAction::triggered, this, &MainWindow::addSource);
    connect(ui->action_Add_feed, &QAction::triggered, this, &MainWindow::addFeed);
    connect(ui->action_Add_folder, &QAction::triggered, this, &MainWindow::addFolder);
    connect(ui->action_Import_OPML, &QAction::triggered, this, &MainWindow::importOPML);
    connect(ui->action_Mark_feed_as_read, &QAction::triggered, this, &MainWindow::markFeedAsRead);
    connect(ui->action_Refresh_all_feeds, &QAction::triggered, this, &MainWindow::refreshAllFeeds);
    connect(ui->treeViewSources, &TreeViewSources::customContextMenuRequested, this, &MainWindow::sourceTreeViewContextMenuRequested);
    connect(ui->treeViewSources, &TreeViewSources::currentSourceChanged, this, &MainWindow::sourceTreeViewItemSelected);
    connect(ui->tableViewPosts, &TableViewPosts::selectedPostsChanged, this, &MainWindow::postsTableViewSelectionChanged);
    connect(ui->tableViewPosts, &TableViewPosts::customContextMenuRequested, this, &MainWindow::postsTableViewContextMenuRequested);
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, &MainWindow::colorSchemeChanged);

    fixPalette();
    setupToolbarIcons();
    setupToolbarEnabledStates();
    reloadSources();
    ui->treeViewSources->setItemDelegate(new ItemDelegateSource(ui->treeViewSources));
    ui->tableViewPosts->setItemDelegate(new ItemDelegatePost(ui->tableViewPosts));

    mPostWebEnginePage = std::make_unique<WebEnginePagePost>(this);
    ui->webViewPost->setPage(mPostWebEnginePage.get());
    connect(mPostWebEnginePage.get(), &QWebEnginePage::linkHovered, this, &MainWindow::postLinkHovered);

    restoreSettings();
    reloadCurrentPost();
    createContextMenus();
}

ZapFR::Client::MainWindow::~MainWindow()
{
    delete ui;
}

void ZapFR::Client::MainWindow::closeEvent(QCloseEvent* /*event*/)
{
    saveSettings();
}

void ZapFR::Client::MainWindow::colorSchemeChanged(Qt::ColorScheme /*scheme*/)
{
    reloadCurrentPost();
    setupToolbarIcons();
}

void ZapFR::Client::MainWindow::saveSettings() const
{
    QJsonObject root;
    root.insert(SETTING_MAINWINDOW_STATE, QString::fromUtf8(saveState().toBase64()));
    root.insert(SETTING_MAINWINDOW_GEOMETRY, QString::fromUtf8(saveGeometry().toBase64()));
    root.insert(SETTING_SPLITTERLEFT_STATE, QString::fromUtf8(ui->splitterLeft->saveState().toBase64()));
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
                if (root.contains(SETTING_SPLITTERRIGHT_STATE))
                {
                    ui->splitterRight->restoreState(QByteArray::fromBase64(root.value(SETTING_SPLITTERRIGHT_STATE).toVariant().toByteArray()));
                }
                if (root.contains(SETTING_SOURCETREEVIEW_EXPANSION))
                {
                    expandSourceTreeItems(root.value(SETTING_SOURCETREEVIEW_EXPANSION).toArray());
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
    auto selectionModel = ui->treeViewSources->selectionModel();
    if (selectionModel != nullptr)
    {
        auto selectedIndexes = selectionModel->selectedIndexes();
        if (selectedIndexes.count() == 1)
        {
            auto currentIndex = selectedIndexes.at(0);
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
        }
    }
    return std::make_tuple(sourceID, folderID);
}

void ZapFR::Client::MainWindow::reloadSources(bool performClickOnSelection)
{
    // preserve the expansion of the source items and selected item data
    auto expandedItems = expandedSourceTreeItems();
    uint64_t selectedSourceID = 0;
    uint64_t selectedID = 0;
    auto selectionModel = ui->treeViewSources->selectionModel();
    if (selectionModel != nullptr)
    {
        auto selectedIndexes = selectionModel->selectedIndexes();
        if (selectedIndexes.length() > 0)
        {
            auto index = selectedIndexes.at(0);
            if (index.data(SourceTreeEntryTypeRole).toULongLong() == SOURCETREE_ENTRY_TYPE_FEED)
            {
                selectedSourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
                selectedID = index.data(SourceTreeEntryIDRole).toULongLong();
            }
        }
    }

    // recreate the model
    mItemModelSources = std::make_unique<StandardItemModelSources>(this, this);
    ui->treeViewSources->setModel(mItemModelSources.get());
    mItemModelSources->setHorizontalHeaderItem(0, new QStandardItem(tr("Sources & Feeds")));

    // lambda to recursively create folder items
    std::unordered_map<uint64_t, QStandardItem*> folderIDToItemMapping; // a map to quickly look up folder items when adding feed items
    std::function<void(ZapFR::Engine::Folder*, uint64_t, QStandardItem*)> createFolderItems;
    createFolderItems = [&](ZapFR::Engine::Folder* folder, uint64_t sourceID, QStandardItem* parentItem)
    {
        auto folderItem = new QStandardItem(QString::fromUtf8(folder->title()));
        folderItem->setData(SOURCETREE_ENTRY_TYPE_FOLDER, SourceTreeEntryTypeRole);
        folderItem->setData(QVariant::fromValue<uint64_t>(folder->id()), SourceTreeEntryIDRole);
        folderItem->setData(QVariant::fromValue<uint64_t>(folder->parentID()), SourceTreeEntryParentFolderIDRole);
        folderItem->setData(QVariant::fromValue<uint64_t>(sourceID), SourceTreeEntryParentSourceIDRole);
        parentItem->appendRow(folderItem);

        if (folder->hasSubfolders())
        {
            for (const auto& subfolder : folder->subfolders())
            {
                createFolderItems(subfolder, sourceID, folderItem);
            }
        }
        folderIDToItemMapping[folder->id()] = folderItem;
    };

    // process all available sources
    auto sources = ZapFR::Engine::Source::getSources({});
    for (const auto& source : sources)
    {
        // create the parent source item
        auto sourceItem = new QStandardItem(QString::fromUtf8(source->title()));
        mItemModelSources->appendRow(sourceItem);
        sourceItem->setData(SOURCETREE_ENTRY_TYPE_SOURCE, SourceTreeEntryTypeRole);
        sourceItem->setData(QVariant::fromValue<uint64_t>(source->id()), SourceTreeEntryIDRole);
        sourceItem->setData(QVariant::fromValue<uint64_t>(source->id()), SourceTreeEntryParentSourceIDRole);

        // create all the folder and subfolder items
        auto rootFolders = source->getFolders(0);
        for (const auto& folder : rootFolders)
        {
            createFolderItems(folder.get(), source->id(), sourceItem);
        }

        // create the subfolder items
        auto feeds = source->getFeeds();
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
            feedItem->setData(QVariant::fromValue<uint64_t>(source->id()), SourceTreeEntryParentSourceIDRole);
            feedItem->setData(QVariant::fromValue<uint64_t>(folderID), SourceTreeEntryParentFolderIDRole);
            auto unreadCount = feed->unreadCount();
            feedItem->setData(QVariant::fromValue<uint64_t>(unreadCount), SourceTreeEntryUnreadCount);
            if (unreadCount >= 999)
            {
                feedItem->setToolTip(tr("%1 unread").arg(unreadCount));
            }

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

            feedItem->setData(FeedIconCache::icon(feed->id()), SourceTreeEntryIcon);

            parentItem->appendRow(feedItem);
        }
    }

    // restore source item expansion and selection
    expandSourceTreeItems(expandedItems);
    if (selectedSourceID != 0 && selectedID != 0)
    {
        std::function<void(QStandardItem*)> selectIndex;
        selectIndex = [&](QStandardItem* parent)
        {
            if (parent->data(SourceTreeEntryTypeRole).toInt() == SOURCETREE_ENTRY_TYPE_FEED &&
                parent->data(SourceTreeEntryParentSourceIDRole).toULongLong() == selectedSourceID && parent->data(SourceTreeEntryIDRole).toULongLong() == selectedID)
            {
                auto indexToSelect = mItemModelSources->indexFromItem(parent);
                ui->treeViewSources->selectionModel()->select(indexToSelect, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                if (performClickOnSelection)
                {
                    ui->treeViewSources->setCurrentIndex(indexToSelect);
                }
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
}

QString ZapFR::Client::MainWindow::dataDir() const
{
    auto data = QStandardPaths::locate(QStandardPaths::StandardLocation::GenericDataLocation, "", QStandardPaths::LocateDirectory);

    auto dir = QDir(QDir::cleanPath(data + QDir::separator() + "ZapFeedReader"));
    if (!dir.exists())
    {
        dir.mkpath("ZapFeedReader");
    }
    return dir.path();
}

QString ZapFR::Client::MainWindow::configDir() const
{
    auto data = QStandardPaths::locate(QStandardPaths::StandardLocation::ConfigLocation, "", QStandardPaths::LocateDirectory);

    auto dir = QDir(QDir::cleanPath(data + QDir::separator() + "ZapFeedReader"));
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

void ZapFR::Client::MainWindow::fixPalette() const
{
    // overwrite the inactive palette with the active palette colors to get rid of the stupid unreadable gray on blue text when focus is lost on
    // the sources tree view and posts table view
    auto palette = QPalette(ui->treeViewSources->palette());
    palette.setColor(QPalette::Inactive, QPalette::Highlight, palette.color(QPalette::Active, QPalette::Highlight));
    palette.setColor(QPalette::Inactive, QPalette::HighlightedText, palette.color(QPalette::Active, QPalette::HighlightedText));
    ui->treeViewSources->setPalette(palette);

    palette = QPalette(ui->tableViewPosts->palette());
    palette.setColor(QPalette::Inactive, QPalette::Highlight, palette.color(QPalette::Active, QPalette::Highlight));
    palette.setColor(QPalette::Inactive, QPalette::HighlightedText, palette.color(QPalette::Active, QPalette::HighlightedText));
    ui->tableViewPosts->setPalette(palette);
}

void ZapFR::Client::MainWindow::sourceTreeViewItemSelected(const QModelIndex& index)
{
    if (index.isValid())
    {
        if (index.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FEED)
        {
            auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
            auto feedID = index.data(SourceTreeEntryIDRole).toULongLong();
            ZapFR::Engine::Agent::getInstance()->queueGetFeedPosts(sourceID, feedID, 100, 1, std::bind(&MainWindow::postsRetrieved, this, _1, _2));
        }
        else if (index.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FOLDER)
        {
            auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
            auto folderID = index.data(SourceTreeEntryIDRole).toULongLong();
            ZapFR::Engine::Agent::getInstance()->queueGetFolderPosts(sourceID, folderID, 100, 1, std::bind(&MainWindow::postsRetrieved, this, _1, _2));
        }
        else if (index.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_SOURCE)
        {
            auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
            ZapFR::Engine::Agent::getInstance()->queueGetSourcePosts(sourceID, 100, 1, std::bind(&MainWindow::postsRetrieved, this, _1, _2));
        }
        else
        {
            loadPosts({});
        }
        QTimer::singleShot(0, [&]() { setupToolbarEnabledStates(); });
    }
}

void ZapFR::Client::MainWindow::postsRetrieved(uint64_t sourceID, const std::vector<ZapFR::Engine::Post*>& posts)
{
    std::function<void(QStandardItem*, ZapFR::Engine::Post*)> setItemData;
    setItemData = [&](QStandardItem* item, ZapFR::Engine::Post* post)
    {
        item->setData(QVariant::fromValue<uint64_t>(post->id()), PostIDRole);
        item->setData(QVariant::fromValue<uint64_t>(sourceID), PostSourceIDRole);
        item->setData(QVariant::fromValue<uint64_t>(post->feedID()), PostFeedIDRole);
        item->setData(QVariant::fromValue<bool>(post->isRead()), PostIsReadRole);
    };

    QList<QList<QStandardItem*>> rows;
    for (const auto& post : posts)
    {
        auto unreadItem = new QStandardItem("");
        setItemData(unreadItem, post);

        auto feedItem = new QStandardItem("");
        setItemData(feedItem, post);

        auto titleItem = new QStandardItem(QString::fromUtf8(post->title()));
        setItemData(titleItem, post);

        auto datePublished = QString::fromUtf8(post->datePublished());
        auto dateItem = new QStandardItem(Utilities::prettyDate(datePublished));
        dateItem->setData(datePublished, PostISODateRole);
        setItemData(dateItem, post);

        QList<QStandardItem*> rowData;
        rowData << unreadItem << feedItem << titleItem << dateItem;
        rows << rowData;
    }

    QMetaObject::invokeMethod(this, "loadPosts", Qt::AutoConnection, rows);
}

void ZapFR::Client::MainWindow::loadPosts(const QList<QList<QStandardItem*>>& posts)
{
    mItemModelPosts = std::make_unique<QStandardItemModel>(this);
    ui->tableViewPosts->setModel(mItemModelPosts.get());
    mItemModelPosts->setHorizontalHeaderItem(PostColumnUnread, new QStandardItem(tr("Unread")));
    mItemModelPosts->setHorizontalHeaderItem(PostColumnFeed, new QStandardItem(tr("Feed")));
    mItemModelPosts->setHorizontalHeaderItem(PostColumnTitle, new QStandardItem(tr("Title")));
    mItemModelPosts->setHorizontalHeaderItem(PostColumnDate, new QStandardItem(tr("Date")));
    for (auto post : posts)
    {
        mItemModelPosts->appendRow(post);
    }
    ui->tableViewPosts->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->tableViewPosts->horizontalHeader()->resizeSection(PostColumnUnread, 50);
    ui->tableViewPosts->horizontalHeader()->resizeSection(PostColumnFeed, 40);
    ui->tableViewPosts->horizontalHeader()->resizeSection(PostColumnDate, 200);
    postsTableViewSelectionChanged({});
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

            ZapFR::Engine::Agent::getInstance()->queueMarkPostRead(mCurrentPostSourceID, mCurrentPostFeedID, mCurrentPostID,
                                                                   [&](uint64_t postID) { QMetaObject::invokeMethod(this, "postMarkedRead", Qt::AutoConnection, postID); });

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

                                                              html << "<!DOCTYPE html>\n<html><head><style type='text/css'>\n" << postStyles() << "\n</style></head><body>";
                                                              html << "<h1 class='zapfr_title'>" << QString::fromUtf8(post->title()) << "</h1>";
                                                              html << QString::fromUtf8(post->description());
                                                              html << "</body></html>";

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
                   "a { color: %4; }\n")
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

void ZapFR::Client::MainWindow::postLinkHovered(const QString& url)
{
    if (!url.isEmpty())
    {
        ui->statusbar->showMessage(url);
    }
    else
    {
        ui->statusbar->clearMessage();
    }
}

void ZapFR::Client::MainWindow::sourceTreeViewContextMenuRequested(const QPoint& p)
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

void ZapFR::Client::MainWindow::postsTableViewContextMenuRequested(const QPoint& p)
{
    mPostContextMenu->popup(ui->tableViewPosts->viewport()->mapToGlobal(p));
}

void ZapFR::Client::MainWindow::feedRefreshed()
{
    reloadSources();
}

void ZapFR::Client::MainWindow::feedAdded()
{
    reloadSources(false);
}

void ZapFR::Client::MainWindow::feedRemoved()
{
    reloadSources();
    loadPosts({});
}

void ZapFR::Client::MainWindow::feedMoved()
{
    reloadSources();
}

void ZapFR::Client::MainWindow::folderMoved()
{
    reloadSources();
}

void ZapFR::Client::MainWindow::folderRemoved()
{
    reloadSources(false);
}

void ZapFR::Client::MainWindow::postMarkedRead(uint64_t postID)
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

    reloadSources(false);
}

void ZapFR::Client::MainWindow::postMarkedUnread(uint64_t postID)
{
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

    reloadSources(false);
}

void ZapFR::Client::MainWindow::feedMarkedRead()
{
    reloadSources();
}

void ZapFR::Client::MainWindow::folderAdded()
{
    reloadSources(false);
}

void ZapFR::Client::MainWindow::setupToolbarIcons()
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

    std::function<void(const QString&, QAction*)> installIcon;
    installIcon = [&](const QString& svgResource, QAction* action)
    {
        auto svgFile = QFile(svgResource);
        svgFile.open(QIODeviceBase::ReadOnly);
        auto svgContents = QString(svgFile.readAll());
        svgFile.close();

        auto icon = QIcon();

        // Icon Normal On
        QImage normalOn;
        auto svg = svgContents;
        svg.replace("{#color}", color);
        normalOn.loadFromData(svg.toUtf8());
        icon.addPixmap(QPixmap::fromImage(normalOn), QIcon::Normal, QIcon::On);

        // Icon Disabled On
        QImage disabledOn;
        svg = svgContents;
        svg.replace("{#color}", colorDisabled);
        disabledOn.loadFromData(svg.toUtf8());
        icon.addPixmap(QPixmap::fromImage(disabledOn), QIcon::Disabled, QIcon::On);

        action->setIcon(icon);
    };

    installIcon(":/refreshFeed.svg", ui->action_Refresh_all_feeds);
    installIcon(":/markAsRead.svg", ui->action_Mark_feed_as_read);
    installIcon(":/addFeed.svg", ui->action_Add_feed);
    installIcon(":/addFolder.svg", ui->action_Add_folder);

    ui->toolBar->setStyleSheet(QString("QToolBar { border-bottom-style: none; }\n"
                                       "QToolButton:disabled { color:%1; }\n")
                                   .arg(colorDisabled));
}

void ZapFR::Client::MainWindow::setupToolbarEnabledStates()
{
    bool anythingSelected{false};
    bool feedSelected{false};

    auto selectionModel = ui->treeViewSources->selectionModel();
    if (selectionModel != nullptr)
    {
        auto selectedIndexes = selectionModel->selectedIndexes();
        if (selectedIndexes.count() == 1)
        {
            auto index = selectedIndexes.at(0);
            if (index.isValid())
            {
                anythingSelected = true;
                feedSelected = (index.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FEED);
            }
        }
    }
    ui->action_Mark_feed_as_read->setEnabled(feedSelected);
    ui->action_Add_feed->setEnabled(anythingSelected);
    ui->action_Add_folder->setEnabled(anythingSelected);
}

void ZapFR::Client::MainWindow::markFeedAsRead()
{
    auto selectionModel = ui->treeViewSources->selectionModel();
    if (selectionModel != nullptr)
    {
        auto selectedIndexes = selectionModel->selectedIndexes();
        if (selectedIndexes.count() == 1)
        {
            auto index = selectedIndexes.at(0);
            if (index.isValid())
            {
                auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
                auto feedID = index.data(SourceTreeEntryIDRole).toULongLong();
                ZapFR::Engine::Agent::getInstance()->queueMarkFeedRead(sourceID, feedID, [&]() { QMetaObject::invokeMethod(this, "feedMarkedRead", Qt::AutoConnection); });
            }
        }
    }
}

void ZapFR::Client::MainWindow::refreshAllFeeds()
{
    ZapFR::Engine::Agent::getInstance()->queueRefreshAllFeeds([&]() { QMetaObject::invokeMethod(this, "feedRefreshed", Qt::AutoConnection); });
}

void ZapFR::Client::MainWindow::createContextMenus()
{
    createContextMenuSource();
    createContextMenuFeed();
    createContextMenuFolder();
    createContextMenuPost();
}

void ZapFR::Client::MainWindow::createContextMenuSource()
{
    mSourceContextMenuSource = std::make_unique<QMenu>(nullptr);

    auto addFolderAction = new QAction(tr("Add &folder"), this);
    connect(addFolderAction, &QAction::triggered, this, &MainWindow::addFolder);
    mSourceContextMenuSource->addAction(addFolderAction);
}

void ZapFR::Client::MainWindow::createContextMenuFeed()
{
    mSourceContextMenuFeed = std::make_unique<QMenu>(nullptr);

    // Feed - Refresh
    auto refreshAction = new QAction(tr("&Refresh"), this);
    connect(refreshAction, &QAction::triggered,
            [&]()
            {
                auto index = ui->treeViewSources->currentIndex();
                if (index.isValid())
                {
                    auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
                    auto feedID = index.data(SourceTreeEntryIDRole).toULongLong();
                    ZapFR::Engine::Agent::getInstance()->queueRefreshFeed(sourceID, feedID, [&]() { QMetaObject::invokeMethod(this, "feedRefreshed", Qt::AutoConnection); });
                }
            });
    mSourceContextMenuFeed->addAction(refreshAction);

    // Feed - Mark all as read
    auto markAllAsReadAction = new QAction(tr("&Mark all as read"), this);
    connect(markAllAsReadAction, &QAction::triggered, this, &MainWindow::markFeedAsRead);
    mSourceContextMenuFeed->addAction(markAllAsReadAction);

    mSourceContextMenuFeed->addSeparator();

    // Feed - Remove
    auto removeAction = new QAction(tr("Remo&ve"), this);
    connect(removeAction, &QAction::triggered,
            [&]()
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
                messageBoxLayout->addItem(new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), messageBoxLayout->rowCount(), 0, 1,
                                          messageBoxLayout->columnCount());
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
            });
    mSourceContextMenuFeed->addAction(removeAction);
}

void ZapFR::Client::MainWindow::createContextMenuFolder()
{
    mSourceContextMenuFolder = std::make_unique<QMenu>(nullptr);

    // Folder - Add subfolder
    auto addFolderAction = new QAction(tr("Add &subfolder"), this);
    connect(addFolderAction, &QAction::triggered, this, &MainWindow::addFolder);
    mSourceContextMenuFolder->addAction(addFolderAction);

    // Folder - Remove
    auto removeFolderAction = new QAction(tr("Remo&ve"), this);
    connect(removeFolderAction, &QAction::triggered,
            [&]()
            {
                QMessageBox messageBox;
                messageBox.setText(tr("Remove folder"));
                messageBox.setWindowTitle(tr("Remove folder"));
                messageBox.setInformativeText(
                    tr("Are you sure you want to remove this folder, all its subfolders, and all feeds they contain? All associated posts will be removed!"));
                messageBox.setIcon(QMessageBox::Warning);
                auto yesButton = messageBox.addButton(QMessageBox::StandardButton::Yes);
                yesButton->setText(tr("Remove"));
                messageBox.addButton(QMessageBox::StandardButton::Cancel);
                auto messageBoxLayout = qobject_cast<QGridLayout*>(messageBox.layout());
                messageBoxLayout->addItem(new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), messageBoxLayout->rowCount(), 0, 1,
                                          messageBoxLayout->columnCount());
                messageBox.exec();
                if (messageBox.buttonRole(messageBox.clickedButton()) == QMessageBox::YesRole)
                {
                    auto index = ui->treeViewSources->currentIndex();
                    if (index.isValid())
                    {
                        auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
                        auto folder = index.data(SourceTreeEntryIDRole).toULongLong();

                        ZapFR::Engine::Agent::getInstance()->queueRemoveFolder(sourceID, folder,
                                                                               [&]() { QMetaObject::invokeMethod(this, "folderRemoved", Qt::AutoConnection); });
                    }
                }
            });
    mSourceContextMenuFolder->addAction(removeFolderAction);
}

void ZapFR::Client::MainWindow::createContextMenuPost()
{
    mPostContextMenu = std::make_unique<QMenu>(nullptr);

    auto markPostUnreadAction = new QAction(tr("&Mark as unread"), this);
    connect(markPostUnreadAction, &QAction::triggered,
            [&]()
            {
                auto selectionModel = ui->tableViewPosts->selectionModel();
                if (selectionModel != nullptr)
                {
                    auto selectedIndexes = selectionModel->selectedIndexes();
                    for (const auto& index : selectedIndexes)
                    {
                        mCurrentPostID = index.data(PostIDRole).toULongLong();
                        mCurrentPostSourceID = index.data(PostSourceIDRole).toULongLong();
                        mCurrentPostFeedID = index.data(PostFeedIDRole).toULongLong();

                        ZapFR::Engine::Agent::getInstance()->queueMarkPostUnread(mCurrentPostSourceID, mCurrentPostFeedID, mCurrentPostID,
                                                                                 [&](uint64_t postID)
                                                                                 { QMetaObject::invokeMethod(this, "postMarkedUnread", Qt::AutoConnection, postID); });
                    }
                }
            });
    mPostContextMenu->addAction(markPostUnreadAction);
}
