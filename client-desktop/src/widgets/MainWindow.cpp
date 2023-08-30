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

#include "widgets/MainWindow.h"
#include "./ui_MainWindow.h"
#include "Utilities.h"
#include "ZapFR/Agent.h"
#include "ZapFR/Database.h"
#include "ZapFR/FeedLocal.h"
#include "ZapFR/Flag.h"
#include "ZapFR/Folder.h"
#include "ZapFR/Log.h"
#include "ZapFR/Post.h"
#include "ZapFR/ScriptFolderLocal.h"
#include "ZapFR/ScriptLocal.h"
#include "delegates/ItemDelegateLog.h"
#include "delegates/ItemDelegatePost.h"
#include "delegates/ItemDelegateScript.h"
#include "delegates/ItemDelegateSource.h"
#include "dialogs/DialogAddFeed.h"
#include "dialogs/DialogAddFolder.h"
#include "dialogs/DialogEditScript.h"
#include "dialogs/DialogEditScriptFolder.h"
#include "dialogs/DialogImportOPML.h"
#include "dialogs/DialogJumpToPage.h"
#include "models/SortFilterProxyModelSources.h"
#include "models/StandardItemModelSources.h"
#include "widgets/LineEditSearch.h"
#include "widgets/WebEnginePagePost.h"

namespace
{
    auto gsPostPaneToolbarSpacerRight{"postPaneToolbarSpacerRight"};
    auto gsPostPaneToolbarSpacerLeft{"postPaneToolbarSpacerLeft"};
    auto gsPostPaneLineEditSearch{"postPaneLineEditSearch"};
    auto gsHamburgerMenuButton{"hamburgerMenuButton"};
} // namespace

ZapFR::Client::MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ZapFR::Engine::ScriptLocal::setScriptDir(QDir::cleanPath(dataDir() + QDir::separator() + "scripts").toStdString());
    ZapFR::Engine::FeedLocal::setIconDir(QDir::cleanPath(dataDir() + QDir::separator() + "icons").toStdString());
    ZapFR::Engine::Database::getInstance()->initialize(QDir::cleanPath(dataDir() + QDir::separator() + "zapfeedreader.db").toStdString(),
                                                       ZapFR::Engine::ApplicationType::Client);
    mPostWebEnginePage = std::make_unique<WebEnginePagePost>(this);

    ui->setupUi(this);
    initializeUI();
    configureConnects();
    createContextMenus();
    configureIcons();
    restoreSettings();
    reloadSources();
    reloadCurrentPost();
    updateActivePostFilter();
}

ZapFR::Client::MainWindow::~MainWindow()
{
    delete ui;
}

void ZapFR::Client::MainWindow::initializeUI()
{
    ui->treeViewSources->setItemDelegate(new ItemDelegateSource(ui->treeViewSources));
    ui->tableViewPosts->setItemDelegate(new ItemDelegatePost(ui->tableViewPosts));
    ui->tableViewLogs->setItemDelegate(new ItemDelegateLog(ui->tableViewLogs));
    ui->tableViewScripts->setItemDelegate(new ItemDelegateScript(ui->tableViewScripts));
    ui->webViewPost->setPage(mPostWebEnginePage.get());

    // prevent the left splitter from resizing while the window resizes
    ui->splitterLeft->setStretchFactor(1, 100);

    // add a spacer in the toolbar to separate the left from the right buttons
    auto spacerWidget = new QWidget();
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto spacerAction = ui->toolBar->insertWidget(ui->action_View_logs, spacerWidget);
    spacerAction->setProperty(gsPostPaneToolbarSpacerLeft, true);

    // add the search widget to the toolbar
    mLineEditSearch = new LineEditSearch();
    auto actionSearch = ui->toolBar->insertWidget(ui->action_View_logs, mLineEditSearch);
    actionSearch->setProperty(gsPostPaneLineEditSearch, true);

    // add the hamburger menu button to the toolbar
    mHamburgerMenuButton = new QPushButton();
    mHamburgerMenuButton->setFlat(true);
    mHamburgerMenuButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto actionHamburgerMenu = ui->toolBar->insertWidget(ui->action_Dummy, mHamburgerMenuButton);
    actionHamburgerMenu->setProperty(gsHamburgerMenuButton, true);
    ui->toolBar->removeAction(ui->action_Dummy);

    // add a spacer in the toolbar before the hamburger menu to ensure it is always at the right
    auto spacerWidget2 = new QWidget();
    spacerWidget2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto spacerAction2 = ui->toolBar->insertWidget(actionHamburgerMenu, spacerWidget2);
    spacerAction2->setProperty(gsPostPaneToolbarSpacerRight, true);

    ui->menubar->setVisible(false);

    ui->stackedWidgetRight->setCurrentIndex(StackedPanePosts);
    ui->stackedWidgetPost->setCurrentIndex(StackedPanePost);
}

