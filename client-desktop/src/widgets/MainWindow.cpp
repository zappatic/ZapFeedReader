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
#include "ZapFR/AutoRefresh.h"
#include "ZapFR/Database.h"
#include "ZapFR/Flag.h"
#include "ZapFR/Log.h"
#include "ZapFR/base/Folder.h"
#include "ZapFR/base/Post.h"
#include "ZapFR/local/FeedLocal.h"
#include "ZapFR/local/ScriptLocal.h"
#include "dialogs/DialogAddFeed.h"
#include "dialogs/DialogAddFolder.h"
#include "dialogs/DialogAddSource.h"
#include "dialogs/DialogEditFolder.h"
#include "dialogs/DialogEditScript.h"
#include "dialogs/DialogEditScriptFolder.h"
#include "dialogs/DialogImportOPML.h"
#include "dialogs/DialogJumpToPage.h"
#include "dialogs/DialogPreferences.h"
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

    // clang-format off
    // generated with MainWindow::dumpPalette()
    static const std::string gsThemeValues{R"({"dark":{"0":{"0":"#ffffff","1":"#2a2a2a","2":"#343434","3":"#e9e7e3","4":"#222222","5":"#a0a0a4","6":"#ffffff","7":"#ffffff","8":"#ffffff","9":"#2a2a2a","10":"#2a2a2a","11":"#1e1e1e","12":"#e95420","13":"#ffffff","14":"#308cc6","15":"#ff00ff","16":"#272727","18":"#ffffdc","19":"#f7f7f7","20":"#9b9b9b"},"1":{"0":"#7f7f7f","1":"#d4d0c8","2":"#ffffff","3":"#e9e7e3","4":"#6a6864","5":"#a0a0a4","6":"#7f7f7f","7":"#ffffff","8":"#ffffff","9":"#d4d0c8","10":"#d4d0c8","11":"#000000","12":"#e95420","13":"#ffffff","14":"#308cc6","15":"#ff00ff","16":"#272727","18":"#ffffdc","19":"#f7f7f7","20":"#9b9b9b"},"2":{"0":"#ffffff","1":"#d4d0c8","2":"#ffffff","3":"#e9e7e3","4":"#6a6864","5":"#a0a0a4","6":"#ffffff","7":"#ffffff","8":"#7f7f7f","9":"#2a2a2a","10":"#2a2a2a","11":"#000000","12":"#308cc6","13":"#343434","14":"#308cc6","15":"#ff00ff","16":"#272727","18":"#ffffdc","19":"#f7f7f7","20":"#9b9b9b"}},"light":{"0":{"0":"#636363","1":"#fcfcfc","2":"#ffffff","3":"#e9e7e3","4":"#cacaca","5":"#a0a0a4","6":"#000000","7":"#ffffff","8":"#000000","9":"#fcfcfc","10":"#fcfcfc","11":"#b1b1b1","12":"#e95420","13":"#ffffff","14":"#0000ff","15":"#ff00ff","16":"#ececec","18":"#ffffdc","19":"#3d3d3d","20":"#646464"},"1":{"0":"#313131","1":"#d4d0c8","2":"#ffffff","3":"#e9e7e3","4":"#6a6864","5":"#a0a0a4","6":"#313131","7":"#ffffff","8":"#000000","9":"#d4d0c8","10":"#d4d0c8","11":"#000000","12":"#e95420","13":"#000000","14":"#0000ff","15":"#ff00ff","16":"#ececec","18":"#ffffdc","19":"#3d3d3d","20":"#646464"},"2":{"0":"#636363","1":"#d4d0c8","2":"#ffffff","3":"#e9e7e3","4":"#6a6864","5":"#a0a0a4","6":"#000000","7":"#ffffff","8":"#313131","9":"#fcfcfc","10":"#fcfcfc","11":"#000000","12":"#308cc6","13":"#fafafa","14":"#0000ff","15":"#ff00ff","16":"#ececec","18":"#ffffdc","19":"#3d3d3d","20":"#646464"}}})"};
    // clang-format on
} // namespace

ZapFR::Client::MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
#ifdef ZFR_DUMP_PALETTE
    dumpPalette();
