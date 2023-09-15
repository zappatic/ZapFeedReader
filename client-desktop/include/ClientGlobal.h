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

#ifndef ZAPFR_CLIENT_CLIENTGLOBAL_H
#define ZAPFR_CLIENT_CLIENTGLOBAL_H

#include "ZapFR/Global.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QDate>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QLocale>
#include <QMainWindow>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMimeDatabase>
#include <QMimeType>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPushButton>
#include <QRegularExpression>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QString>
#include <QStyleHints>
#include <QStylePainter>
#include <QStyledItemDelegate>
#include <QSyntaxHighlighter>
#include <QTableView>
#include <QTextCharFormat>
#include <QTimer>
#include <QToolTip>
#include <QTranslator>
#include <QTreeView>
#include <QWebEngineContextMenuRequest>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QWebEngineView>

namespace ZapFR
{
    namespace Client
    {
        static const QString SETTING_MAINWINDOW_STATE = "mainwindow.state";
        static const QString SETTING_MAINWINDOW_GEOMETRY = "mainwindow.geometry";
        static const QString SETTING_SPLITTERSOURCESANDCONTENTPANES_STATE = "splittersourcesandcontentpanes.state";
        static const QString SETTING_SPLITTERSOURCESANDSCRIPTFOLDERS_STATE = "splittersourcesandscriptfolders.state";
        static const QString SETTING_SPLITTERPOSTSTABLEANDPOSTVIEW_STATE = "splitterpoststableandpostview.state";
        static const QString SETTING_SOURCETREEVIEW_EXPANSION = "sourcetreeview.expansion";
        static const QString SETTING_UI_THEME = "ui.theme";
        static const QString SETTING_UI_FONTSIZE = "ui.fontsize";
        static const QString SETTING_FEEDS_REFRESH_BEHAVIOUR = "feeds.refreshbehaviour";

        static constexpr uint32_t SOURCETREE_ENTRY_TYPE_SOURCE = 0;
        static constexpr uint32_t SOURCETREE_ENTRY_TYPE_FEED = 1;
        static constexpr uint32_t SOURCETREE_ENTRY_TYPE_FOLDER = 2;

        static constexpr uint32_t SourceTreeEntryTypeRole{Qt::ItemDataRole::UserRole + 1};
        static constexpr uint32_t SourceTreeEntryIDRole{Qt::ItemDataRole::UserRole + 2};
        static constexpr uint32_t SourceTreeEntryParentSourceIDRole{Qt::ItemDataRole::UserRole + 3};
        static constexpr uint32_t SourceTreeEntryUnreadCount{Qt::ItemDataRole::UserRole + 4};
        static constexpr uint32_t SourceTreeEntryDisplayUnreadCountBadge{Qt::ItemDataRole::UserRole + 5};
        static constexpr uint32_t SourceTreeEntryParentFolderIDRole{Qt::ItemDataRole::UserRole + 6};
        static constexpr uint32_t SourceTreeEntryErrorRole{Qt::ItemDataRole::UserRole + 7};
        static constexpr uint32_t SourceTreeEntryFeedURLRole{Qt::ItemDataRole::UserRole + 8};
        static constexpr uint32_t SourceTreeEntrySortOrderRole{Qt::ItemDataRole::UserRole + 9};
        static constexpr uint32_t SourceTreeEntrySourceTypeRole{Qt::ItemDataRole::UserRole + 10};

        static constexpr uint32_t PostIDRole{Qt::ItemDataRole::UserRole + 1};
        static constexpr uint32_t PostSourceIDRole{Qt::ItemDataRole::UserRole + 2};
        static constexpr uint32_t PostFeedIDRole{Qt::ItemDataRole::UserRole + 3};
        static constexpr uint32_t PostISODateRole{Qt::ItemDataRole::UserRole + 4};
        static constexpr uint32_t PostIsReadRole{Qt::ItemDataRole::UserRole + 5};
        static constexpr uint32_t PostLinkRole{Qt::ItemDataRole::UserRole + 6};
        static constexpr uint32_t PostAppliedFlagsRole{Qt::ItemDataRole::UserRole + 7};

        static constexpr uint32_t PostColumnUnread = 0;
        static constexpr uint32_t PostColumnFlag = 1;
        static constexpr uint32_t PostColumnFeed = 2;
        static constexpr uint32_t PostColumnTitle = 3;
        static constexpr uint32_t PostColumnDate = 4;

