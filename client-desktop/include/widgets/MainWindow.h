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
#include "ZapFR/Flag.h"
#include "ZapFR/base/Post.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

namespace ZapFR
{
    namespace Engine
    {
        class Database;
    } // namespace Engine

    namespace Client
    {
        class StandardItemModelSources;
        class SortFilterProxyModelSources;
        class DialogAddSource;
        class DialogAddFeed;
        class DialogAddFolder;
        class DialogEditFolder;
        class DialogImportOPML;
        class DialogJumpToPage;
        class DialogPreferences;
        class LineEditSearch;
        class TreeViewSources;

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

        class MainWindow : public QMainWindow
        {
            Q_OBJECT

          public:
            MainWindow(QWidget* parent = nullptr);
            ~MainWindow();
            MainWindow(const MainWindow& e) = delete;
            MainWindow& operator=(const MainWindow&) = delete;
            MainWindow(MainWindow&&) = delete;
            MainWindow& operator=(MainWindow&&) = delete;

            StandardItemModelSources* sourcesItemModel() const noexcept { return mItemModelSources.get(); }
            Theme currentPreferenceTheme() const noexcept { return mPreferenceTheme; }
            uint16_t currentPreferenceUIFontSize() const noexcept { return mPreferenceUIFontSize; }
            uint16_t currentPreferencePostFontSize() const noexcept { return mPreferencePostFontSize; }
            bool currentPreferenceDetectBrowsers() const noexcept { return mPreferenceDetectBrowsers; }
            RefreshBehaviour currentPreferenceRefreshBehaviour() const noexcept { return mPreferenceRefreshBehaviour; }

            Theme getCurrentColorTheme() const;

            // TODO : move into respective widgets:
            void reloadSources();
            void reloadUsedFlagColors(bool forceReload = false);

            uint64_t previouslySelectedSourceID() const noexcept { return mPreviouslySelectedSourceID; }
            void setPreviouslySelectedSourceID(uint64_t sID) noexcept { mPreviouslySelectedSourceID = sID; }
            void setStatusBarMessage(const QString& message, int32_t timeout = StatusBarDefaultTimeout);
            void setContentPane(int32_t contentPaneID) const;
            Ui::MainWindow* getUI() const noexcept;
            void showJumpToPageDialog(uint64_t currentPage, uint64_t pageCount, std::function<void(uint64_t)> callback);
            QString searchQuery() const;
            void updateToolbar();
            QString configDir() const;
            void updateFeedUnreadCountBadge(uint64_t sourceID, std::unordered_set<uint64_t> feedIDs, bool markEntireSourceAsRead, uint64_t unreadCount);

          private slots:
            // actions
            void addSource();
            void removeSource();
            void setUnreadBadgesShown(bool b);

            void addFeed();
            void refreshViaToolbarButton();
            void refreshViaContextMenu();
            void removeFeed();

            void addFolder();
            void editFolder();
            void removeFolder();

            void importOPML();
            void exportOPML();

            void showPreferences();
            void applyColorScheme();

            // callbacks
            void agentErrorOccurred(uint64_t sourceID, const std::string& errorMessage);
            void sourcePropertiesReceived(const QMap<QString, QVariant>& props);
            void remoteSourceUnreadCountsReceived(uint64_t sourceID, const std::unordered_map<uint64_t, uint64_t>& unreadCounts);

            void feedRefreshed(uint64_t sourceID, uint64_t feedID, uint64_t feedUnreadCount, const std::string& error, const std::string& feedTitle,
                               const std::string& iconHash, const std::string& icon);
            void feedAdded(uint64_t sourceID, uint64_t feedID);
            void feedRemoved();
            void feedPropertiesReceived(const QMap<QString, QVariant>& props);

            void folderRemoved();
            void folderAdded();
            void folderUpdated(uint64_t sourceID, uint64_t folderID, const std::string& newTitle);
            void folderPropertiesReceived(const QMap<QString, QVariant>& props);

