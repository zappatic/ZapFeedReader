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

#ifndef ZAPFR_CLIENT_DIALOGEDITSCRIPTFOLDER_H
#define ZAPFR_CLIENT_DIALOGEDITSCRIPTFOLDER_H

#include <QDialog>

#include "../ClientGlobal.h"

namespace Ui
{
    class DialogEditScriptFolder;
}

namespace ZapFR
{
    namespace Client
    {
        class DialogEditScriptFolder : public QDialog
        {
            Q_OBJECT

          public:
            explicit DialogEditScriptFolder(QWidget* parent = nullptr);
            ~DialogEditScriptFolder();
            DialogEditScriptFolder(const DialogEditScriptFolder& e) = delete;
            DialogEditScriptFolder& operator=(const DialogEditScriptFolder&) = delete;
            DialogEditScriptFolder(DialogEditScriptFolder&&) = delete;
            DialogEditScriptFolder& operator=(DialogEditScriptFolder&&) = delete;

            enum class DisplayMode
            {
                Add,
                Edit
            };
            DisplayMode displayMode() const noexcept;
            void reset(DisplayMode dm, uint64_t sourceID, uint64_t id, const QString& title, bool showTotal, bool showUnread);
            QString title() const noexcept;
            bool showTotal() const noexcept;
            bool showUnread() const noexcept;
            uint64_t sourceID() const noexcept;
            uint64_t id() const noexcept;

          private:
            Ui::DialogEditScriptFolder* ui;
            uint64_t mCurrentSourceID{0};
            uint64_t mCurrentID{0};
            DisplayMode mDisplayMode{DisplayMode::Add};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_DIALOGEDITSCRIPTFOLDER_H
