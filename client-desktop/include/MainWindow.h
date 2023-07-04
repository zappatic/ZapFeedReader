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

#include "Global.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

namespace ZapFR
{
    namespace Client
    {
        class MainWindow : public QMainWindow
        {
            Q_OBJECT

          public:
            MainWindow(QWidget* parent = nullptr);
            ~MainWindow();

          private slots:
            void addSource();

          private:
            Ui::MainWindow* ui;
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_MAINWINDOW_H