#endif
    ZapFR::Engine::FeedLocal::setIconDir(QDir::cleanPath(dataDir() + QDir::separator() + "icons").toStdString());

    ZapFR::Engine::Database::getInstance()->initialize(QDir::cleanPath(dataDir() + QDir::separator() + "zapfeedreader.db").toStdString(),
                                                       ZapFR::Engine::ApplicationType::Client);

    ZapFR::Engine::Agent::getInstance()->registerErrorCallback([&](uint64_t sourceID, const std::string& errorMessage)
                                                               { QMetaObject::invokeMethod(this, [&]() { agentErrorOccurred(sourceID, errorMessage); }); });

    ZapFR::Engine::AutoRefresh::getInstance()->setFeedRefreshedCallback(
        [&](uint64_t sourceID, ZapFR::Engine::Feed* refreshedFeed)
        {
            auto id = refreshedFeed->id();
            auto unreadCount = refreshedFeed->unreadCount();
            auto error = refreshedFeed->lastRefreshError();
            auto title = refreshedFeed->title();
            auto iconHash = refreshedFeed->iconHash();
            auto iconData = refreshedFeed->iconData();
            QMetaObject::invokeMethod(this, [&]() { feedRefreshed(sourceID, id, unreadCount, error.has_value() ? error.value() : "", title, iconHash, iconData); });
        });

    ui->setupUi(this);
    initializeUI();
    configureConnects();
    createContextMenus();
    configureIcons();
    reloadSources();
    restoreSettings();
    reloadCurrentPost();
    updateActivePostFilter();
}

ZapFR::Client::MainWindow::~MainWindow()
{
    delete ui;
}

void ZapFR::Client::MainWindow::initializeUI()
{
    initializeUISources();
    initializeUIPosts();
    initializeUILogs();
    initializeUIScripts();
    initializeUIScriptFolders();
    updatePreferredFontSize();

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

    ui->stackedWidgetContentPanes->setCurrentIndex(StackedPanePosts);
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
    root.insert(SETTING_SPLITTERSOURCESANDCONTENTPANES_STATE, QString::fromUtf8(ui->splitterSourcesAndContentPanes->saveState().toBase64()));
    root.insert(SETTING_SPLITTERSOURCESANDSCRIPTFOLDERS_STATE, QString::fromUtf8(ui->splitterSourcesAndScriptFolders->saveState().toBase64()));
    root.insert(SETTING_SPLITTERPOSTSTABLEANDPOSTVIEW_STATE, QString::fromUtf8(ui->splitterPostsTableAndPostView->saveState().toBase64()));
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
    if (mPreferenceTheme != Theme::UseSystem)
    {
        root.insert(SETTING_UI_THEME, mPreferenceTheme == Theme::Light ? "light" : "dark");
    }
    root.insert(SETTING_UI_FONTSIZE, mPreferenceUIFontSize);
    root.insert(SETTING_POST_FONTSIZE, mPreferencePostFontSize);
    root.insert(SETTING_FEEDS_REFRESH_BEHAVIOUR, mPreferenceRefreshBehaviour == RefreshBehaviour::EntireSource ? "entiresource" : "currentselection");
    auto ar = ZapFR::Engine::AutoRefresh::getInstance();
    root.insert(SETTING_FEEDS_AUTOREFRESH_INTERVAL, static_cast<int32_t>(ar->feedRefreshInterval()));
    root.insert(SETTING_FEEDS_AUTOREFRESH_ENABLED, ar->isEnabled());

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
                if (root.contains(SETTING_SPLITTERSOURCESANDCONTENTPANES_STATE))
                {
                    ui->splitterSourcesAndContentPanes->restoreState(
                        QByteArray::fromBase64(root.value(SETTING_SPLITTERSOURCESANDCONTENTPANES_STATE).toVariant().toByteArray()));
                }
                if (root.contains(SETTING_SPLITTERSOURCESANDSCRIPTFOLDERS_STATE))
                {
                    ui->splitterSourcesAndScriptFolders->restoreState(
                        QByteArray::fromBase64(root.value(SETTING_SPLITTERSOURCESANDSCRIPTFOLDERS_STATE).toVariant().toByteArray()));
                }
                if (root.contains(SETTING_SPLITTERPOSTSTABLEANDPOSTVIEW_STATE))
                {
                    ui->splitterPostsTableAndPostView->restoreState(QByteArray::fromBase64(root.value(SETTING_SPLITTERPOSTSTABLEANDPOSTVIEW_STATE).toVariant().toByteArray()));
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
                if (root.contains(SETTING_UI_THEME))
                {
                    auto value = root.value(SETTING_UI_THEME);
                    mPreferenceTheme = (value == "light" ? Theme::Light : Theme::Dark);
                    applyColorScheme();
                }
                if (root.contains(SETTING_UI_FONTSIZE))
                {
                    mPreferenceUIFontSize = static_cast<uint16_t>(root.value(SETTING_UI_FONTSIZE).toInt(11));
                    updatePreferredFontSize();
                }
                if (root.contains(SETTING_POST_FONTSIZE))
                {
                    mPreferencePostFontSize = static_cast<uint16_t>(root.value(SETTING_POST_FONTSIZE).toInt(11));
                }
                if (root.contains(SETTING_FEEDS_REFRESH_BEHAVIOUR))
                {
                    mPreferenceRefreshBehaviour =
                        (root.value(SETTING_FEEDS_REFRESH_BEHAVIOUR).toString() == "entiresource" ? RefreshBehaviour::EntireSource : RefreshBehaviour::CurrentSelection);
                }

                auto enableAutoRefresh{true};
                if (root.contains(SETTING_FEEDS_AUTOREFRESH_ENABLED))
                {
                    enableAutoRefresh = root.value(SETTING_FEEDS_AUTOREFRESH_ENABLED).toBool();
                }
                ZapFR::Engine::AutoRefresh::getInstance()->setEnabled(enableAutoRefresh);

                uint64_t autoRefreshInterval{ZapFR::Engine::DefaultFeedAutoRefreshInterval};
                if (root.contains(SETTING_FEEDS_AUTOREFRESH_INTERVAL))
                {
                    autoRefreshInterval = static_cast<uint64_t>(root.value(SETTING_FEEDS_AUTOREFRESH_INTERVAL).toInt(ZapFR::Engine::DefaultFeedAutoRefreshInterval));
                }
                ZapFR::Engine::AutoRefresh::getInstance()->setFeedRefreshInterval(autoRefreshInterval);
            }
        }
    }
    catch (...)
    {
    }
}

