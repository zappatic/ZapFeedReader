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

#ifndef ZAPFR_CLIENT_DIALOGJUMPTOPAGE_H
#define ZAPFR_CLIENT_DIALOGJUMPTOPAGE_H

#include "ClientGlobal.h"

namespace Ui
{
    class DialogJumpToPage;
}

namespace ZapFR
{
    namespace Client
    {
        class DialogJumpToPage : public QDialog
        {
            Q_OBJECT

          public:
            explicit DialogJumpToPage(QWidget* parent = nullptr);
            ~DialogJumpToPage();
            DialogJumpToPage(const DialogJumpToPage& e) = delete;
            DialogJumpToPage& operator=(const DialogJumpToPage&) = delete;
            DialogJumpToPage(DialogJumpToPage&&) = delete;
            DialogJumpToPage& operator=(DialogJumpToPage&&) = delete;

            void reset(uint64_t currentPage, uint64_t totalPageCount, std::function<void(uint64_t)> callback);

            uint64_t pageToJumpTo() const;
            std::function<void(uint64_t)> callback() const;

          private slots:
            void jumpTextChanged(const QString& text);

          private:
            Ui::DialogJumpToPage* ui;
            uint64_t mCurrentPage{1};
            uint64_t mTotalPageCount{1};
            std::function<void(uint64_t)> mCallback{};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_DIALOGJUMPTOPAGE_H
