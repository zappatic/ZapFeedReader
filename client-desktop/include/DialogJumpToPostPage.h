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

#ifndef ZAPFR_CLIENT_DIALOGJUMPTOPOSTPAGE_H
#define ZAPFR_CLIENT_DIALOGJUMPTOPOSTPAGE_H

#include "ClientGlobal.h"

namespace Ui
{
    class DialogJumpToPostPage;
}

namespace ZapFR
{
    namespace Client
    {
        class DialogJumpToPostPage : public QDialog
        {
            Q_OBJECT

          public:
            explicit DialogJumpToPostPage(QWidget* parent = nullptr);
            ~DialogJumpToPostPage();

            void reset(uint64_t currentPage, uint64_t totalPageCount);
            uint64_t pageToJumpTo() const;

          private slots:
            void jumpTextChanged(const QString& text);

          private:
            Ui::DialogJumpToPostPage* ui;
            uint64_t mCurrentPage{1};
            uint64_t mTotalPageCount{1};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_DIALOGJUMPTOPOSTPAGE_H