void ZapFR::Client::MainWindow::closeEvent(QCloseEvent* /*event*/)
{
    saveSettings();
    ZapFR::Engine::Agent::getInstance()->joinAll();
}

void ZapFR::Client::MainWindow::saveSettings() const
{
    QJsonObject root;
    root.insert(SETTING_MAINWINDOW_STATE, QString::fromUtf8(saveState().toBase64()));
    root.insert(SETTING_MAINWINDOW_GEOMETRY, QString::fromUtf8(saveGeometry().toBase64()));
    root.insert(SETTING_SPLITTERLEFT_STATE, QString::fromUtf8(ui->splitterLeft->saveState().toBase64()));
    root.insert(SETTING_SPLITTERLEFTINNER_STATE, QString::fromUtf8(ui->splitterLeftInner->saveState().toBase64()));
    root.insert(SETTING_SPLITTERRIGHT_STATE, QString::fromUtf8(ui->splitterRight->saveState().toBase64()));
    switch (mProxyModelSources->displayMode())
    {
        case SortFilterProxyModelSources::SourceTreeDisplayMode::ShowAll:
        {
            root.insert(SETTING_SOURCETREEVIEW_EXPANSION, expandedSourceTreeItems());
            break;
        }
        case SortFilterProxyModelSources::SourceTreeDisplayMode::ShowSourcesOnly:
        {
            if (mReloadSourcesExpansionSelectionState != nullptr)
            {
                auto expandedItems = mReloadSourcesExpansionSelectionState->value("expanded").toArray();
                root.insert(SETTING_SOURCETREEVIEW_EXPANSION, expandedItems);
            }
            break;
        }
    }

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

void ZapFR::Client::MainWindow::exportOPML()
{
    auto index = ui->treeViewSources->currentIndex();
    if (!index.isValid())
    {
        return;
    }

    auto sourceItem = findSourceStandardItem(index.data(SourceTreeEntryParentSourceIDRole).toULongLong());
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
            auto childType = child->data(SourceTreeEntryTypeRole).toULongLong();
            switch (childType)
            {
                case SOURCETREE_ENTRY_TYPE_FOLDER:
                {
                    auto outline = xml.createElement("outline");
                    outline.setAttribute("text", child->data(Qt::DisplayRole).toString());
                    parentElement.appendChild(outline);
                    addOutlines(child, outline);
                    break;
                }
                case SOURCETREE_ENTRY_TYPE_FEED:
                {
                    auto outline = xml.createElement("outline");
                    outline.setAttribute("type", "rss");
                    outline.setAttribute("text", child->data(Qt::DisplayRole).toString());
                    outline.setAttribute("xmlUrl", child->data(SourceTreeEntryFeedURLRole).toString());
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
    ui->action_Edit_script_folder->setIcon(configureIcon(":/edit.svg"));
    ui->action_Remove_script_folder->setIcon(configureIcon(":/remove.svg"));
    ui->action_Add_script_folder->setIcon(configureIcon(":/addFeed.svg"));
    ui->action_View_properties->setIcon(configureIcon(":/properties.svg"));
    ui->pushButtonPostPreviousPage->setIcon(configureIcon(":/previousPage.svg"));
    ui->pushButtonPostFirstPage->setIcon(configureIcon(":/firstPage.svg"));
    ui->pushButtonPostNextPage->setIcon(configureIcon(":/nextPage.svg"));
    ui->pushButtonPostLastPage->setIcon(configureIcon(":/lastPage.svg"));
    ui->pushButtonLogPreviousPage->setIcon(configureIcon(":/previousPage.svg"));
    ui->pushButtonLogFirstPage->setIcon(configureIcon(":/firstPage.svg"));
    ui->pushButtonLogNextPage->setIcon(configureIcon(":/nextPage.svg"));
    ui->pushButtonLogLastPage->setIcon(configureIcon(":/lastPage.svg"));
    mHamburgerMenuButton->setIcon(configureIcon(":/hamburger.svg"));

    ui->toolBar->setStyleSheet(QString("QToolBar { border-bottom-style: none; }\n"
                                       "QToolButton:disabled { color:%1; }\n")
                                   .arg(colorDisabled));

    auto palette = ui->frameFlagFilters->palette();
    ui->frameFlagFilters->setStyleSheet(QString("QFrame { border-top: 0px; border-left: 0px; border-right: 1px solid %1; border-bottom: 1px solid %1;}")
                                            .arg(palette.color(QPalette::Active, QPalette::Dark).name()));

    mLineEditSearch->setSearchIconColor(currentColorScheme == Qt::ColorScheme::Dark ? "#eee" : "#333");
}

void ZapFR::Client::MainWindow::updateToolbar()
{
    // hide all actions
    for (const auto& action : ui->toolBar->actions())
    {
        if (!action->property(gsHamburgerMenuButton).isValid())
        {
            action->setVisible(false);
        }
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
                if (action->property(gsPostPaneToolbarSpacerLeft).isValid() || action->property(gsPostPaneLineEditSearch).isValid())
                {
                    action->setVisible(true);
                }
            }
            break;
        }
        case StackedPaneLogs:
        {
            ui->action_Back_to_posts->setVisible(true);
            for (const auto& action : ui->toolBar->actions())
            {
                if (action->property(gsPostPaneToolbarSpacerRight).isValid())
                {
                    action->setVisible(true);
                }
            }
            break;
        }
        case StackedPaneScripts:
        {
            ui->action_Back_to_posts->setVisible(true);
            ui->action_Edit_script->setVisible(true);
            ui->action_Remove_script->setVisible(true);
            ui->action_Add_script->setVisible(true);
            for (const auto& action : ui->toolBar->actions())
            {
                if (action->property(gsPostPaneToolbarSpacerRight).isValid())
                {
                    action->setVisible(true);
                }
            }
            break;
        }
        case StackedPaneProperties:
        {
            ui->action_Back_to_posts->setVisible(true);
            for (const auto& action : ui->toolBar->actions())
            {
                if (action->property(gsPostPaneToolbarSpacerRight).isValid())
                {
                    action->setVisible(true);
                }
            }
            break;
        }
    }
}

void ZapFR::Client::MainWindow::updateActivePostFilter()
{
    auto selectedScriptFolder = ui->tableViewScriptFolders->currentIndex();

    auto isScriptFolderFilterActive{selectedScriptFolder.isValid()};
    auto isFlagFilterActive{mFlagFilter != ZapFR::Engine::FlagColor::Gray};
    auto isOnlyUnreadFilterActive{mShowOnlyUnreadPosts};
    auto isTextSearchFilterActive{!mLineEditSearch->text().isEmpty()};
    auto isOtherFilterActive{isScriptFolderFilterActive || isOnlyUnreadFilterActive || isTextSearchFilterActive};

    ui->labelActiveFilter->setVisible(isFlagFilterActive || isOtherFilterActive);
    ui->labelActiveFilterFlag->setVisible(isFlagFilterActive);
    ui->labelActiveFilterOther->setVisible(isOtherFilterActive);

    if (isFlagFilterActive)
    {
        ui->labelActiveFilterFlag->setPixmap(Utilities::flag(mFlagFilter, Utilities::FlagStyle::Filled));
    }
    QStringList otherFilters;
    if (isScriptFolderFilterActive)
    {
        otherFilters << tr("Script folder '%1'").arg(selectedScriptFolder.data(Qt::DisplayRole).toString());
    }
    if (isOnlyUnreadFilterActive)
    {
        otherFilters << tr("Only unread");
    }
    if (isTextSearchFilterActive)
    {
        otherFilters << tr("Search '%1'").arg(mLineEditSearch->text());
    }

    if (isOtherFilterActive)
    {
        ui->labelActiveFilterOther->setText(otherFilters.join(", "));
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
    createSourceContextMenus();
    createFolderContextMenus();
    createFeedContextMenus();
    createPostContextMenus();
    createScriptContextMenus();
    createScriptFolderContextMenus();
}

void ZapFR::Client::MainWindow::configureConnects()
{
    connect(ui->action_Import_OPML, &QAction::triggered, this, &MainWindow::importOPML);
    connect(ui->action_Export_OPML, &QAction::triggered, this, &MainWindow::exportOPML);
    connect(ui->action_Exit, &QAction::triggered, [&]() { QGuiApplication::quit(); });

    connect(mHamburgerMenuButton, &QPushButton::clicked,
            [&]()
            {
                auto p = qobject_cast<QWidget*>(ui->toolBar->parent())->mapToGlobal(ui->toolBar->geometry().bottomRight());
                p.setX(p.x() - ui->menu_Hamburger->sizeHint().width());
                ui->menu_Hamburger->popup(p);
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
                        if (mProxyModelSources != nullptr)
                        {
                            mProxyModelSources->setDisplayMode(SortFilterProxyModelSources::SourceTreeDisplayMode::ShowAll);
                            restoreSourceTreeExpansionSelectionState(nullptr);
                            mItemModelSources->setHorizontalHeaderItem(0, new QStandardItem(tr("Sources & Feeds")));
                        }
                        break;
                    }
                    case StackedPaneLogs:
                    {
                        ui->frameFlagFilters->setVisible(false);
                        ui->tableViewScriptFolders->setVisible(false);
                        setUnreadBadgesShown(false);
                        if (mProxyModelSources != nullptr)
                        {
                            mProxyModelSources->setDisplayMode(SortFilterProxyModelSources::SourceTreeDisplayMode::ShowAll);
                            restoreSourceTreeExpansionSelectionState(nullptr);
                            mItemModelSources->setHorizontalHeaderItem(0, new QStandardItem(tr("Sources & Feeds")));
                        }
                        break;
                    }
                    case StackedPaneScripts:
                    {
                        ui->frameFlagFilters->setVisible(false);
                        ui->tableViewScriptFolders->setVisible(false);
                        setUnreadBadgesShown(false);
                        if (mProxyModelSources != nullptr)
                        {
                            preserveSourceTreeExpansionSelectionState();
                            mProxyModelSources->setDisplayMode(SortFilterProxyModelSources::SourceTreeDisplayMode::ShowSourcesOnly);
                            mItemModelSources->setHorizontalHeaderItem(0, new QStandardItem(tr("Sources")));
                        }
                        break;
                    }
                    case StackedPaneProperties:
                    {
                        ui->frameFlagFilters->setVisible(false);
                        ui->tableViewScriptFolders->setVisible(false);
                        setUnreadBadgesShown(false);
                        if (mProxyModelSources != nullptr)
                        {
                            mProxyModelSources->setDisplayMode(SortFilterProxyModelSources::SourceTreeDisplayMode::ShowAll);
                            restoreSourceTreeExpansionSelectionState(nullptr);
                            mItemModelSources->setHorizontalHeaderItem(0, new QStandardItem(tr("Sources & Feeds")));
                        }
                        break;
                    }
                }
            });

    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
            [&](Qt::ColorScheme /*scheme*/)
            {
                reloadCurrentPost();
                configureIcons();
            });

    connectSourceStuff();
    connectPostStuff();
    connectFeedStuff();
    connectFolderStuff();
    connectLogsStuff();
    connectFlagStuff();
    connectScriptStuff();
    connectScriptFolderStuff();
    connectPropertiesStuff();
}
