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

#include <Poco/JSON/Parser.h>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QStyleHints>

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
#include "dialogs/DialogAddSource.h"
#include "dialogs/DialogEditScript.h"
#include "dialogs/DialogEditScriptFolder.h"
#include "dialogs/DialogJumpToPage.h"
#include "dialogs/DialogPreferences.h"
#include "models/SortFilterProxyModelSources.h"
#include "models/StandardItemModelSources.h"
#include "widgets/LineEditSearch.h"
#include "widgets/MainWindow.h"

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
    ui->setupUi(this);

#ifdef ZFR_DUMP_PALETTE
    dumpPalette();
#endif
    ZapFR::Engine::FeedLocal::setIconDir(QDir::cleanPath(dataDir() + QDir::separator() + "icons").toStdString());

    ZapFR::Engine::Database::getInstance()->initialize(QDir::cleanPath(dataDir() + QDir::separator() + "zapfeedreader.db").toStdString(),
                                                       ZapFR::Engine::ApplicationType::Client);

    ZapFR::Engine::Agent::getInstance()->registerErrorCallback(
        [&](uint64_t sourceID, const std::string& errorMessage)
        { QMetaObject::invokeMethod(this, [=, this]() { ui->treeViewSources->agentErrorOccurred(sourceID, errorMessage); }); });

    ZapFR::Engine::AutoRefresh::getInstance()->setFeedRefreshedCallback(
        [&](uint64_t sourceID, ZapFR::Engine::Feed* refreshedFeed)
        {
            auto id = refreshedFeed->id();
            auto unreadCount = refreshedFeed->unreadCount();
            auto error = refreshedFeed->lastRefreshError();
            auto title = refreshedFeed->title();
            auto iconHash = refreshedFeed->iconHash();
            auto iconData = refreshedFeed->iconData();
            QMetaObject::invokeMethod(this, [=, this]()
                                      { ui->treeViewSources->feedRefreshed(sourceID, id, unreadCount, error.has_value() ? error.value() : "", title, iconHash, iconData); });
        });

    mPreferences = std::make_unique<Preferences>();
    initializeUI();
    connectStuff();
    configureIcons();
    restoreSettings();
    ui->treeViewSources->reload();
    ui->tableViewPosts->reloadCurrentPost();
    ui->tableViewPosts->updateActivePostFilter();
}

ZapFR::Client::MainWindow::~MainWindow()
{
    delete ui;
}

