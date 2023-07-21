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
#include "AgentGetPosts.h"
#include "AgentRefreshFeed.h"
#include "AgentRemoveFeed.h"
#include "AgentRemoveFolder.h"
#include "Feed.h"
#include "ItemDelegatePost.h"
#include "ItemDelegateSource.h"
#include "Post.h"
#include "Source.h"
#include "StandardItemModelSources.h"
#include "Utilities.h"
#include "WebEnginePagePost.h"

ZapFR::Client::MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    mDatabase = std::make_unique<ZapFR::Engine::Database>(QDir::cleanPath(dataDir() + QDir::separator() + "zapfeedreader-client.db").toStdString());
    ZapFR::Engine::Source::registerDatabaseInstance(mDatabase.get());
    ZapFR::Engine::Feed::registerDatabaseInstance(mDatabase.get());
    ZapFR::Engine::Post::registerDatabaseInstance(mDatabase.get());

    ui->setupUi(this);
    connect(ui->action_Add_source, &QAction::triggered, this, &MainWindow::addSource);
    connect(ui->action_Add_feed, &QAction::triggered, this, &MainWindow::addFeed);
    connect(ui->action_Import_OPML, &QAction::triggered, this, &MainWindow::importOPML);
    connect(ui->treeViewSources, &TreeViewSources::customContextMenuRequested, this, &MainWindow::sourceTreeViewContextMenuRequested);
    connect(ui->treeViewSources, &TreeViewSources::currentSourceChanged, this, &MainWindow::sourceTreeViewItemSelected);
    connect(ui->tableViewPosts, &TableViewPosts::currentPostChanged, this, &MainWindow::postsTableViewItemSelected);
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, &MainWindow::colorSchemeChanged);

    fixPalette();
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
                    o.insert("title", getFolderHierarchy(parent));
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
            QString folderHierarchyToMatch;
            auto typeToMatch = QString("");
            if (parent->data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_SOURCE)
            {
                typeToMatch = "source";
            }
            else if (parent->data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FOLDER)
            {
                typeToMatch = "folder";
                folderHierarchyToMatch = getFolderHierarchy(parent);
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
                        auto title = o.value("title").toString();
                        shouldExpand = (sourceID == sourceIDToMatch && title == folderHierarchyToMatch);
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
                        ZapFR::Engine::Agent::getInstance()->queueSubscribeFeed(mDialogAddFeed->sourceID(), mDialogAddFeed->url().toStdString(),
                                                                                mDialogAddFeed->folderHierarchy().toStdString(),
                                                                                [&]() { QMetaObject::invokeMethod(this, "feedAdded", Qt::AutoConnection); });
                    }
                });
    }

    auto sources = ZapFR::Engine::Source::getSources({});
    mDialogAddFeed->reset(sources);
    mDialogAddFeed->open();
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
                            auto folderHierarchy = mDialogImportOPML->folderHierarchy();
                            if (!feed.folderHierarchy.empty() && !folderHierarchy.endsWith("/"))
                            {
                                folderHierarchy += "/";
                            }
                            folderHierarchy += QString::fromUtf8(feed.folderHierarchy);

                            ZapFR::Engine::Agent::getInstance()->queueSubscribeFeed(mDialogImportOPML->sourceID(), feed.url, folderHierarchy.toStdString(),
                                                                                    [&]() { QMetaObject::invokeMethod(this, "feedAdded", Qt::AutoConnection); });
                        }
                    }
                });
    }

    auto sources = ZapFR::Engine::Source::getSources({});
    mDialogImportOPML->reset(sources);
    mDialogImportOPML->open();
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

    auto sources = ZapFR::Engine::Source::getSources({});
    for (const auto& source : sources)
    {
        // create the parent source item
        auto sourceItem = new QStandardItem(QString::fromUtf8(source->title()));
        mItemModelSources->appendRow(sourceItem);
        sourceItem->setData(SOURCETREE_ENTRY_TYPE_SOURCE, SourceTreeEntryTypeRole);
        sourceItem->setData(QVariant::fromValue<uint64_t>(source->id()), SourceTreeEntryIDRole);
        sourceItem->setData(QVariant::fromValue<uint64_t>(source->id()), SourceTreeEntryParentSourceIDRole);

        // create the subfolder items
        auto feeds = source->getFeeds();
        for (const auto& feed : feeds)
        {
            auto currentParent = sourceItem;
            auto folderHierarchy = QString::fromUtf8(feed->folderHierarchy());
            if (!folderHierarchy.isEmpty())
            {
                auto subfolders = folderHierarchy.split("/", Qt::SkipEmptyParts);
                for (const auto& subfolder : subfolders)
                {
                    auto subfolderFound{false};
                    for (int i = 0; i < currentParent->rowCount(); ++i)
                    {
                        auto item = currentParent->child(i);
                        if (item->data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FOLDER && item->data(Qt::DisplayRole).toString() == subfolder)
                        {
                            subfolderFound = true;
                            currentParent = item;
                            break;
                        }
                    }
                    if (!subfolderFound)
                    {
                        auto subfolderItem = new QStandardItem(subfolder);
                        subfolderItem->setData(SOURCETREE_ENTRY_TYPE_FOLDER, SourceTreeEntryTypeRole);
                        subfolderItem->setData(QVariant::fromValue<uint64_t>(source->id()), SourceTreeEntryParentSourceIDRole);
                        currentParent->appendRow(subfolderItem);
                        currentParent = subfolderItem;
                    }
                }
            }

            // create the feed item
            auto feedItem = new QStandardItem(QString::fromUtf8(feed->title()));
            feedItem->setData(SOURCETREE_ENTRY_TYPE_FEED, SourceTreeEntryTypeRole);
            feedItem->setData(QVariant::fromValue<uint64_t>(feed->id()), SourceTreeEntryIDRole);
            feedItem->setData(QVariant::fromValue<uint64_t>(source->id()), SourceTreeEntryParentSourceIDRole);
            auto unreadCount = feed->unreadCount();
            feedItem->setData(QVariant::fromValue<uint64_t>(unreadCount), SourceTreeEntryUnreadCount);
            if (unreadCount >= 999)
            {
                feedItem->setToolTip(tr("%1 unread").arg(unreadCount));
            }
            currentParent->appendRow(feedItem);
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
                    sourceTreeViewItemSelected(indexToSelect);
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

QString ZapFR::Client::MainWindow::getFolderHierarchy(QStandardItem* parent) const
{
    std::function<void(QStandardItem*, QStringList&)> getFolderHierarchy;
    getFolderHierarchy = [&](QStandardItem* item, QStringList& subfolders)
    {
        if (item->data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FOLDER)
        {
            subfolders.insert(0, item->data(Qt::DisplayRole).toString());
            getFolderHierarchy(item->parent(), subfolders);
        }
    };

    QStringList subfolders;
    getFolderHierarchy(parent, subfolders);
    return subfolders.join("/");
}

void ZapFR::Client::MainWindow::sourceTreeViewItemSelected(const QModelIndex& index)
{
    if (index.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FEED)
    {

        auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
        auto feedID = index.data(SourceTreeEntryIDRole).toULongLong();
        ZapFR::Engine::Agent::getInstance()->queueGetPosts(sourceID, feedID, 100, 1,
                                                           [&](uint64_t sourceID, uint64_t feedID, std::vector<std::unique_ptr<ZapFR::Engine::Post>> posts)
                                                           {
                                                               QList<QList<QStandardItem*>> rows;
                                                               for (const auto& post : posts)
                                                               {
                                                                   auto titleItem = new QStandardItem(QString::fromUtf8(post->title()));
                                                                   titleItem->setData(QVariant::fromValue<uint64_t>(post->id()), PostIDRole);
                                                                   titleItem->setData(QVariant::fromValue<uint64_t>(sourceID), PostSourceIDRole);
                                                                   titleItem->setData(QVariant::fromValue<uint64_t>(feedID), PostFeedDRole);
                                                                   titleItem->setData(QVariant::fromValue<bool>(post->isRead()), PostIsReadRole);

                                                                   auto datePublished = QString::fromUtf8(post->datePublished());
                                                                   auto dateItem = new QStandardItem(Utilities::prettyDate(datePublished));
                                                                   dateItem->setData(datePublished, PostISODateRole);
                                                                   dateItem->setData(QVariant::fromValue<uint64_t>(post->id()), PostIDRole);
                                                                   dateItem->setData(QVariant::fromValue<uint64_t>(sourceID), PostSourceIDRole);
                                                                   dateItem->setData(QVariant::fromValue<uint64_t>(feedID), PostFeedDRole);
                                                                   dateItem->setData(QVariant::fromValue<bool>(post->isRead()), PostIsReadRole);

                                                                   QList<QStandardItem*> rowData;
                                                                   rowData << titleItem << dateItem;
                                                                   rows << rowData;
                                                               }

                                                               QMetaObject::invokeMethod(this, "loadPosts", Qt::AutoConnection, rows);
                                                           });
    }
}

void ZapFR::Client::MainWindow::loadPosts(const QList<QList<QStandardItem*>>& posts)
{
    mItemModelPosts = std::make_unique<QStandardItemModel>(this);
    ui->tableViewPosts->setModel(mItemModelPosts.get());
    mItemModelPosts->setHorizontalHeaderItem(0, new QStandardItem(tr("Title")));
    mItemModelPosts->setHorizontalHeaderItem(1, new QStandardItem(tr("Date")));
    for (auto post : posts)
    {
        mItemModelPosts->appendRow(post);
    }
    ui->tableViewPosts->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableViewPosts->horizontalHeader()->setMinimumSectionSize(200);
    postsTableViewItemSelected(QModelIndex());
}

void ZapFR::Client::MainWindow::postsTableViewItemSelected(const QModelIndex& index)
{
    if (index.isValid())
    {
        mCurrentPostID = index.data(PostIDRole).toULongLong();
        mCurrentPostSourceID = index.data(PostSourceIDRole).toULongLong();
        mCurrentPostFeedID = index.data(PostFeedDRole).toULongLong();

        ZapFR::Engine::Agent::getInstance()->queueMarkPostRead(mCurrentPostSourceID, mCurrentPostFeedID, mCurrentPostID,
                                                               [&]() { QMetaObject::invokeMethod(this, "postMarkedRead", Qt::AutoConnection); });

        for (int32_t col = 0; col < mItemModelPosts->columnCount(); ++col)
        {
            auto item = mItemModelPosts->item(index.row(), col);
            item->setData(QVariant::fromValue<bool>(true), PostIsReadRole);
        }
        ui->tableViewPosts->scrollTo(index);
    }
    else
    {
        mCurrentPostID = 0;
        mCurrentPostSourceID = 0;
        mCurrentPostFeedID = 0;
    }
    reloadCurrentPost();
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
        QString htmlStr;
        QTextStream html(&htmlStr, QIODeviceBase::ReadWrite);

        html << "<!DOCTYPE html>\n<html><head><style type='text/css'>\n" << postStyles() << "\n</style></head><body>";
        html << "</body></html>";
        setPostHTML(htmlStr);
    }
}

void ZapFR::Client::MainWindow::setPostHTML(const QString& html)
{
    ui->webViewPost->setHtml(html);
}

QString ZapFR::Client::MainWindow::postStyles() const
{
    auto font = ui->treeViewSources->font();
    auto commonStyles = QString::fromUtf8(R"(body { font-family: "%1", sans-serif; }\n)").arg(font.family());

    auto currentColorScheme = QGuiApplication::styleHints()->colorScheme();
    if (currentColorScheme == Qt::ColorScheme::Dark)
    {
        auto override = QFile(QDir::cleanPath(configDir() + QDir::separator() + "posttheme.dark.css"));
        if (override.exists())
        {
            override.open(QIODeviceBase::ReadOnly);
            auto styles = QString::fromUtf8(override.readAll());
            override.close();
            return styles;
        }
        return "body { background-color: #2a2a2a; color: #fff; }\n" + commonStyles;
        ;
    }
    else
    {
        auto override = QFile(QDir::cleanPath(configDir() + QDir::separator() + "posttheme.light.css"));
        if (override.exists())
        {
            override.open(QIODeviceBase::ReadOnly);
            auto styles = QString::fromUtf8(override.readAll());
            override.close();
            return styles;
        }
        return "body { background-color: #fff; color: #000; }\n" + commonStyles;
    }
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
        }
    }
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

