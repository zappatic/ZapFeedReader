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

#ifndef ZAPFR_CLIENT_LINEEDITSEARCH_H
#define ZAPFR_CLIENT_LINEEDITSEARCH_H

#include <QLineEdit>

namespace ZapFR
{
    namespace Client
    {
        class LineEditSearch : public QLineEdit
        {
            Q_OBJECT

          public:
            explicit LineEditSearch(QWidget* parent = nullptr);
            ~LineEditSearch() = default;
            LineEditSearch(const LineEditSearch& e) = delete;
            LineEditSearch& operator=(const LineEditSearch&) = delete;
            LineEditSearch(LineEditSearch&&) = delete;
            LineEditSearch& operator=(LineEditSearch&&) = delete;

            void setSearchIconColor(const QString& color);

          signals:
            void searchRequested();

          private:
            QAction* mSearchIconAction{nullptr};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_LINEEDITSEARCH_H