void ZapFR::Client::MainWindow::initializeUI()
{
    updatePreferredFontSize();

    ui->treeViewSources->setMainWindow(this);
    ui->tableViewPosts->setMainWindow(this);
    ui->tableViewLogs->setMainWindow(this);
    ui->tableViewScriptFolders->setMainWindow(this);
    ui->tableViewScripts->setMainWindow(this);
    ui->frameFlagFilters->setMainWindow(this);
    ui->webViewPost->setMainWindow(this);
    ui->tableViewPostEnclosures->setMainWindow(this);

    mActionShowPreferences = std::make_unique<QAction>(tr("Preferences"), this);
    mActionExit = std::make_unique<QAction>(tr("Exit"), this);
    mActionBackToPosts = std::make_unique<QAction>(tr("Back to posts"), this);

    ui->menubar->setVisible(false);
    ui->stackedWidgetContentPanes->setCurrentIndex(ContentPane::Posts);

    // prevent the left splitter from resizing while the window resizes
    ui->splitterSourcesAndContentPanes->setStretchFactor(1, 100);

    // add the hamburger menu button to the toolbar
    mHamburgerMenuButton = std::make_unique<QPushButton>();
    mHamburgerMenuButton->setFlat(true);
    mHamburgerMenuButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto actionHamburgerMenu = ui->toolBar->insertWidget(nullptr, mHamburgerMenuButton.get());
    actionHamburgerMenu->setProperty(gsHamburgerMenuButton, true);

    ui->toolBar->insertAction(actionHamburgerMenu, ui->treeViewSources->actionAddFeed());
    ui->toolBar->insertAction(actionHamburgerMenu, ui->treeViewSources->actionAddFolder());
    ui->toolBar->insertAction(actionHamburgerMenu, ui->treeViewSources->actionToolbarRefresh());
    ui->toolBar->insertAction(actionHamburgerMenu, ui->tableViewPosts->actionMarkAsRead());

    ui->toolBar->insertAction(actionHamburgerMenu, ui->treeViewSources->actionReconnectToSource());
    ui->toolBar->insertAction(actionHamburgerMenu, mActionBackToPosts.get());

    ui->toolBar->insertAction(actionHamburgerMenu, ui->tableViewScripts->actionAddScript());
    ui->toolBar->insertAction(actionHamburgerMenu, ui->tableViewScripts->actionEditScript());
    ui->toolBar->insertAction(actionHamburgerMenu, ui->tableViewScripts->actionRemoveScript());

    ui->toolBar->insertAction(actionHamburgerMenu, ui->tableViewLogs->actionClearLogs());

    // add a spacer in the toolbar to separate the left from the right buttons
    mToolbarSpacerLeft = std::make_unique<QWidget>();
    mToolbarSpacerLeft->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto actionSpacerLeft = ui->toolBar->insertWidget(actionHamburgerMenu, mToolbarSpacerLeft.get());
    actionSpacerLeft->setProperty(gsPostPaneToolbarSpacerLeft, true);

    // add the search widget to the toolbar
    mLineEditSearch = std::make_unique<LineEditSearch>();
    auto actionSearch = ui->toolBar->insertWidget(actionHamburgerMenu, mLineEditSearch.get());
    actionSearch->setProperty(gsPostPaneLineEditSearch, true);

    ui->toolBar->insertAction(actionHamburgerMenu, ui->tableViewLogs->actionViewLogs());
    ui->toolBar->insertAction(actionHamburgerMenu, ui->tableViewScripts->actionViewScripts());

    // add a spacer in the toolbar before the hamburger menu to ensure it is always at the right
    mToolbarSpacerRight = std::make_unique<QWidget>();
    mToolbarSpacerRight->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto actionSpacerRight = ui->toolBar->insertWidget(actionHamburgerMenu, mToolbarSpacerRight.get());
    actionSpacerRight->setProperty(gsPostPaneToolbarSpacerRight, true);

    // create the hamburger menu
    ui->menu_Hamburger->addAction(ui->treeViewSources->actionAddSource());
    ui->menu_Hamburger->addAction(ui->treeViewSources->actionRemoveSource());
    ui->menu_Hamburger->addSeparator();
    ui->menu_Hamburger->addAction(ui->treeViewSources->actionImportOPML());
    ui->menu_Hamburger->addAction(ui->treeViewSources->actionExportOPML());
    ui->menu_Hamburger->addSeparator();
    ui->menu_Hamburger->addAction(mActionShowPreferences.get());
    ui->menu_Hamburger->addAction(mActionExit.get());
}

void ZapFR::Client::MainWindow::closeEvent(QCloseEvent* event)
{
    saveSettings();

    if (mPreferences->minimizeInsteadOfClose && !mForceClose)
    {
        event->ignore();
        setWindowState(Qt::WindowMinimized);
    }
    else
    {
        if (mStartupDetectBrowsersThread != nullptr && mStartupDetectBrowsersThread->joinable())
        {
            mStartupDetectBrowsersThread->join();
        }
        ZapFR::Engine::Agent::getInstance()->joinAll();
    }
}

