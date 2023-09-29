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
#include <QGridLayout>
#include <QGuiApplication>
#include <QHeaderView>
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
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QSortFilterProxyModel>
#include <QSpacerItem>
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
        static const QString SETTING_UI_HIDE_LOCAL_SOURCE = "ui.hidelocalsource";
        static const QString SETTING_UI_MINIMIZE_INSTEAD_OF_CLOSE = "ui.minimizeinsteadofclose";
        static const QString SETTING_POST_FONTSIZE = "post.fontsize";
        static const QString SETTING_POST_DETECTBROWSERS = "post.detectbrowsers";
        static const QString SETTING_FEEDS_REFRESH_BEHAVIOUR = "feeds.refreshbehaviour";
        static const QString SETTING_FEEDS_AUTOREFRESH_ENABLED = "feeds.autorefresh.enabled";
        static const QString SETTING_FEEDS_AUTOREFRESH_INTERVAL = "feeds.autorefresh.interval";
        static const QString SETTING_FEEDS_LOGLEVEL = "feeds.loglevel";

        enum ContentPane
        {
            Posts = 0,
            Logs = 1,
            Scripts = 2,
            Properties = 3,
        };

        enum PostPane
        {
            Post = 0,
            PostCaption = 1,
        };

        enum PropertiesPane
        {
            SourceProps = 0,
            FolderProps = 1,
            FeedProps = 2,
        };

        enum class Theme
        {
            Light,
            Dark,
            UseSystem
        };

        enum class RefreshBehaviour
        {
            CurrentSelection,
            EntireSource,
        };

        struct Preferences
        {
            Theme theme{Theme::UseSystem};
            uint16_t uiFontSize{11};
            uint16_t postFontSize{16};
            RefreshBehaviour refreshBehaviour{RefreshBehaviour::CurrentSelection};
            bool detectBrowsers{false};
            bool hideLocalSource{false};
            bool minimizeInsteadOfClose{false};
        };

        static const QString MIMETYPE_DRAGGABLE_FEED = "application/x-zapfr-feed";
        static const QString MIMETYPE_DRAGGABLE_FOLDER = "application/x-zapfr-folder";
        static const QString MIMETYPE_COPIED_TEST_POST = "application/x-zapfr-testpost";

        static const uint32_t StatusBarDefaultTimeout{2500};

    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_CLIENTGLOBAL_H