        static constexpr uint32_t PostEnclosureLinkRole{Qt::ItemDataRole::UserRole + 1};

        static constexpr uint32_t PostEnclosuresColumnIcon = 0;
        static constexpr uint32_t PostEnclosuresColumnURL = 1;
        static constexpr uint32_t PostEnclosuresColumnMimetype = 2;
        static constexpr uint32_t PostEnclosuresColumnFilesize = 3;

        static constexpr uint32_t LogIDRole{Qt::ItemDataRole::UserRole + 1};
        static constexpr uint32_t LogFeedIDRole{Qt::ItemDataRole::UserRole + 2};
        static constexpr uint32_t LogLevelRole{Qt::ItemDataRole::UserRole + 3};
        static constexpr uint32_t LogParentSourceIDRole{Qt::ItemDataRole::UserRole + 4};

        // ui->stackedWidgetContentPanes's panes
        static constexpr uint32_t StackedPanePosts = 0;
        static constexpr uint32_t StackedPaneLogs = 1;
        static constexpr uint32_t StackedPaneScripts = 2;
        static constexpr uint32_t StackedPaneProperties = 3;

        // ui->stackedWidgetPost's panes
        static constexpr uint32_t StackedPanePost = 0;
        static constexpr uint32_t StackedPanePostCaption = 1;

        // ui->stackedWidgetProperties's panes
        static constexpr uint32_t StackedPanePropertiesSource = 0;
        static constexpr uint32_t StackedPanePropertiesFolder = 1;
        static constexpr uint32_t StackedPanePropertiesFeed = 2;

        static constexpr uint32_t LogsColumnLogLevel = 0;
        static constexpr uint32_t LogsColumnFeed = 1;
        static constexpr uint32_t LogsColumnTimestamp = 2;
        static constexpr uint32_t LogsColumnMessage = 3;

        static constexpr uint32_t ScriptFolderIDRole{Qt::ItemDataRole::UserRole + 1};
        static constexpr uint32_t ScriptFolderSourceIDRole{Qt::ItemDataRole::UserRole + 2};
        static constexpr uint32_t ScriptFolderShowTotalRole{Qt::ItemDataRole::UserRole + 3};
        static constexpr uint32_t ScriptFolderShowUnreadRole{Qt::ItemDataRole::UserRole + 4};
        static constexpr uint32_t ScriptFolderTotalPostCountRole{Qt::ItemDataRole::UserRole + 5};
        static constexpr uint32_t ScriptFolderTotalUnreadCountRole{Qt::ItemDataRole::UserRole + 6};

        static constexpr uint32_t ScriptFolderColumnTitle = 0;

        static constexpr uint32_t ScriptsColumnType = 0;
        static constexpr uint32_t ScriptsColumnTitle = 1;
        static constexpr uint32_t ScriptsColumnIsEnabled = 2;
        static constexpr uint32_t ScriptsColumnRunOnEvents = 3;
        static constexpr uint32_t ScriptsColumnRunOnFeedIDs = 4;

        static constexpr uint32_t ScriptIDRole{Qt::ItemDataRole::UserRole + 1};
        static constexpr uint32_t ScriptIsEnabledRole{Qt::ItemDataRole::UserRole + 2};
        static constexpr uint32_t ScriptSourceIDRole{Qt::ItemDataRole::UserRole + 3};
        static constexpr uint32_t ScriptRunOnEventsRole{Qt::ItemDataRole::UserRole + 5};
        static constexpr uint32_t ScriptRunOnFeedIDsRole{Qt::ItemDataRole::UserRole + 6};
        static constexpr uint32_t ScriptEventCountRole{Qt::ItemDataRole::UserRole + 7};
        static constexpr uint32_t ScriptTitleRole{Qt::ItemDataRole::UserRole + 8};
        static constexpr uint32_t ScriptScriptRole{Qt::ItemDataRole::UserRole + 9};

        static const QString MIMETYPE_DRAGGABLE_FEED = "application/x-zapfr-feed";
        static const QString MIMETYPE_DRAGGABLE_FOLDER = "application/x-zapfr-folder";

    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_CLIENTGLOBAL_H