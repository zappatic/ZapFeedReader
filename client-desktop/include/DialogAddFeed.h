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

#ifndef ZAPFR_CLIENT_DIALOGADDFEED_H
#define ZAPFR_CLIENT_DIALOGADDFEED_H

#include "ClientGlobal.h"
#include "DialogWithSourcesAndFolders.h"

namespace Ui
{
    class DialogAddFeed;
}

namespace ZapFR
{
    namespace Client
    {
        class DialogAddFeed : public DialogWithSourcesAndFolders
        {
            Q_OBJECT

          public:
            explicit DialogAddFeed(QWidget* parent = nullptr);
            ~DialogAddFeed();

            QString url() const;

            void reset(uint64_t selectedSourceID, uint64_t selectedFolderID);

          private:
            Ui::DialogAddFeed* ui;
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_DIALOGADDFEED_H
