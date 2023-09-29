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

#ifndef ZAPFR_CLIENT_SORTFILTERPROXYMODELSOURCES_H
#define ZAPFR_CLIENT_SORTFILTERPROXYMODELSOURCES_H

#include <Poco/JSON/Object.h>
#include <QSortFilterProxyModel>

#include "widgets/TreeViewSources.h"

namespace ZapFR
{
    namespace Client
    {
        class MainWindow;

        class SortFilterProxyModelSources : public QSortFilterProxyModel
        {
            Q_OBJECT

          public:
            SortFilterProxyModelSources(MainWindow* mainWindow, QObject* parent = nullptr);
            ~SortFilterProxyModelSources() = default;
            SortFilterProxyModelSources(const SortFilterProxyModelSources& e) = delete;
            SortFilterProxyModelSources& operator=(const SortFilterProxyModelSources&) = delete;
            SortFilterProxyModelSources(SortFilterProxyModelSources&&) = delete;
            SortFilterProxyModelSources& operator=(SortFilterProxyModelSources&&) = delete;

            void setDisplayMode(TreeViewSources::DisplayMode mode);
            void setAllowDragAndDrop(bool b) noexcept { mAllowDragAndDrop = b; }

          protected:
            bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
            bool lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight) const override;

            Qt::ItemFlags flags(const QModelIndex& index) const override;
            bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const override;
            bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
            QStringList mimeTypes() const override;
            QMimeData* mimeData(const QModelIndexList& indexes) const override;

          private:
            MainWindow* mMainWindow;
            TreeViewSources::DisplayMode mDisplayMode{TreeViewSources::DisplayMode::ShowAll};
            bool mAllowDragAndDrop{true};

            std::string serializeItem(const QModelIndex& index) const;
            QStandardItem* unserializeItem(const Poco::JSON::Object::Ptr o) const;
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_SORTFILTERPROXYMODELSOURCES_H
