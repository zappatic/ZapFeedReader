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

#ifndef ZAPFR_CLIENT_MAINWINDOW_H
#define ZAPFR_CLIENT_MAINWINDOW_H

#include "Database.h"
#include "DialogAddFeed.h"
#include "Global.h"
#include <QMainWindow>
#include <QStandardItemModel>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

namespace ZapFR
{
    namespace Client
    {
        class WebEnginePagePost;

        class MainWindow : public QMainWindow
        {
            Q_OBJECT

          public:
            MainWindow(QWidget* parent = nullptr);
            ~MainWindow();

          private slots:
            void addSource();
            void addFeed();
            void sourceTreeViewItemClicked(const QModelIndex& index);
            void postsTableViewItemClicked(const QModelIndex& index);
            void colorSchemeChanged(Qt::ColorScheme scheme);
            void sourceTreeViewContextMenuRequested(const QPoint& p);

          protected:
            void closeEvent(QCloseEvent* event) override;

          private:
            Ui::MainWindow* ui;
            std::unique_ptr<QStandardItemModel> mItemModelSources{nullptr};
            std::unique_ptr<QStandardItemModel> mItemModelPosts{nullptr};
            std::unique_ptr<ZapFR::Engine::Database> mDatabase{nullptr};
            std::unique_ptr<DialogAddFeed> mDialogAddFeed{nullptr};
            std::unique_ptr<WebEnginePagePost> mPostWebEnginePage{nullptr};
            std::unique_ptr<QMenu> mSourceContextMenuFeed{nullptr};
            uint64_t mCurrentPostSourceID{0};
            uint64_t mCurrentPostFeedID{0};
            uint64_t mCurrentPostID{0};

            void reloadSources();
            QString dataDir() const;
            QString configDir() const;
            QString settingsFile() const;

            void saveSettings() const;
            void restoreSettings();
            QJsonArray expandedSourceTreeItems() const;
            void expandSourceTreeItems(const QJsonArray& items) const;

            QString getFolderHierarchy(QStandardItem* item) const;
            void reloadCurrentPost() const;
            QString postStyles() const;
            void createContextMenus();
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_MAINWINDOW_H
