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

#ifndef ZAPFR_CLIENT_STANDARDITEMMODELSOURCES_H
#define ZAPFR_CLIENT_STANDARDITEMMODELSOURCES_H

#include "ClientGlobal.h"

namespace ZapFR
{
    namespace Client
    {
        class MainWindow;

        class StandardItemModelSources : public QStandardItemModel
        {
            Q_OBJECT

          public:
            StandardItemModelSources(MainWindow* mainWindow, QObject* parent = nullptr);
            ~StandardItemModelSources() = default;

            Qt::ItemFlags flags(const QModelIndex& index) const override;
            bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
            QStringList mimeTypes() const override;
            QMimeData* mimeData(const QModelIndexList& indexes) const override;

          private:
            MainWindow* mMainWindow;
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_STANDARDITEMMODELSOURCES_H
