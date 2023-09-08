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

#ifndef ZAPFR_CLIENT_DIALOGEDITFOLDER_H
#define ZAPFR_CLIENT_DIALOGEDITFOLDER_H

#include "../ClientGlobal.h"

namespace Ui
{
    class DialogEditFolder;
}

namespace ZapFR
{
    namespace Client
    {
        class DialogEditFolder : public QDialog
        {
            Q_OBJECT

          public:
            explicit DialogEditFolder(QWidget* parent = nullptr);
            ~DialogEditFolder();
            DialogEditFolder(const DialogEditFolder& e) = delete;
            DialogEditFolder& operator=(const DialogEditFolder&) = delete;
            DialogEditFolder(DialogEditFolder&&) = delete;
            DialogEditFolder& operator=(DialogEditFolder&&) = delete;

            void reset(uint64_t sourceID, uint64_t id, const QString& title);
            QString title() const noexcept;
            uint64_t sourceID() const noexcept;
            uint64_t id() const noexcept;

          private:
            Ui::DialogEditFolder* ui;
            uint64_t mCurrentSourceID{0};
            uint64_t mCurrentID{0};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_DIALOGEDITFOLDER_H
