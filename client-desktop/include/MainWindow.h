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

#include "ClientGlobal.h"
#include "Database.h"
#include "DialogAddFeed.h"
#include "DialogAddFolder.h"
#include "DialogImportOPML.h"
#include "DialogJumpToPostPage.h"
#include "Global.h"

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
        class StandardItemModelSources;

        class MainWindow : public QMainWindow
        {
            Q_OBJECT

          public:
            MainWindow(QWidget* parent = nullptr);
            ~MainWindow();

            void reloadSources(bool performClickOnSelection = true);

          private slots:
            // actions
            void addSource();
            void addFeed();
            void addFolder();
            void importOPML();
            void markAsRead();
            void refreshAllFeeds();
            void reloadPosts();
            void navigateNextPostPage();
            void navigatePreviousPostPage();
            void navigateFirstPostPage();
            void navigateLastPostPage();

            // events
            void sourceTreeViewItemSelected(const QModelIndex& index);
            void postsTableViewSelectionChanged(const QModelIndexList& selected);
            void colorSchemeChanged(Qt::ColorScheme scheme);
            void sourceTreeViewContextMenuRequested(const QPoint& p);
            void postsTableViewContextMenuRequested(const QPoint& p);
            void postLinkHovered(const QString& url);
            void toggleShowOnlyUnread();
            void postPageNumberClicked();

            // callbacks
            void feedRefreshed(uint64_t feedID);
            void feedAdded();
            void feedRemoved();
            void feedMoved();
            void folderRemoved();
            void folderMoved();
            void folderAdded();
            void postMarkedRead(uint64_t postID);
            void postsMarkedUnread(std::vector<std::tuple<uint64_t, uint64_t>> postIDs);
            void feedMarkedRead();
            void setPostHTML(const QString& html);
            void populatePosts(const QList<QList<QStandardItem*>>& posts = {}, uint64_t pageNumber = 1, uint64_t totalPostCount = 1);

          protected:
            void closeEvent(QCloseEvent* event) override;

          private:
            Ui::MainWindow* ui;
            std::unique_ptr<StandardItemModelSources> mItemModelSources{nullptr};
            std::unique_ptr<QStandardItemModel> mItemModelPosts{nullptr};
            std::unique_ptr<ZapFR::Engine::Database> mDatabase{nullptr};
            std::unique_ptr<DialogAddFeed> mDialogAddFeed{nullptr};
            std::unique_ptr<DialogAddFolder> mDialogAddFolder{nullptr};
            std::unique_ptr<DialogImportOPML> mDialogImportOPML{nullptr};
            std::unique_ptr<DialogJumpToPostPage> mDialogJumpToPostPage{nullptr};
            std::unique_ptr<WebEnginePagePost> mPostWebEnginePage{nullptr};
            std::unique_ptr<QMenu> mSourceContextMenuSource{nullptr};
            std::unique_ptr<QMenu> mSourceContextMenuFeed{nullptr};
            std::unique_ptr<QMenu> mSourceContextMenuFolder{nullptr};
            std::unique_ptr<QMenu> mPostContextMenu{nullptr};
            uint64_t mCurrentPostSourceID{0};
            uint64_t mCurrentPostFeedID{0};
            uint64_t mCurrentPostID{0};
            uint64_t mCurrentPostPage{1};
            uint64_t mCurrentPostCount{1};
            uint64_t mCurrentPostPageCount{1};
            bool mShowOnlyUnreadPosts{false};
            QStandardItem* mFirstSource{nullptr};
            bool mReclickOnSource{true};

            QString dataDir() const;
            QString configDir() const;
            QString settingsFile() const;

            void fixPalette() const;
            void saveSettings() const;
            void restoreSettings();
            QJsonArray expandedSourceTreeItems() const;
            void expandSourceTreeItems(const QJsonArray& items) const;
            std::tuple<uint64_t, uint64_t> getCurrentlySelectedSourceAndFolderID() const;

            void reloadCurrentPost();
            QString postStyles() const;
            QString textMessageHTML(const QString& message) const;
            void configureIcons();
            void setupToolbarEnabledStates();
            void createContextMenus();
            void createContextMenuSource();
            void createContextMenuFeed();
            void createContextMenuFolder();
            void createContextMenuPost();
            QModelIndex selectedSourceTreeIndex() const;

            static constexpr uint64_t msPostsPerPage{100};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_MAINWINDOW_H
