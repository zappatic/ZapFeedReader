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

#ifndef ZAPFR_CLIENT_TREEVIEWSOURCES_H
#define ZAPFR_CLIENT_TREEVIEWSOURCES_H

#include <unordered_set>

#include <QJsonObject>
#include <QKeyEvent>
#include <QStandardItemModel>
#include <QTimer>

#include "ZapFR/base/Feed.h"
#include "ZapFR/base/Folder.h"
#include "ZapFR/base/Source.h"
#include "widgets/TreeViewPaletteCorrected.h"

namespace ZapFR
{
    namespace Client
    {
        class MainWindow;
        class SortFilterProxyModelSources;
        class DialogAddSource;
        class DialogAddFeed;
        class DialogAddFolder;
        class DialogEditFolder;
        class DialogImportOPML;

        class TreeViewSources : public TreeViewPaletteCorrected
        {
            Q_OBJECT

          public:
            TreeViewSources(QWidget* parent = nullptr);
            ~TreeViewSources() = default;
            TreeViewSources(const TreeViewSources& e) = delete;
            TreeViewSources& operator=(const TreeViewSources&) = delete;
            TreeViewSources(TreeViewSources&&) = delete;
            TreeViewSources& operator=(TreeViewSources&&) = delete;

            void reload();
            void setMainWindow(MainWindow* mw) noexcept;
            QStandardItemModel* sourcesItemModel() const noexcept { return mItemModelSources.get(); }
            uint64_t previouslySelectedSourceID() const noexcept { return mPreviouslySelectedSourceID; }
            void setPreviouslySelectedSourceID(uint64_t sID) noexcept { mPreviouslySelectedSourceID = sID; }
            uint64_t highestPostID(uint64_t sourceID) const;

            std::tuple<uint64_t, uint64_t> getCurrentlySelectedSourceAndFolderID() const;
            std::tuple<uint64_t, uint64_t> getCurrentlySelectedSourceAndFeedID() const;
            void selectFeed(uint64_t sourceID, uint64_t feedID);
            bool doesSourceHaveError(uint64_t sourceID);

            QStandardItem* findSourceStandardItem(uint64_t sourceID);
            QStandardItem* findFolderStandardItem(uint64_t sourceID, uint64_t folderID);
            std::unordered_set<QStandardItem*> findFeedStandardItems(QStandardItem* sourceItem, const std::optional<std::vector<uint64_t>>& feedIDs);

            void setUnreadBadgesShown(bool b);
            void refreshBadges();
            void setAllowDragAndDrop(bool b);
            void agentErrorOccurred(uint64_t sourceID, const std::string& errorMessage);
            void updateFeedUnreadCountBadge(uint64_t sourceID, std::vector<uint64_t> feedIDs, bool markEntireSourceAsRead, uint64_t unreadCount);
            void updateFeedSortOrders(uint64_t sourceID, const std::unordered_map<uint64_t, uint64_t>& feedIDs);
            void updateFolderSortOrders(uint64_t sourceID, const std::unordered_map<uint64_t, uint64_t>& folderIDs);
            void saveSettings(QJsonObject& root);
            void restoreSettings(const QJsonObject& root);

            void reloadPropertiesPane();
            void cloneSourceTreeContents(uint64_t sourceID, QStandardItemModel* destination, const std::optional<std::unordered_set<uint64_t>>& feedIDsToCheck);
            void feedRefreshed(uint64_t sourceID, uint64_t feedID, uint64_t feedUnreadCount, const std::string& error, const std::string& feedTitle,
                               const std::string& iconHash, const std::string& icon);

            QAction* actionAddSource() const noexcept { return mActionAddSource.get(); }
            QAction* actionRemoveSource() const noexcept { return mActionRemoveSource.get(); }
            QAction* actionAddFeed() const noexcept { return mActionAddFeed.get(); }
            QAction* actionRemoveFeed() const noexcept { return mActionRemoveFeed.get(); }
            QAction* actionAddFolder() const noexcept { return mActionAddFolder.get(); }
            QAction* actionEditFolder() const noexcept { return mActionEditFolder.get(); }
            QAction* actionRemoveFolder() const noexcept { return mActionRemoveFolder.get(); }
            QAction* actionToolbarRefresh() const noexcept { return mActionToolbarRefresh.get(); }
            QAction* actionRefresh() const noexcept { return mActionRefresh.get(); }
            QAction* actionReconnectToSource() const noexcept { return mActionReconnectToSource.get(); }
            QAction* actionImportOPML() const noexcept { return mActionImportOPML.get(); }
            QAction* actionExportOPML() const noexcept { return mActionExportOPML.get(); }
            QAction* actionViewProperties() const noexcept { return mActionViewProperties.get(); }
            QAction* actionOpenAssociatedWebsite() const noexcept { return mActionOpenAssociatedWebsite.get(); }
            QAction* actionSortFolder() const noexcept { return mActionSortFolder.get(); }

            enum class DisplayMode
            {
                ShowSourcesOnly,
                ShowAll
            };
            void setDisplayMode(DisplayMode dm);
            DisplayMode displayMode() const noexcept { return mDisplayMode; }