void ZapFR::Client::MainWindow::saveSettings() const
{
    QJsonObject root;
    root.insert(Setting::MainWindowState, QString::fromUtf8(saveState().toBase64()));
    root.insert(Setting::MainWindowGeometry, QString::fromUtf8(saveGeometry().toBase64()));
    root.insert(Setting::SplitterSourcesAndContentPanesState, QString::fromUtf8(ui->splitterSourcesAndContentPanes->saveState().toBase64()));
    root.insert(Setting::SplitterSourcesAndScriptFoldersState, QString::fromUtf8(ui->splitterSourcesAndScriptFolders->saveState().toBase64()));
    root.insert(Setting::SplitterPostsTableAdPostViewState, QString::fromUtf8(ui->splitterPostsTableAndPostView->saveState().toBase64()));
    ui->treeViewSources->saveSettings(root);
    if (mPreferences->theme != Theme::UseSystem)
    {
        root.insert(Setting::UITheme, mPreferences->theme == Theme::Light ? "light" : "dark");
    }
    root.insert(Setting::UIFontSize, mPreferences->uiFontSize);
    root.insert(Setting::PostFontSize, mPreferences->postFontSize);
    root.insert(Setting::PostDetectBrowsers, mPreferences->detectBrowsers);
    root.insert(Setting::UIHideLocalSource, mPreferences->hideLocalSource);
    root.insert(Setting::UIMinimizeInsteadOfClose, mPreferences->minimizeInsteadOfClose);
    root.insert(Setting::FeedsRefreshBehaviour, mPreferences->refreshBehaviour == RefreshBehaviour::EntireSource ? "entiresource" : "currentselection");
    auto ar = ZapFR::Engine::AutoRefresh::getInstance();
    root.insert(Setting::FeedsAutoRefreshInterval, static_cast<int32_t>(ar->feedRefreshInterval()));
    root.insert(Setting::FeedsAutoRefreshEnabled, ar->isEnabled());
    root.insert(Setting::FeedsLogLevel, ZapFR::Engine::Log::logLevel());

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
                if (root.contains(Setting::MainWindowState))
                {
                    restoreState(QByteArray::fromBase64(root.value(Setting::MainWindowState).toVariant().toByteArray()));
                }
                if (root.contains(Setting::MainWindowGeometry))
                {
                    restoreGeometry(QByteArray::fromBase64(root.value(Setting::MainWindowGeometry).toVariant().toByteArray()));
                }
                if (root.contains(Setting::SplitterSourcesAndContentPanesState))
                {
                    ui->splitterSourcesAndContentPanes->restoreState(
                        QByteArray::fromBase64(root.value(Setting::SplitterSourcesAndContentPanesState).toVariant().toByteArray()));
                }
                if (root.contains(Setting::SplitterSourcesAndScriptFoldersState))
                {
                    ui->splitterSourcesAndScriptFolders->restoreState(
                        QByteArray::fromBase64(root.value(Setting::SplitterSourcesAndScriptFoldersState).toVariant().toByteArray()));
                }
                if (root.contains(Setting::SplitterPostsTableAdPostViewState))
                {
                    ui->splitterPostsTableAndPostView->restoreState(QByteArray::fromBase64(root.value(Setting::SplitterPostsTableAdPostViewState).toVariant().toByteArray()));
                }

                ui->treeViewSources->restoreSettings(root);

                if (root.contains(Setting::UITheme))
                {
                    auto value = root.value(Setting::UITheme);
                    mPreferences->theme = (value == "light" ? Theme::Light : Theme::Dark);
                    applyColorScheme();
                }
                if (root.contains(Setting::UIFontSize))
                {
                    mPreferences->uiFontSize = static_cast<uint16_t>(root.value(Setting::UIFontSize).toInt(11));
                    updatePreferredFontSize();
                }
                if (root.contains(Setting::PostFontSize))
                {
                    mPreferences->postFontSize = static_cast<uint16_t>(root.value(Setting::PostFontSize).toInt(11));
                }
                if (root.contains(Setting::PostDetectBrowsers))
                {
                    mPreferences->detectBrowsers = root.value(Setting::PostDetectBrowsers).toBool();
                    if (mPreferences->detectBrowsers)
                    {
                        mStartupDetectBrowsersThread = std::make_unique<std::thread>([this]() { ui->webViewPost->detectBrowsers(); });
                    }
                }
                if (root.contains(Setting::UIHideLocalSource))
                {
                    mPreferences->hideLocalSource = root.value(Setting::UIHideLocalSource).toBool();
                }
                if (root.contains(Setting::UIMinimizeInsteadOfClose))
                {
                    mPreferences->minimizeInsteadOfClose = root.value(Setting::UIMinimizeInsteadOfClose).toBool();
                }
                if (root.contains(Setting::FeedsRefreshBehaviour))
                {
                    mPreferences->refreshBehaviour =
                        (root.value(Setting::FeedsRefreshBehaviour).toString() == "entiresource" ? RefreshBehaviour::EntireSource : RefreshBehaviour::CurrentSelection);
                }

                auto enableAutoRefresh{true};
                if (root.contains(Setting::FeedsAutoRefreshEnabled))
                {
                    enableAutoRefresh = root.value(Setting::FeedsAutoRefreshEnabled).toBool();
                }
                ZapFR::Engine::AutoRefresh::getInstance()->setEnabled(enableAutoRefresh);

                uint64_t autoRefreshInterval{ZapFR::Engine::DefaultFeedAutoRefreshInterval};
                if (root.contains(Setting::FeedsAutoRefreshInterval))
                {
                    autoRefreshInterval = static_cast<uint64_t>(root.value(Setting::FeedsAutoRefreshInterval).toInt(ZapFR::Engine::DefaultFeedAutoRefreshInterval));
                }

                if (root.contains(Setting::FeedsLogLevel))
                {
                    ZapFR::Engine::Log::setLogLevel(static_cast<ZapFR::Engine::LogLevel>(root.value(Setting::FeedsLogLevel).toInt(ZapFR::Engine::LogLevel::Warning)));
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
    font.setPointSizeF(mPreferences->uiFontSize);
    ui->treeViewSources->setFont(font);

    auto tableViewRowHeight = static_cast<int32_t>(std::ceil(static_cast<float>(mPreferences->uiFontSize) * 1.333f) + 8);

    font = ui->tableViewPosts->font();
    font.setPointSizeF(mPreferences->uiFontSize);
    ui->tableViewPosts->setFont(font);
    ui->tableViewPosts->verticalHeader()->setDefaultSectionSize(tableViewRowHeight);

    font = ui->tableViewPostEnclosures->font();
    font.setPointSizeF(mPreferences->uiFontSize);
    ui->tableViewPostEnclosures->setFont(font);
    ui->tableViewPostEnclosures->verticalHeader()->setDefaultSectionSize(tableViewRowHeight);

    font = ui->tableViewScriptFolders->font();
    font.setPointSizeF(mPreferences->uiFontSize);
    ui->tableViewScriptFolders->setFont(font);
    ui->tableViewScriptFolders->verticalHeader()->setDefaultSectionSize(tableViewRowHeight);

    font = ui->tableViewLogs->font();
    font.setPointSizeF(mPreferences->uiFontSize);
    ui->tableViewLogs->setFont(font);
    ui->tableViewLogs->verticalHeader()->setDefaultSectionSize(tableViewRowHeight);

    font = ui->tableViewScripts->font();
    font.setPointSizeF(mPreferences->uiFontSize);
    ui->tableViewScripts->setFont(font);
    ui->tableViewScripts->verticalHeader()->setDefaultSectionSize(tableViewRowHeight);

    ui->webViewPost->invalidatePostStylesCache();
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

    ui->treeViewSources->actionRefresh()->setIcon(configureIcon(":/refreshFeed.svg"));
    ui->treeViewSources->actionToolbarRefresh()->setIcon(configureIcon(":/refreshFeed.svg"));
    ui->treeViewSources->actionReconnectToSource()->setIcon(configureIcon(":/refreshFeed.svg"));
    ui->treeViewSources->actionAddFeed()->setIcon(configureIcon(":/addFeed.svg"));
    ui->treeViewSources->actionAddSource()->setIcon(configureIcon(":/addFeed.svg"));
    ui->treeViewSources->actionAddFolder()->setIcon(configureIcon(":/addFolder.svg"));
    ui->treeViewSources->actionEditFolder()->setIcon(configureIcon(":/edit.svg"));
    ui->treeViewSources->actionRemoveFeed()->setIcon(configureIcon(":/remove.svg"));
    ui->treeViewSources->actionRemoveFolder()->setIcon(configureIcon(":/remove.svg"));
    ui->treeViewSources->actionRemoveSource()->setIcon(configureIcon(":/remove.svg"));
    ui->treeViewSources->actionViewProperties()->setIcon(configureIcon(":/properties.svg"));
    ui->treeViewSources->actionOpenAssociatedWebsite()->setIcon(configureIcon(":/globe.svg"));
    ui->treeViewSources->actionSortFolder()->setIcon(configureIcon(":/sort.svg"));
    ui->tableViewPosts->actionMarkAsRead()->setIcon(configureIcon(":/markAsRead.svg"));
    ui->tableViewLogs->actionViewLogs()->setIcon(configureIcon(":/viewLogs.svg"));
    ui->tableViewLogs->actionClearLogs()->setIcon(configureIcon(":/remove.svg"));
    ui->tableViewScripts->actionViewScripts()->setIcon(configureIcon(":/script.svg"));
    ui->tableViewScripts->actionEditScript()->setIcon(configureIcon(":/edit.svg"));
    ui->tableViewScripts->actionRemoveScript()->setIcon(configureIcon(":/remove.svg"));
    ui->tableViewScripts->actionAddScript()->setIcon(configureIcon(":/addFeed.svg"));
    ui->tableViewScriptFolders->actionEditScriptFolder()->setIcon(configureIcon(":/edit.svg"));
    ui->tableViewScriptFolders->actionRemoveScriptFolder()->setIcon(configureIcon(":/remove.svg"));
    ui->tableViewScriptFolders->actionAddScriptFolder()->setIcon(configureIcon(":/addFeed.svg"));
    ui->pushButtonPostPreviousPage->setIcon(configureIcon(":/previousPage.svg"));
    ui->pushButtonPostFirstPage->setIcon(configureIcon(":/firstPage.svg"));
    ui->pushButtonPostNextPage->setIcon(configureIcon(":/nextPage.svg"));
    ui->pushButtonPostLastPage->setIcon(configureIcon(":/lastPage.svg"));
    ui->pushButtonLogPreviousPage->setIcon(configureIcon(":/previousPage.svg"));
    ui->pushButtonLogFirstPage->setIcon(configureIcon(":/firstPage.svg"));
    ui->pushButtonLogNextPage->setIcon(configureIcon(":/nextPage.svg"));
    ui->pushButtonLogLastPage->setIcon(configureIcon(":/lastPage.svg"));
    mActionBackToPosts->setIcon(configureIcon(":/back.svg"));
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

    ui->webViewPost->invalidatePostStylesCache();
    ui->tableViewPosts->reloadCurrentPost();
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
    ui->treeViewSources->actionAddSource()->setVisible(false);
    ui->treeViewSources->actionRemoveSource()->setVisible(false);
    ui->treeViewSources->actionImportOPML()->setVisible(false);
    ui->treeViewSources->actionExportOPML()->setVisible(false);

    switch (ui->stackedWidgetContentPanes->currentIndex())
    {
        case ContentPane::Posts:
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

                auto type = index.data(TreeViewSources::Role::Type).toULongLong();
                parentSourceHasError = ui->treeViewSources->doesSourceHaveError(index.data(TreeViewSources::Role::ParentSourceID).toULongLong());
                switch (type)
                {
                    case TreeViewSources::EntryType::Feed:
                    {
                        markAsReadCaption = tr("Mark feed as read");
                        refreshToolbarCaption = (mPreferences->refreshBehaviour == RefreshBehaviour::CurrentSelection ? tr("Refresh feed") : tr("Refresh source"));
                        refreshCaption = tr("Refresh feed");
                        break;
                    }
                    case TreeViewSources::EntryType::Folder:
                    {
                        markAsReadCaption = tr("Mark folder as read");
                        refreshToolbarCaption = (mPreferences->refreshBehaviour == RefreshBehaviour::CurrentSelection ? tr("Refresh folder") : tr("Refresh source"));
                        refreshCaption = tr("Refresh folder");
                        break;
                    }
                    case TreeViewSources::EntryType::Source:
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
                ui->treeViewSources->actionReconnectToSource()->setVisible(true);
                ui->treeViewSources->actionRemoveSource()->setVisible(true);
            }
            else
            {
                ui->treeViewSources->actionAddFeed()->setVisible(true);
                ui->treeViewSources->actionAddFolder()->setVisible(true);
                ui->treeViewSources->actionRefresh()->setVisible(true);
                ui->treeViewSources->actionToolbarRefresh()->setVisible(true);
                ui->tableViewPosts->actionMarkAsRead()->setVisible(true);
                ui->tableViewLogs->actionViewLogs()->setVisible(true);
                ui->tableViewScripts->actionViewScripts()->setVisible(true);
                ui->treeViewSources->actionAddSource()->setVisible(true);
                ui->treeViewSources->actionRemoveSource()->setVisible(true);
                ui->treeViewSources->actionImportOPML()->setVisible(true);
                ui->treeViewSources->actionExportOPML()->setVisible(true);

                ui->treeViewSources->actionAddFeed()->setEnabled(anythingSelected);
                ui->treeViewSources->actionAddFolder()->setEnabled(anythingSelected);
                ui->tableViewPosts->actionMarkAsRead()->setEnabled(anythingSelected);
                ui->tableViewPosts->actionMarkAsRead()->setText(markAsReadCaption);
                ui->tableViewLogs->actionViewLogs()->setEnabled(anythingSelected);
                ui->treeViewSources->actionRefresh()->setEnabled(anythingSelected);
                ui->treeViewSources->actionRefresh()->setText(refreshCaption);
                ui->treeViewSources->actionToolbarRefresh()->setEnabled(anythingSelected);
                ui->treeViewSources->actionToolbarRefresh()->setText(refreshToolbarCaption);
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
        case ContentPane::Logs:
        {
            mActionBackToPosts->setVisible(true);
            ui->tableViewLogs->actionClearLogs()->setVisible(true);
            for (const auto& action : ui->toolBar->actions())
            {
                if (action->property(gsPostPaneToolbarSpacerRight).isValid())
                {
                    action->setVisible(true);
                }
            }
            break;
        }
        case ContentPane::Scripts:
        {
            mActionBackToPosts->setVisible(true);
            ui->tableViewScripts->actionEditScript()->setVisible(true);
            ui->tableViewScripts->actionRemoveScript()->setVisible(true);
            ui->tableViewScripts->actionAddScript()->setVisible(true);
            for (const auto& action : ui->toolBar->actions())
            {
                if (action->property(gsPostPaneToolbarSpacerRight).isValid())
                {
                    action->setVisible(true);
                }
            }
            break;
        }
        case ContentPane::Properties:
        {
            mActionBackToPosts->setVisible(true);
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
                        mPreferences->theme = mDialogPreferences->theme();
                        applyColorScheme();

                        mPreferences->uiFontSize = mDialogPreferences->uiFontSize();
                        updatePreferredFontSize();

                        mPreferences->postFontSize = mDialogPreferences->postFontSize();
                        ui->tableViewPosts->reloadCurrentPost();

                        mPreferences->detectBrowsers = mDialogPreferences->detectBrowsersEnabled();

                        mPreferences->hideLocalSource = mDialogPreferences->hideLocalSource();
                        mPreferences->minimizeInsteadOfClose = mDialogPreferences->minimizeInsteadOfClose();
                        ui->treeViewSources->reload();

                        mPreferences->refreshBehaviour = mDialogPreferences->refreshBehaviour();
                        updateToolbar();

                        auto ar = ZapFR::Engine::AutoRefresh::getInstance();
                        ar->setEnabled(mDialogPreferences->autoRefreshEnabled());
                        ar->setFeedRefreshInterval(mDialogPreferences->autoRefreshInterval());

                        ZapFR::Engine::Log::setLogLevel(mDialogPreferences->logLevel());

                        saveSettings();
                    }
                });
    }
    mDialogPreferences->reset();
    mDialogPreferences->open();
}