void ZapFR::Client::MainWindow::folderRemoved()
{
    reloadSources(false);
}

void ZapFR::Client::MainWindow::postMarkedRead()
{
    reloadSources(false);
}

void ZapFR::Client::MainWindow::createContextMenus()
{
    // FEEDS
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
                    ZapFR::Engine::Agent::getInstance()->queueRefreshFeed(
                        sourceID, feedID, [&](uint64_t /*sourceID*/, uint64_t /*feedID*/) { QMetaObject::invokeMethod(this, "feedRefreshed", Qt::AutoConnection); });
                }
            });
    mSourceContextMenuFeed->addAction(refreshAction);

    // Feed - Mark all as read
    auto markAllAsReadAction = new QAction(tr("&Mark all as read"), this);
    connect(markAllAsReadAction, &QAction::triggered,
            [&]()
            {
                auto index = ui->treeViewSources->currentIndex();
                if (index.isValid())
                {
                    auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
                    auto feedID = index.data(SourceTreeEntryIDRole).toULongLong();

                    auto source = ZapFR::Engine::Source::getSource(sourceID);
                    if (source.has_value())
                    {
                        auto feed = source.value()->getFeed(feedID);
                        if (feed.has_value())
                        {
                            feed.value()->markAllAsRead();
                            // TODO, this should reload the posts, so they can reflect read status
                        }
                    }
                    reloadSources();
                }
            });
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

    // FOLDERS
    mSourceContextMenuFolder = std::make_unique<QMenu>(nullptr);

    // Folder - remove
    auto removeFolderAction = new QAction(tr("Remo&ve"), this);
    connect(removeFolderAction, &QAction::triggered,
            [&]()
            {
                QMessageBox messageBox;
                messageBox.setText(tr("Remove folder"));
                messageBox.setWindowTitle(tr("Remove folder"));
                messageBox.setInformativeText(tr("Are you sure you want to remove this folder and all feeds it contains? All associated posts will be removed!"));
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
                        auto cur = index;
                        QStringList subfolders;
                        while (true)
                        {
                            subfolders << cur.data(Qt::DisplayRole).toString();
                            cur = cur.parent();
                            if (!cur.isValid() || cur.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_SOURCE)
                            {
                                break;
                            }
                        }
                        std::reverse(subfolders.begin(), subfolders.end());
                        auto folderHierarchy = subfolders.join("/").toStdString();

                        ZapFR::Engine::Agent::getInstance()->queueRemoveFolder(index.data(SourceTreeEntryParentSourceIDRole).toULongLong(), folderHierarchy,
                                                                               [&]() { QMetaObject::invokeMethod(this, "folderRemoved", Qt::AutoConnection); });
                    }
                }
            });
    mSourceContextMenuFolder->addAction(removeFolderAction);
}