#ifdef ZFR_DUMP_PALETTE
void ZapFR::Client::MainWindow::dumpPalette()
{
    std::array<QPalette::ColorGroup, 3> colorGroups{QPalette::Disabled, QPalette::Active, QPalette::Inactive};
    std::array<QString, 3> colorGroupNames{"QPalette::Disabled", "QPalette::Active", "QPalette::Inactive"};

    std::array<QPalette::ColorRole, 20> colorRoles{
        QPalette::Window, QPalette::WindowText, QPalette::Base,       QPalette::AlternateBase,   QPalette::ToolTipBase, QPalette::ToolTipText, QPalette::PlaceholderText,
        QPalette::Text,   QPalette::Button,     QPalette::ButtonText, QPalette::BrightText,      QPalette::Light,       QPalette::Midlight,    QPalette::Dark,
        QPalette::Mid,    QPalette::Shadow,     QPalette::Highlight,  QPalette::HighlightedText, QPalette::Link,        QPalette::LinkVisited,
    };
    std::array<QString, 20> colorRoleNames{
        "QPalette::Window",
        "QPalette::WindowText",
        "QPalette::Base",
        "QPalette::AlternateBase",
        "QPalette::ToolTipBase",
        "QPalette::ToolTipText",
        "QPalette::PlaceholderText",
        "QPalette::Text",
        "QPalette::Button",
        "QPalette::ButtonText",
        "QPalette::BrightText",
        "QPalette::Light",
        "QPalette::Midlight",
        "QPalette::Dark",
        "QPalette::Mid",
        "QPalette::Shadow",
        "QPalette::Highlight",
        "QPalette::HighlightedText",
        "QPalette::Link",
        "QPalette::LinkVisited",
    };

    auto palette = QGuiApplication::palette();
    Poco::JSON::Object o;
    for (const auto& colorGroup : colorGroups)
    {
        Poco::JSON::Object p;
        for (const auto& colorRole : colorRoles)
        {
            p.set(std::to_string(static_cast<std::underlying_type_t<QPalette::ColorRole>>(colorRole)), palette.color(colorGroup, colorRole).name().toStdString());
        }
        o.set(std::to_string(static_cast<std::underlying_type_t<QPalette::ColorGroup>>(colorGroup)), p);
    }
    Poco::JSON::Stringifier::stringify(o, std::cout);
    std::cout << std::endl;

    QString theme = QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark ? "Dark" : "Light";

    int32_t imageWidth{900}, imageHeight{600};
    QImage img({imageWidth, imageHeight}, QImage::Format::Format_RGB32);
    QPainter painter(&img);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(QRect(0, 0, imageWidth, imageHeight), Qt::white);

    int32_t marginTop{40};
    int32_t marginLeft{20};
    int32_t marginRow{5};
    int32_t marginCol{5};
    int32_t rowHeight{20};
    int32_t swatchTableYStart{marginTop + 30};
    int32_t colorRoleNameColumnWidth{185};
    int32_t swatchWidth{100};
    int32_t swatchColorNameWidth{125};
    int32_t x{marginLeft}, y{marginTop};

    auto defaultFont = painter.font();
    auto headerFont = defaultFont;
    headerFont.setPointSize(18);
    auto colorNameFont = defaultFont;
    colorNameFont.setFamily("Courier");

    painter.setFont(headerFont);
    painter.drawText(QPoint(x, y), theme);
    y = swatchTableYStart;

    for (size_t col = 0; col < colorGroups.size(); ++col)
    {
        auto columnX = marginLeft + colorRoleNameColumnWidth + (marginCol * 2) + (static_cast<int32_t>(col) * (swatchWidth + swatchColorNameWidth + marginCol));
        painter.setFont(defaultFont);
        painter.drawText(QPoint(columnX, y), colorGroupNames.at(col));
        y += marginRow;

        for (size_t row = 0; row < colorRoles.size(); ++row)
        {
            if (col == 0)
            {
                painter.setFont(defaultFont);
                painter.drawText(QRect(x, y, colorRoleNameColumnWidth, rowHeight), Qt::AlignRight | Qt::AlignVCenter, colorRoleNames.at(row));
            }
            x = columnX;

            auto color = palette.color(colorGroups.at(col), colorRoles.at(row));
            auto swatchRect = QRect(x, y, swatchWidth, rowHeight);
            painter.setPen(Qt::black);
            painter.fillRect(swatchRect, color);
            painter.drawRect(swatchRect);
            painter.setFont(colorNameFont);
            painter.drawText(QRect(x + swatchWidth + marginCol, y, swatchColorNameWidth, rowHeight), Qt::AlignLeft | Qt::AlignVCenter, color.name());

            y += rowHeight + marginRow;
            x = marginLeft;
        }
        y = swatchTableYStart;
    }

    painter.end();
    img.save(QString("%1.jpg").arg(theme));
}
#endif

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
                        ZapFR::Engine::Agent::getInstance()->queueImportOPML(
                            mDialogImportOPML->selectedSourceID(), mDialogImportOPML->OPML(), mDialogImportOPML->selectedFolderID(),
                            [&]() { QMetaObject::invokeMethod(this, [&]() { reloadSources(); }); },
                            [&](uint64_t affectedSourceID, ZapFR::Engine::Feed* refreshedFeed)
                            {
                                auto id = refreshedFeed->id();
                                auto unreadCount = refreshedFeed->unreadCount();
                                auto error = refreshedFeed->lastRefreshError();
                                auto title = refreshedFeed->title();
                                auto iconHash = refreshedFeed->iconHash();
                                auto iconData = refreshedFeed->iconData();
                                QMetaObject::invokeMethod(
                                    this, [&]() { feedRefreshed(affectedSourceID, id, unreadCount, error.has_value() ? error.value() : "", title, iconHash, iconData); });
                            });

                        // give it a bit of time to parse the OPML file, then start checking whether the refreshing has completed
                        QTimer::singleShot(2500,
                                           [&]() {
                                               ZapFR::Engine::Agent::getInstance()->queueMonitorFeedRefreshCompletion(
                                                   [&]() { QMetaObject::invokeMethod(this, [&]() { reloadSources(); }); });
                                           });
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