ZapFR::Client::Theme ZapFR::Client::MainWindow::getCurrentColorTheme() const
{
    auto currentColorTheme = mPreferences->theme;

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

void ZapFR::Client::MainWindow::connectStuff()
{
    connect(mActionExit.get(), &QAction::triggered,
            [&]()
            {
                mForceClose = true;
                QGuiApplication::quit();
            });
    connect(mActionShowPreferences.get(), &QAction::triggered, this, &MainWindow::showPreferences);
    connect(mActionBackToPosts.get(), &QAction::triggered, [&]() { setContentPane(ContentPane::Posts); });

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, &MainWindow::applyColorScheme);
#endif

    connect(mHamburgerMenuButton.get(), &QPushButton::clicked,
            [&]()
            {
                auto p = qobject_cast<QWidget*>(ui->toolBar->parent())->mapToGlobal(ui->toolBar->geometry().bottomRight());
                p.setX(p.x() - ui->menu_Hamburger->sizeHint().width());
                ui->menu_Hamburger->popup(p);
            });

    connect(mLineEditSearch.get(), &LineEditSearch::searchRequested,
            [&]()
            {
                ui->tableViewPosts->updateActivePostFilter();
                ui->tableViewPosts->reload();
            });

    connect(ui->stackedWidgetContentPanes, &QStackedWidget::currentChanged,
            [&]()
            {
                updateToolbar();
                switch (ui->stackedWidgetContentPanes->currentIndex())
                {
                    case ContentPane::Posts:
                    {
                        ui->frameFlagFilters->setVisible(true);
                        ui->tableViewScriptFolders->setVisible(true);
                        ui->treeViewSources->setAllowDragAndDrop(true);
                        ui->treeViewSources->setUnreadBadgesShown(true);
                        ui->tableViewPosts->setPage(1);
                        ui->tableViewPosts->reload();
                        ui->treeViewSources->setPreviouslySelectedSourceID(0);
                        ui->frameFlagFilters->reload();
                        ui->treeViewSources->setDisplayMode(TreeViewSources::DisplayMode::ShowAll);
                        break;
                    }
                    case ContentPane::Logs:
                    {
                        ui->frameFlagFilters->setVisible(false);
                        ui->tableViewScriptFolders->setVisible(false);
                        ui->treeViewSources->setAllowDragAndDrop(false);
                        ui->treeViewSources->setUnreadBadgesShown(false);
                        ui->treeViewSources->setDisplayMode(TreeViewSources::DisplayMode::ShowAll);
                        break;
                    }
                    case ContentPane::Scripts:
                    {
                        ui->frameFlagFilters->setVisible(false);
                        ui->tableViewScriptFolders->setVisible(false);
                        ui->treeViewSources->setAllowDragAndDrop(false);
                        ui->treeViewSources->setUnreadBadgesShown(false);
                        ui->treeViewSources->setDisplayMode(TreeViewSources::DisplayMode::ShowSourcesOnly);
                        break;
                    }
                    case ContentPane::Properties:
                    {
                        ui->frameFlagFilters->setVisible(false);
                        ui->tableViewScriptFolders->setVisible(false);
                        ui->treeViewSources->setAllowDragAndDrop(false);
                        ui->treeViewSources->setUnreadBadgesShown(false);
                        ui->treeViewSources->setDisplayMode(TreeViewSources::DisplayMode::ShowAll);
                        break;
                    }
                }
            });
}

void ZapFR::Client::MainWindow::setStatusBarMessage(const QString& message, int32_t timeout)
{
    ui->statusbar->showMessage(message, timeout);
}

void ZapFR::Client::MainWindow::setContentPane(int32_t contentPaneID) const
{
    ui->stackedWidgetContentPanes->setCurrentIndex(contentPaneID);
}

int32_t ZapFR::Client::MainWindow::currentContentPane() const noexcept
{
    return ui->stackedWidgetContentPanes->currentIndex();
}

Ui::MainWindow* ZapFR::Client::MainWindow::getUI() const noexcept
{
    return ui;
}

ZapFR::Client::TreeViewSources* ZapFR::Client::MainWindow::treeViewSources() const noexcept
{
    return ui->treeViewSources;
}

QString ZapFR::Client::MainWindow::searchQuery() const
{
    return mLineEditSearch->text();
}

void ZapFR::Client::MainWindow::cloneSourceTreeContents(uint64_t sourceID, QStandardItemModel* destination, const std::optional<std::unordered_set<uint64_t>>& feedIDsToCheck)
{
    ui->treeViewSources->cloneSourceTreeContents(sourceID, destination, feedIDsToCheck);
}
