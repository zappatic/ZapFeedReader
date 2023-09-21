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
        class DialogJumpToPage;
        class DialogPreferences;
        class LineEditSearch;
        class TreeViewSources;

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

            Theme currentPreferenceTheme() const noexcept { return mPreferenceTheme; }
            uint16_t currentPreferenceUIFontSize() const noexcept { return mPreferenceUIFontSize; }
            uint16_t currentPreferencePostFontSize() const noexcept { return mPreferencePostFontSize; }
            bool currentPreferenceDetectBrowsers() const noexcept { return mPreferenceDetectBrowsers; }
            RefreshBehaviour currentPreferenceRefreshBehaviour() const noexcept { return mPreferenceRefreshBehaviour; }

            Theme getCurrentColorTheme() const;

            Ui::MainWindow* getUI() const noexcept;
            TreeViewSources* treeViewSources() const noexcept;
            void setStatusBarMessage(const QString& message, int32_t timeout = StatusBarDefaultTimeout);
            void showJumpToPageDialog(uint64_t currentPage, uint64_t pageCount, std::function<void(uint64_t)> callback);
            QString searchQuery() const;
            void updateToolbar();

            void setContentPane(int32_t contentPaneID) const;
            int32_t currentContentPane() const noexcept;

            QString configDir() const;

            void cloneSourceTreeContents(uint64_t sourceID, QStandardItemModel* destination, const std::optional<std::unordered_set<uint64_t>>& feedIDsToCheck);

          private slots:
            void showPreferences();
            void applyColorScheme();

          protected:
            void closeEvent(QCloseEvent* event) override;

          private:
            Ui::MainWindow* ui;

            std::unique_ptr<ZapFR::Engine::Database> mDatabase{nullptr};

            std::unique_ptr<DialogJumpToPage> mDialogJumpToPage{nullptr};
            std::unique_ptr<DialogPreferences> mDialogPreferences{nullptr};
            std::unique_ptr<LineEditSearch> mLineEditSearch{nullptr};
            std::unique_ptr<QPushButton> mHamburgerMenuButton{nullptr};
            std::unique_ptr<QWidget> mToolbarSpacerLeft{nullptr};
            std::unique_ptr<QWidget> mToolbarSpacerRight{nullptr};

            Theme mPreferenceTheme{Theme::UseSystem};
            uint16_t mPreferenceUIFontSize{11};
            uint16_t mPreferencePostFontSize{16};
            RefreshBehaviour mPreferenceRefreshBehaviour{RefreshBehaviour::CurrentSelection};
            bool mPreferenceDetectBrowsers{false};

            std::unique_ptr<QAction> mActionShowPreferences{nullptr};
            std::unique_ptr<QAction> mActionExit{nullptr};
            std::unique_ptr<QAction> mActionBackToPosts{nullptr};

            QString dataDir() const;
            QString settingsFile() const;

            void initializeUI();
            void connectStuff();
            void configureIcons();
            void updatePreferredFontSize();

            void saveSettings() const;
            void restoreSettings();

#ifdef ZFR_DUMP_PALETTE
            void dumpPalette();
#endif
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_MAINWINDOW_H