void ZapFR::Client::MainWindow::updatePreferredFontSize()
{
    auto font = ui->treeViewSources->font();
    font.setPointSizeF(mPreferenceUIFontSize);
    ui->treeViewSources->setFont(font);

    auto tableViewRowHeight = static_cast<int32_t>(std::ceil(static_cast<float>(mPreferenceUIFontSize) * 1.333f) + 8);

    font = ui->tableViewPosts->font();
    font.setPointSizeF(mPreferenceUIFontSize);
    ui->tableViewPosts->setFont(font);
    ui->tableViewPosts->verticalHeader()->setDefaultSectionSize(tableViewRowHeight);

    font = ui->tableViewPostEnclosures->font();
    font.setPointSizeF(mPreferenceUIFontSize);
    ui->tableViewPostEnclosures->setFont(font);
    ui->tableViewPostEnclosures->verticalHeader()->setDefaultSectionSize(tableViewRowHeight);

    font = ui->tableViewScriptFolders->font();
    font.setPointSizeF(mPreferenceUIFontSize);
    ui->tableViewScriptFolders->setFont(font);
    ui->tableViewScriptFolders->verticalHeader()->setDefaultSectionSize(tableViewRowHeight);

    font = ui->tableViewLogs->font();
    font.setPointSizeF(mPreferenceUIFontSize);
    ui->tableViewLogs->setFont(font);
    ui->tableViewLogs->verticalHeader()->setDefaultSectionSize(tableViewRowHeight);

    font = ui->tableViewScripts->font();
    font.setPointSizeF(mPreferenceUIFontSize);
    ui->tableViewScripts->setFont(font);
    ui->tableViewScripts->verticalHeader()->setDefaultSectionSize(tableViewRowHeight);

    mPostStylesCacheValid = false;
}

