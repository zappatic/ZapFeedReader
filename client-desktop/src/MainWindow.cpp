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
#include "Feed.h"
#include "ItemDelegateSource.h"
#include "Post.h"
#include "Source.h"
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardItem>
#include <QStandardPaths>

static const QString SETTING_MAINWINDOW_STATE = "mainwindow.state";
static const QString SETTING_MAINWINDOW_GEOMETRY = "mainwindow.geometry";
static const QString SETTING_SPLITTERLEFT_STATE = "splitterleft.state";
static const QString SETTING_SPLITTERRIGHT_STATE = "splitterright.state";
static const QString SETTING_SOURCETREEVIEW_EXPANSION = "sourcetreeview.expansion";

static constexpr uint32_t SOURCETREE_ENTRY_TYPE_SOURCE = 0;
static constexpr uint32_t SOURCETREE_ENTRY_TYPE_FEED = 1;
static constexpr uint32_t SOURCETREE_ENTRY_TYPE_FOLDER = 2;
static constexpr uint32_t SourceTreeEntryTypeRole{Qt::ItemDataRole::UserRole + 1};
static constexpr uint32_t SourceTreeEntryIDRole{Qt::ItemDataRole::UserRole + 2};
static constexpr uint32_t SourceTreeEntryParentSourceIDRole{Qt::ItemDataRole::UserRole + 3};

ZapFR::Client::MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    mDatabase = std::make_unique<ZapFR::Engine::Database>(QDir::cleanPath(dataDir() + QDir::separator() + "zapfeedreader-client.db").toStdString());
    ZapFR::Engine::Source::registerDatabaseInstance(mDatabase.get());
    ZapFR::Engine::Feed::registerDatabaseInstance(mDatabase.get());
    ZapFR::Engine::Post::registerDatabaseInstance(mDatabase.get());

    ui->setupUi(this);
    connect(ui->action_Add_source, &QAction::triggered, this, &MainWindow::addSource);
    connect(ui->action_Add_feed, &QAction::triggered, this, &MainWindow::addFeed);
    connect(ui->treeViewSources, &QTreeView::clicked, this, &MainWindow::sourceTreeViewItemClicked);

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

    reloadSources();
    ui->treeViewSources->setItemDelegate(new ItemDelegateSource(ui->treeViewSources));

    restoreSettings();
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
    root.insert(SETTING_SPLITTERRIGHT_STATE, QString::fromUtf8(ui->splitterRight->saveState().toBase64()));

    // save which sources/folders are expanded in the source tree view
    QJsonArray expandedSourceTreeItems;
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
    root.insert(SETTING_SOURCETREEVIEW_EXPANSION, expandedSourceTreeItems);

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
                    auto expansions = root.value(SETTING_SOURCETREEVIEW_EXPANSION).toArray();
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

                            for (const auto& entry : expansions)
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
            }
        }
    }
    catch (...)
    {
    }
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
                        try
                        {
                            auto source = ZapFR::Engine::Source::getSource(mDialogAddFeed->sourceID());
                            if (source.has_value())
                            {
                                source.value()->addFeed(mDialogAddFeed->url().toStdString());
                                reloadSources();
                            }
                        }
                        catch (const std::runtime_error& e)
                        {
                            std::cerr << e.what() << "\n";
                        }
                    }
                });
    }

    auto sources = ZapFR::Engine::Source::getSources({});
    mDialogAddFeed->reset(sources);
    mDialogAddFeed->open();
}

void ZapFR::Client::MainWindow::reloadSources()
{
    mItemModelSources = std::make_unique<QStandardItemModel>(this);
    ui->treeViewSources->setModel(mItemModelSources.get());
    mItemModelSources->setHorizontalHeaderItem(0, new QStandardItem(tr("Sources & Feeds")));

    auto sources = ZapFR::Engine::Source::getSources({});
    for (const auto& source : sources)
    {
        auto sourceItem = new QStandardItem(QString::fromUtf8(source->title()));
        mItemModelSources->appendRow(sourceItem);
        sourceItem->setData(SOURCETREE_ENTRY_TYPE_SOURCE, SourceTreeEntryTypeRole);
        sourceItem->setData(QVariant::fromValue<uint64_t>(source->id()), SourceTreeEntryIDRole);
        sourceItem->setData(QVariant::fromValue<uint64_t>(source->id()), SourceTreeEntryParentSourceIDRole);

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

            auto feedItem = new QStandardItem(QString::fromUtf8(feed->title()));
            feedItem->setData(SOURCETREE_ENTRY_TYPE_FEED, SourceTreeEntryTypeRole);
            feedItem->setData(QVariant::fromValue<uint64_t>(feed->id()), SourceTreeEntryIDRole);
            feedItem->setData(QVariant::fromValue<uint64_t>(source->id()), SourceTreeEntryParentSourceIDRole);
            currentParent->appendRow(feedItem);
        }
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

void ZapFR::Client::MainWindow::sourceTreeViewItemClicked(const QModelIndex& index)
{
    if (index.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FEED)
    {
        auto source = ZapFR::Engine::Source::getSource(index.data(SourceTreeEntryParentSourceIDRole).toULongLong());
        if (source.has_value())
        {
            auto feed = source.value()->getFeed(index.data(SourceTreeEntryIDRole).toULongLong());
            if (feed.has_value())
            {
                auto posts = feed.value()->getPosts(100, 1);

                mItemModelPosts = std::make_unique<QStandardItemModel>(this);
                ui->tableViewPosts->setModel(mItemModelPosts.get());
                mItemModelPosts->setHorizontalHeaderItem(0, new QStandardItem(tr("Title")));
                mItemModelPosts->setHorizontalHeaderItem(1, new QStandardItem(tr("Date")));

                for (const auto& post : posts)
                {
                    QList<QStandardItem*> rowData;
                    rowData << new QStandardItem(QString::fromUtf8(post->title()));
                    rowData << new QStandardItem(QString::fromUtf8(post->datePublished()));
                    mItemModelPosts->appendRow(rowData);
                }
                ui->tableViewPosts->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
                ui->tableViewPosts->horizontalHeader()->setMinimumSectionSize(200);
            }
        }
    }
}
