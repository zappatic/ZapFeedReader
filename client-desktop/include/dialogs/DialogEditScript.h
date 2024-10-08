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

#ifndef ZAPFR_CLIENT_DIALOGEDITSCRIPT_H
#define ZAPFR_CLIENT_DIALOGEDITSCRIPT_H

#include <QDialog>
#include <QStandardItemModel>

#include "../ClientGlobal.h"
#include "ZapFR/base/Script.h"
#include "ZapFR/dummy/FeedDummy.h"
#include "ZapFR/dummy/PostDummy.h"
#include "ZapFR/dummy/SourceDummy.h"
#include "dialogs/DialogTestScriptEditEnclosure.h"

namespace Ui
{
    class DialogEditScript;
}

namespace ZapFR
{
    namespace Client
    {
        class SyntaxHighlighterLua;

        class DialogEditScript : public QDialog
        {
            Q_OBJECT

          public:
            explicit DialogEditScript(QWidget* parent = nullptr);
            ~DialogEditScript();
            DialogEditScript(const DialogEditScript& e) = delete;
            DialogEditScript& operator=(const DialogEditScript&) = delete;
            DialogEditScript(DialogEditScript&&) = delete;
            DialogEditScript& operator=(DialogEditScript&&) = delete;

            enum class DisplayMode
            {
                Add,
                Edit
            };
            DisplayMode displayMode() const noexcept;
            void reset(DisplayMode dm, uint64_t sourceID, uint64_t id, const QString& title, bool isEnabled,
                       const std::unordered_set<ZapFR::Engine::Script::Event>& runOnEvents, const std::optional<std::unordered_set<uint64_t>>& runOnFeedIDs,
                       const QString& scripts);

            uint64_t scriptID() const noexcept;
            uint64_t scriptSourceID() const noexcept;
            QString title() const noexcept;
            QString script() const noexcept;
            bool isScriptEnabled() const noexcept;
            std::unordered_set<ZapFR::Engine::Script::Event> runOnEvents() const;
            bool runOnAllFeeds() const noexcept;
            std::unordered_set<uint64_t> runOnFeedIDs() const;

          protected:
            void closeEvent(QCloseEvent* e) override;

          private slots:
            void markDirty();
            void runTestScript();
            void resetTestValues();
            void clearLog();
            void appendToLog(const QString& message);
            void pasteTestPost();
            void addEnclosure();
            void editEnclosure();
            void removeEnclosure();

          private:
            Ui::DialogEditScript* ui;
            uint64_t mCurrentSourceID{0};
            uint64_t mCurrentID{0};
            DisplayMode mDisplayMode{DisplayMode::Add};
            std::unique_ptr<QStandardItemModel> mFeedsModel{nullptr};
            std::unique_ptr<QStandardItemModel> mTestEnclosuresModel{nullptr};
            std::unique_ptr<DialogTestScriptEditEnclosure> mDialogEditEnclosure{nullptr};
            std::unique_ptr<SyntaxHighlighterLua> mSyntaxHighlighterLua{nullptr};
            bool mIsDirty{false};

            std::unique_ptr<ZapFR::Engine::SourceDummy> mDummySource{nullptr};
            std::unique_ptr<ZapFR::Engine::FeedDummy> mDummyFeed{nullptr};
            std::unique_ptr<ZapFR::Engine::PostDummy> mDummyPost{nullptr};
            void initializeTestEnvironment();
            void updateTestUI();

            enum EnclosureColumn
            {
                URL = 0,
                MimeType = 1,
                Size = 2,
            };

            enum DialogEditScriptPane
            {
                Details = 0,
                Script = 1,
                Test = 2,
            };
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_DIALOGEDITSCRIPT_H
