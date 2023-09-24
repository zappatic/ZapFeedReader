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

#ifndef ZAPFR_CLIENT_TABLEVIEWPOSTS_H
#define ZAPFR_CLIENT_TABLEVIEWPOSTS_H

#include "ClientGlobal.h"
#include "Utilities.h"
#include "ZapFR/Flag.h"
#include "ZapFR/base/Post.h"
#include "widgets/TableViewPaletteCorrected.h"

namespace ZapFR
{
    namespace Client
    {
        class PopupFlagChooser;
        class MainWindow;
        class WebEnginePagePost;

        class TableViewPosts : public TableViewPaletteCorrected
        {
            Q_OBJECT

          public:
            TableViewPosts(QWidget* parent = nullptr);
            ~TableViewPosts() = default;
            TableViewPosts(const TableViewPosts& e) = delete;
            TableViewPosts& operator=(const TableViewPosts&) = delete;
            TableViewPosts(TableViewPosts&&) = delete;
            TableViewPosts& operator=(TableViewPosts&&) = delete;

            void setMainWindow(MainWindow* mainWindow) noexcept;
            void reload();
            void reloadCurrentPost();
            void clearPosts();
            void setPage(uint64_t page) noexcept { mCurrentPostPage = page; }
            void updateActivePostFilter();
            void markAsRead();

            ZapFR::Engine::FlagColor flagFilter() const noexcept { return mFlagFilter; }
            void setFlagFilter(ZapFR::Engine::FlagColor f) noexcept { mFlagFilter = f; }

            QAction* actionMarkAsRead() const noexcept { return mActionMarkAsRead.get(); }
            QAction* actionMarkSelectionAsRead() const noexcept { return mActionMarkSelectionAsRead.get(); }
            QAction* actionMarkSelectionAsUnread() const noexcept { return mActionMarkSelectionAsUnread.get(); }

            enum Column
            {
                UnreadCol = 0,
                FlagCol = 1,
                FeedCol = 2,
                TitleCol = 3,
                DateCol = 4,
            };

            enum Role
            {
                ID = Qt::ItemDataRole::UserRole + 1,
                SourceID = Qt::ItemDataRole::UserRole + 2,
                FeedID = Qt::ItemDataRole::UserRole + 3,
                ISODate = Qt::ItemDataRole::UserRole + 4,
                IsRead = Qt::ItemDataRole::UserRole + 5,
                Link = Qt::ItemDataRole::UserRole + 6,
                AppliedFlags = Qt::ItemDataRole::UserRole + 7,
            };

          signals:
            void postMarkedFlagged(uint64_t sourceID, uint64_t feedID, uint64_t postID, ZapFR::Engine::FlagColor flagColor);
            void postMarkedUnflagged(uint64_t sourceID, uint64_t feedID, uint64_t postID, ZapFR::Engine::FlagColor flagColor);
            void clearAllFlagsRequested(uint64_t sourceID, uint64_t feedID, uint64_t postID);

          private slots:
            void processFlagToggle(ZapFR::Engine::FlagColor flagColor, Utilities::FlagStyle flagStyle);
            void openPostInExternalBrowser();

          protected:
            void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;
            void mouseMoveEvent(QMouseEvent* event) override;
            void mouseReleaseEvent(QMouseEvent* event) override;
            void keyPressEvent(QKeyEvent* event) override;
            void paintEvent(QPaintEvent* event) override;

          private:
            MainWindow* mMainWindow{nullptr};
            std::unique_ptr<QStandardItemModel> mItemModelPosts{nullptr};
            std::unique_ptr<PopupFlagChooser> mPopupFlagChooser{nullptr};
            std::unordered_set<uint64_t> mPreviouslySelectedPostIDs{};
            std::unique_ptr<QMenu> mPostContextMenu{nullptr};
            std::unique_ptr<WebEnginePagePost> mPostWebEnginePage{nullptr};

            std::unique_ptr<QAction> mActionMarkAsRead{nullptr};
            std::unique_ptr<QAction> mActionMarkSelectionAsRead{nullptr};
            std::unique_ptr<QAction> mActionMarkSelectionAsUnread{nullptr};
            std::unique_ptr<QAction> mActionOpenInExternalBrowser{nullptr};
            std::unique_ptr<QAction> mActionCopyForTestScript{nullptr};

            uint64_t mCurrentPostSourceID{0};
            uint64_t mCurrentPostFeedID{0};
            uint64_t mCurrentPostID{0};
            uint64_t mCurrentPostPage{1};
            uint64_t mCurrentPostCount{0};
            uint64_t mCurrentPostPageCount{1};
            bool mShowOnlyUnreadPosts{false};
            ZapFR::Engine::FlagColor mFlagFilter{ZapFR::Engine::FlagColor::Gray};

            void populatePosts(const QList<QList<QStandardItem*>>& posts = {}, uint64_t pageNumber = 1, uint64_t totalPostCount = 0);
            void handleSelectionChanged(const QModelIndexList& selected);
            void connectStuff();
            void createContextMenus();
            std::vector<std::tuple<uint64_t, uint64_t>> selectedPostIDs() const;

            void postReadyToBeShown(const QString& html, const std::vector<ZapFR::Engine::Post::Enclosure>& enclosures);
            void postsMarkedFlagged(bool reloadPosts);
            void postsMarkedUnflagged(bool reloadPosts);
            void postsMarkedRead(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& postIDs);
            void postsMarkedUnread(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& postIDs);
            void postsAssignedToScriptFolder(uint64_t sourceID, uint64_t scriptFolderID);
            void postsRemovedFromScriptFolder(uint64_t sourceID, uint64_t scriptFolderID);

            void markPostSelectionAsRead();
            void markPostSelectionAsUnread();
            void markPostSelectionFlagged();
            void markPostSelectionUnflagged();
            void assignPostSelectionToScriptFolder();
            void removePostSelectionFromScriptFolder();

            static constexpr uint64_t msPostsPerPage{100};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_TABLEVIEWPOSTS_H