void ZapFR::Client::MainWindow::configureIcons()
{
    auto theme = getCurrentColorTheme();

    // the defaults are for the light theme
    auto color = QString("#000");
    auto colorDisabled = QString("#aaa");
    auto colorBorder = QString("#cacaca");
    if (theme == Theme::Dark)
    {
        color = "#fff";
        colorDisabled = "#555";
        colorBorder = "#222222";
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

    ui->action_Refresh->setIcon(configureIcon(":/refreshFeed.svg"));
    ui->action_Toolbar_refresh->setIcon(configureIcon(":/refreshFeed.svg"));
    ui->action_Reconnect_to_source->setIcon(configureIcon(":/refreshFeed.svg"));
    ui->action_Mark_as_read->setIcon(configureIcon(":/markAsRead.svg"));
    ui->action_Add_feed->setIcon(configureIcon(":/addFeed.svg"));
    ui->action_Add_source->setIcon(configureIcon(":/addFeed.svg"));
    ui->action_Add_folder->setIcon(configureIcon(":/addFolder.svg"));
    ui->action_Edit_folder->setIcon(configureIcon(":/edit.svg"));
    ui->action_View_logs->setIcon(configureIcon(":/viewLogs.svg"));
    ui->action_Clear_logs->setIcon(configureIcon(":/remove.svg"));
    ui->action_View_scripts->setIcon(configureIcon(":/script.svg"));
    ui->action_Back_to_posts->setIcon(configureIcon(":/back.svg"));
    ui->action_Edit_script->setIcon(configureIcon(":/edit.svg"));
    ui->action_Remove_script->setIcon(configureIcon(":/remove.svg"));
    ui->action_Add_script->setIcon(configureIcon(":/addFeed.svg"));
    ui->action_Edit_script_folder->setIcon(configureIcon(":/edit.svg"));
    ui->action_Remove_script_folder->setIcon(configureIcon(":/remove.svg"));
    ui->action_Add_script_folder->setIcon(configureIcon(":/addFeed.svg"));
    ui->action_View_properties->setIcon(configureIcon(":/properties.svg"));
    ui->action_Remove_feed->setIcon(configureIcon(":/remove.svg"));
    ui->action_Remove_folder->setIcon(configureIcon(":/remove.svg"));
    ui->action_Remove_source->setIcon(configureIcon(":/remove.svg"));
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

    ui->frameFlagFilters->setStyleSheet(QString("QFrame { border-top: 0px; border-left: 0px; border-right: 1px solid %1; border-bottom: 1px solid %1;}").arg(colorBorder));

    mLineEditSearch->setSearchIconColor(theme == Theme::Dark ? "#eee" : "#333");
}

void ZapFR::Client::MainWindow::applyColorScheme()
{
    static std::optional<std::unique_ptr<QPalette>> cacheLightTheme{};
    static std::optional<std::unique_ptr<QPalette>> cacheDarkTheme{};

    auto parseThemeValues = [&](const std::string& theme) -> std::unique_ptr<QPalette>
    {
        auto themeValues = gsThemeValues;
        auto overrideBuggedValues{true};

        auto override = QFile(QDir::cleanPath(configDir() + QDir::separator() + "theme.json"));
        if (override.exists())
        {
            override.open(QIODeviceBase::ReadOnly);
            themeValues = QString::fromUtf8(override.readAll()).toStdString();
            override.close();
            overrideBuggedValues = false;
        }

        auto palette = std::make_unique<QPalette>();
        try
        {
            Poco::JSON::Parser parser;
            auto root = parser.parse(themeValues);
            auto themes = root.extract<Poco::JSON::Object::Ptr>();
            if (!themes->has(theme))
            {
                return std::make_unique<QPalette>();
            }
            auto colorGroupsObj = themes->getObject(theme);

            for (const auto& colorGroupStr : colorGroupsObj->getNames())
            {
                auto colorGroup = static_cast<QPalette::ColorGroup>(Poco::NumberParser::parseUnsigned(colorGroupStr));

                auto colorRolesObj = colorGroupsObj->getObject(colorGroupStr);
                for (const auto& colorRoleStr : colorRolesObj->getNames())
                {
                    auto colorRole = static_cast<QPalette::ColorRole>(Poco::NumberParser::parseUnsigned(colorRoleStr));
                    auto colorValue = QColor(colorRolesObj->getValue<std::string>(colorRoleStr).c_str());

                    // don't set the highlight color to the predetermined color, as it may be set as an accent color in the OS
                    if (colorRole == QPalette::Highlight)
                    {
                        continue;
                    }

                    palette->setColor(colorGroup, colorRole, colorValue);
                }
            }

            if (overrideBuggedValues)
            {
                // explicitly overwrite these colors, as the inactive/disabled ones seem to be wrong by default :/
                palette->setColor(QPalette::Inactive, QPalette::Highlight, palette->color(QPalette::Active, QPalette::Highlight));
                palette->setColor(QPalette::Inactive, QPalette::HighlightedText, palette->color(QPalette::Active, QPalette::HighlightedText));
                palette->setColor(QPalette::Inactive, QPalette::Button, palette->color(QPalette::Active, QPalette::Button));
                palette->setColor(QPalette::Inactive, QPalette::ButtonText, palette->color(QPalette::Active, QPalette::ButtonText));
                palette->setColor(QPalette::Disabled, QPalette::AlternateBase, palette->color(QPalette::Active, QPalette::AlternateBase));
                palette->setColor(QPalette::Disabled, QPalette::Base, palette->color(QPalette::Active, QPalette::Base));
                palette->setColor(QPalette::Disabled, QPalette::Button, palette->color(QPalette::Active, QPalette::Button));
                palette->setColor(QPalette::Disabled, QPalette::Window, palette->color(QPalette::Active, QPalette::Window));
                // ToolTipText in dark mode seems to be switched to white, but the ToolTipBase is the same light yellow, making it unreadable
                // TODO: this doesn't seem to work though, as this color is not picked up by the tooltips, so (temporarily?) fixed by subclassing
                // treeview, so that at least the error message on a feed is visible
                palette->setColor(QPalette::ToolTipText, Qt::blue);
            }
        }
        catch (const Poco::Exception& e)
        {
            std::cerr << "Could not parse theme values: " << e.displayText() << "\n";
        }
        catch (...)
        {
            std::cerr << "Could not parse theme values\n";
        }

        return palette;
    };

    auto theme = getCurrentColorTheme();

    std::optional<QPalette*> paletteToEnforce{};
    switch (theme)
    {
        case Theme::Light:
        {
            if (!cacheLightTheme.has_value())
            {
                cacheLightTheme = parseThemeValues("light");
            }
            paletteToEnforce = cacheLightTheme.value().get();
            break;
        }
        case Theme::Dark:
        {
            if (!cacheDarkTheme.has_value())
            {
                cacheDarkTheme = parseThemeValues("dark");
            }
            paletteToEnforce = cacheDarkTheme.value().get();
            break;
        }
        case Theme::UseSystem:
        {
            break;
        }
    }

    if (paletteToEnforce.has_value())
    {
        QGuiApplication::setPalette(*(paletteToEnforce.value()));
    }

    mPostStylesCacheValid = false;
    reloadCurrentPost();
    configureIcons();
}

void ZapFR::Client::MainWindow::updateToolbar()
{
    // hide all actions from toolbar (and applicable ones from hamburger menu)
    for (const auto& action : ui->toolBar->actions())
    {
        if (!action->property(gsHamburgerMenuButton).isValid())
        {
            action->setVisible(false);
        }
    }
    ui->action_Add_source->setVisible(false);
    ui->action_Remove_source->setVisible(false);
    ui->action_Import_OPML->setVisible(false);
    ui->action_Export_OPML->setVisible(false);

    switch (ui->stackedWidgetContentPanes->currentIndex())
    {
        case StackedPanePosts:
        {
            bool anythingSelected{false};
            QString markAsReadCaption;
            QString refreshCaption;
            QString refreshToolbarCaption;
            auto parentSourceHasError{false};
            auto showSearchField{true};

            auto index = ui->treeViewSources->currentIndex();
            if (index.isValid())
            {
                anythingSelected = true;

                auto type = index.data(SourceTreeEntryTypeRole).toULongLong();
                parentSourceHasError = doesSourceHaveError(index.data(SourceTreeEntryParentSourceIDRole).toULongLong());
                switch (type)
                {
                    case SOURCETREE_ENTRY_TYPE_FEED:
                    {
                        markAsReadCaption = tr("Mark feed as read");
                        refreshToolbarCaption = (mPreferenceRefreshBehaviour == RefreshBehaviour::CurrentSelection ? tr("Refresh feed") : tr("Refresh source"));
                        refreshCaption = tr("Refresh feed");
                        break;
                    }
                    case SOURCETREE_ENTRY_TYPE_FOLDER:
                    {
                        markAsReadCaption = tr("Mark folder as read");
                        refreshToolbarCaption = (mPreferenceRefreshBehaviour == RefreshBehaviour::CurrentSelection ? tr("Refresh folder") : tr("Refresh source"));
                        refreshCaption = tr("Refresh folder");
                        break;
                    }
                    case SOURCETREE_ENTRY_TYPE_SOURCE:
                    {
                        markAsReadCaption = tr("Mark source as read");
                        refreshToolbarCaption = tr("Refresh source");
                        refreshCaption = tr("Refresh source");
                        break;
                    }
                }
            }

            if (parentSourceHasError)
            {
                showSearchField = false;
                ui->action_Reconnect_to_source->setVisible(true);
            }
            else
            {
                ui->action_Add_feed->setVisible(true);
                ui->action_Add_folder->setVisible(true);
                ui->action_Refresh->setVisible(true);
                ui->action_Toolbar_refresh->setVisible(true);
                ui->action_Mark_as_read->setVisible(true);
                ui->action_View_logs->setVisible(true);
                ui->action_View_scripts->setVisible(true);
                ui->action_Add_source->setVisible(true);
                ui->action_Remove_source->setVisible(true);
                ui->action_Import_OPML->setVisible(true);
                ui->action_Export_OPML->setVisible(true);

                ui->action_Add_feed->setEnabled(anythingSelected);
                ui->action_Add_folder->setEnabled(anythingSelected);
                ui->action_Mark_as_read->setEnabled(anythingSelected);
                ui->action_Mark_as_read->setText(markAsReadCaption);
                ui->action_View_logs->setEnabled(anythingSelected);
                ui->action_Refresh->setEnabled(anythingSelected);
                ui->action_Refresh->setText(refreshCaption);
                ui->action_Toolbar_refresh->setEnabled(anythingSelected);
                ui->action_Toolbar_refresh->setText(refreshToolbarCaption);
            }

            for (const auto& action : ui->toolBar->actions())
            {
                if (action->property(gsPostPaneToolbarSpacerLeft).isValid() || (showSearchField && action->property(gsPostPaneLineEditSearch).isValid()))
                {
                    action->setVisible(true);
                }
            }
            break;
        }
        case StackedPaneLogs:
        {
            ui->action_Back_to_posts->setVisible(true);
            ui->action_Clear_logs->setVisible(true);
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

void ZapFR::Client::MainWindow::showPreferences()
{
    if (mDialogPreferences == nullptr)
    {
        mDialogPreferences = std::make_unique<DialogPreferences>(this);
        connect(mDialogPreferences.get(), &QDialog::finished,
                [&](int result)
                {
                    if (result == QDialog::DialogCode::Accepted)
                    {
                        mPreferenceTheme = mDialogPreferences->chosenTheme();
                        applyColorScheme();

                        mPreferenceUIFontSize = mDialogPreferences->chosenUIFontSize();
                        updatePreferredFontSize();

                        mPreferencePostFontSize = mDialogPreferences->chosenPostFontSize();
                        reloadCurrentPost();

                        mPreferenceRefreshBehaviour = mDialogPreferences->chosenRefreshBehaviour();
                        updateToolbar();

                        auto ar = ZapFR::Engine::AutoRefresh::getInstance();
                        ar->setEnabled(mDialogPreferences->chosenAutoRefreshEnabled());
                        ar->setFeedRefreshInterval(mDialogPreferences->chosenAutoRefreshInterval());

                        saveSettings();
                    }
                });
    }
    mDialogPreferences->reset();
    mDialogPreferences->open();
}

void ZapFR::Client::MainWindow::agentErrorOccurred(uint64_t sourceID, const std::string& errorMessage)
{
    auto rootItem = mItemModelSources->invisibleRootItem();
    for (auto i = 0; i < rootItem->rowCount(); ++i)
    {
        auto child = rootItem->child(i);
        auto type = child->data(SourceTreeEntryTypeRole).toULongLong();
        auto id = child->data(SourceTreeEntryIDRole).toULongLong();
        if (type == SOURCETREE_ENTRY_TYPE_SOURCE && id == sourceID)
        {
            child->setData(QString::fromUtf8(errorMessage), SourceTreeEntryErrorRole);
            child->setData(QString::fromUtf8(errorMessage), Qt::ToolTipRole);
            break;
        }
    }
}

ZapFR::Client::Theme ZapFR::Client::MainWindow::getCurrentColorTheme() const
{
    auto currentColorTheme = mPreferenceTheme;

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (currentColorTheme == Theme::UseSystem)
    {
        switch (QGuiApplication::styleHints()->colorScheme())
        {
            case Qt::ColorScheme::Light:
            case Qt::ColorScheme::Unknown:
            {
                currentColorTheme = Theme::Light;
                break;
            }
            case Qt::ColorScheme::Dark:
            {
                currentColorTheme = Theme::Dark;
                break;
            }
        }
    }
#endif

    if (currentColorTheme == Theme::UseSystem) // this only happens when the pref is on useSystem and Qt < 6.5 (so no colorScheme function)
    {
        currentColorTheme = Theme::Light;
    }

    return currentColorTheme;
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
    connect(ui->action_Show_preferences, &QAction::triggered, this, &MainWindow::showPreferences);

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, &MainWindow::applyColorScheme);
#endif

    connect(mHamburgerMenuButton, &QPushButton::clicked,
            [&]()
            {
                auto p = qobject_cast<QWidget*>(ui->toolBar->parent())->mapToGlobal(ui->toolBar->geometry().bottomRight());
                p.setX(p.x() - ui->menu_Hamburger->sizeHint().width());
                ui->menu_Hamburger->popup(p);
            });

    connect(ui->stackedWidgetContentPanes, &QStackedWidget::currentChanged,
            [&]()
            {
                updateToolbar();
                switch (ui->stackedWidgetContentPanes->currentIndex())
                {
                    case StackedPanePosts:
                    {
                        ui->frameFlagFilters->setVisible(true);
                        ui->tableViewScriptFolders->setVisible(true);
                        mItemModelSources->setAllowDragAndDrop(true);
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
                        mItemModelSources->setAllowDragAndDrop(false);
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
                        mItemModelSources->setAllowDragAndDrop(false);
                        setUnreadBadgesShown(false);
                        if (mProxyModelSources != nullptr)
                        {
                            preserveSourceTreeExpansionSelectionState();
                            uint64_t currentParentSource{0};
                            auto index = ui->treeViewSources->currentIndex();
                            if (index.isValid())
                            {
                                currentParentSource = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
                            }

                            mProxyModelSources->setDisplayMode(SortFilterProxyModelSources::SourceTreeDisplayMode::ShowSourcesOnly);
                            mItemModelSources->setHorizontalHeaderItem(0, new QStandardItem(tr("Sources")));

                            // auto select the source for the item that was currently selected, so the selection isn't empty after
                            // only showing sources
                            if (currentParentSource != 0)
                            {
                                auto rootItem = mItemModelSources->invisibleRootItem();
                                for (int32_t i = 0; i < rootItem->rowCount(); ++i)
                                {
                                    auto child = rootItem->child(i);
                                    if (child->data(SourceTreeEntryParentSourceIDRole).toULongLong() == currentParentSource)
                                    {
                                        ui->treeViewSources->setCurrentIndex(mProxyModelSources->mapFromSource(mItemModelSources->indexFromItem(child)));
                                        break;
                                    }
                                }
                            }
                        }
                        break;
                    }
                    case StackedPaneProperties:
                    {
                        ui->frameFlagFilters->setVisible(false);
                        ui->tableViewScriptFolders->setVisible(false);
                        mItemModelSources->setAllowDragAndDrop(false);
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