            enum Role
            {
                Type = Qt::ItemDataRole::UserRole + 1,
                ID = Qt::ItemDataRole::UserRole + 2,
                ParentSourceID = Qt::ItemDataRole::UserRole + 3,
                UnreadCount = Qt::ItemDataRole::UserRole + 4,
                DisplayUnreadCountBadge = Qt::ItemDataRole::UserRole + 5,
                ParentFolderID = Qt::ItemDataRole::UserRole + 6,
                Error = Qt::ItemDataRole::UserRole + 7,
                FeedURL = Qt::ItemDataRole::UserRole + 8,
                FeedLink = Qt::ItemDataRole::UserRole + 9,
                SortOrder = Qt::ItemDataRole::UserRole + 10,
                SourceType = Qt::ItemDataRole::UserRole + 11,
            };

            enum EntryType : uint64_t
            {
                Source = 0,
                Feed = 1,
                Folder = 2,
            };

          signals:
            void deletePressed();
            void folderDoubleClicked();

          protected:
            void currentChanged(const QModelIndex& current, const QModelIndex& previous) override;
            void keyPressEvent(QKeyEvent* event) override;
            void mouseDoubleClickEvent(QMouseEvent* event) override;

          private slots:
            void addSource();
            void removeSource();

            void addFolder();
            void editFolder();
            void sortFolder();
            void removeFolder();

            void addFeed();
            void removeFeed();
            void refreshViaToolbarButton();
            void refreshViaContextMenu();

            void importOPML();
            void exportOPML();

            void feedAdded(uint64_t sourceID, QStandardItem* feedItem);

          private:
            MainWindow* mMainWindow{nullptr};

            std::unique_ptr<QStandardItemModel> mItemModelSources{nullptr};
            std::unique_ptr<SortFilterProxyModelSources> mProxyModelSources{nullptr};
            std::unique_ptr<DialogAddSource> mDialogAddSource{nullptr};
            std::unique_ptr<DialogAddFolder> mDialogAddFolder{nullptr};
            std::unique_ptr<DialogEditFolder> mDialogEditFolder{nullptr};
            std::unique_ptr<DialogImportOPML> mDialogImportOPML{nullptr};
            std::unique_ptr<DialogAddFeed> mDialogAddFeed{nullptr};
            std::unique_ptr<QMenu> mContextMenuSource{nullptr};
            std::unique_ptr<QMenu> mContextMenuSourceError{nullptr};
            std::unique_ptr<QMenu> mContextMenuFeed{nullptr};
            std::unique_ptr<QMenu> mContextMenuFolder{nullptr};
            std::unique_ptr<QTimer> mUpdateRemoteSourceBadgesTimer{nullptr};

            std::unique_ptr<QAction> mActionAddSource{nullptr};
            std::unique_ptr<QAction> mActionRemoveSource{nullptr};
            std::unique_ptr<QAction> mActionAddFeed{nullptr};
            std::unique_ptr<QAction> mActionRemoveFeed{nullptr};
            std::unique_ptr<QAction> mActionAddFolder{nullptr};
            std::unique_ptr<QAction> mActionEditFolder{nullptr};
            std::unique_ptr<QAction> mActionRemoveFolder{nullptr};
            std::unique_ptr<QAction> mActionToolbarRefresh{nullptr};
            std::unique_ptr<QAction> mActionRefresh{nullptr};
            std::unique_ptr<QAction> mActionReconnectToSource{nullptr};
            std::unique_ptr<QAction> mActionImportOPML{nullptr};
            std::unique_ptr<QAction> mActionExportOPML{nullptr};
            std::unique_ptr<QAction> mActionViewProperties{nullptr};
            std::unique_ptr<QAction> mActionOpenAssociatedWebsite{nullptr};
            std::unique_ptr<QAction> mActionSortFolder{nullptr};

            uint64_t mPreviouslySelectedSourceID{0};
            std::unique_ptr<QJsonObject> mReloadExpansionSelectionState{nullptr};
            DisplayMode mDisplayMode{DisplayMode::ShowAll};

            std::unordered_map<uint64_t, uint64_t> mHighestPostIDs{};

            void preserveExpansionSelectionState();
            void restoreExpansionSelectionState();
            QJsonArray expandedItems() const;
            void expandItems(const QJsonArray& items);
            void remoteSourceStatusReceived(uint64_t sourceID, const Poco::JSON::Object& statusObj);
            void refreshSourceEntryType(const QModelIndex& index, uint64_t type);

            void connectStuff();
            void createContextMenus();
            QStandardItem* createSourceStandardItem(ZapFR::Engine::Source* source);
            QStandardItem* createFolderStandardItem(uint64_t sourceID, ZapFR::Engine::Folder* folder, std::unordered_map<uint64_t, QStandardItem*>* folderIDToIDMap = nullptr);
            QStandardItem* createFeedStandardItem(uint64_t sourceID, ZapFR::Engine::Feed* feed);
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_TREEVIEWSOURCES_H
