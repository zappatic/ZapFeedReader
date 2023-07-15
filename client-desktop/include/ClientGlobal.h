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

#include "Global.h"

#include <QApplication>
#include <QDate>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QDropEvent>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QMainWindow>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QRegularExpression>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QString>
#include <QStyleHints>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QTimer>
#include <QTranslator>
#include <QTreeView>
#include <QWebEnginePage>

namespace ZapFR
{
    namespace Client
    {
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
        static constexpr uint32_t SourceTreeEntryUnreadCount{Qt::ItemDataRole::UserRole + 4};

        static constexpr uint32_t PostIDRole{Qt::ItemDataRole::UserRole + 1};
        static constexpr uint32_t PostSourceIDRole{Qt::ItemDataRole::UserRole + 2};
        static constexpr uint32_t PostFeedDRole{Qt::ItemDataRole::UserRole + 3};
        static constexpr uint32_t PostISODateRole{Qt::ItemDataRole::UserRole + 4};
        static constexpr uint32_t PostIsReadRole{Qt::ItemDataRole::UserRole + 5};

        static const QString MIMETYPE_DRAGGABLE_FEED = "application/x-zapfr-feed";

    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_CLIENTGLOBAL_H