            void populateSources(uint64_t sourceID, QStandardItem* sourceItem);
            void populateUsedFlags(uint64_t sourceID, const std::unordered_set<ZapFR::Engine::FlagColor>& flagColors);

          protected:
            void closeEvent(QCloseEvent* event) override;

          private:
            Ui::MainWindow* ui;
            std::unique_ptr<StandardItemModelSources> mItemModelSources{nullptr};
            std::unique_ptr<SortFilterProxyModelSources> mProxyModelSources{nullptr};

            std::unique_ptr<ZapFR::Engine::Database> mDatabase{nullptr};

            std::unique_ptr<DialogAddSource> mDialogAddSource{nullptr};
            std::unique_ptr<DialogAddFeed> mDialogAddFeed{nullptr};
            std::unique_ptr<DialogAddFolder> mDialogAddFolder{nullptr};
            std::unique_ptr<DialogEditFolder> mDialogEditFolder{nullptr};
            std::unique_ptr<DialogImportOPML> mDialogImportOPML{nullptr};
            std::unique_ptr<DialogJumpToPage> mDialogJumpToPage{nullptr};
            std::unique_ptr<DialogPreferences> mDialogPreferences{nullptr};

            std::unique_ptr<QMenu> mSourceContextMenuSource{nullptr};
            std::unique_ptr<QMenu> mSourceContextMenuSourceError{nullptr};
            std::unique_ptr<QMenu> mSourceContextMenuFeed{nullptr};
            std::unique_ptr<QMenu> mSourceContextMenuFolder{nullptr};

            std::unique_ptr<QTimer> mUpdateRemoteSourceBadgesTimer{nullptr};
            std::unique_ptr<QJsonObject> mReloadSourcesExpansionSelectionState{nullptr};

            uint64_t mPreviouslySelectedSourceID{0};
            uint64_t mInitialSourceCount{0};

            std::unique_ptr<LineEditSearch> mLineEditSearch{nullptr};
            std::unique_ptr<QPushButton> mHamburgerMenuButton{nullptr};
            std::unique_ptr<QWidget> mToobarSpacerLeft{nullptr};
            std::unique_ptr<QWidget> mToobarSpacerRight{nullptr};

            Theme mPreferenceTheme{Theme::UseSystem};
            uint16_t mPreferenceUIFontSize{11};
            uint16_t mPreferencePostFontSize{16};
            RefreshBehaviour mPreferenceRefreshBehaviour{RefreshBehaviour::CurrentSelection};
            bool mPreferenceDetectBrowsers{false};

            QString dataDir() const;
            QString settingsFile() const;

            void configureConnects();
            void connectSourceStuff();
            void connectFeedStuff();
            void connectFolderStuff();
            void connectFlagStuff();
            void connectPropertiesStuff();

            void createContextMenus();
            void createSourceContextMenus();
            void createFolderContextMenus();
            void createFeedContextMenus();

            void initializeUI();
            void initializeUISources();

            void saveSettings() const;
            void restoreSettings();
            QJsonArray expandedSourceTreeItems() const;
            void preserveSourceTreeExpansionSelectionState();
            void restoreSourceTreeExpansionSelectionState(QStandardItem* sourceItem);
            void expandSourceTreeItems(const QJsonArray& items) const;
            std::tuple<uint64_t, uint64_t> getCurrentlySelectedSourceAndFolderID() const;

            void reloadPropertiesPane();

            void configureIcons();
            void updatePreferredFontSize();
            void refreshSourceEntryType(const QModelIndex& index, uint64_t type);
            bool doesSourceHaveError(uint64_t sourceID);
            QStandardItem* findSourceStandardItem(uint64_t sourceID);
            std::unordered_set<QStandardItem*> findFeedStandardItems(QStandardItem* sourceItem, const std::optional<std::unordered_set<uint64_t>>& feedIDs);

#ifdef ZFR_DUMP_PALETTE
            void dumpPalette();
#endif
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_MAINWINDOW_H
