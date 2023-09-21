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

#include "models/SortFilterProxyModelSources.h"
#include "widgets/TreeViewSources.h"

ZapFR::Client::SortFilterProxyModelSources::SortFilterProxyModelSources(QObject* parent) : QSortFilterProxyModel(parent)
{
}

bool ZapFR::Client::SortFilterProxyModelSources::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (mDisplayMode == TreeViewSources::DisplayMode::ShowAll)
    {
        return true;
    }
    else
    {
        auto ix = sourceModel()->index(sourceRow, 0, sourceParent);
        if (ix.isValid())
        {
            auto type = ix.data(TreeViewSources::Role::Type).toULongLong();
            return (type == TreeViewSources::EntryType::Source);
        }
    }
    return true;
}

void ZapFR::Client::SortFilterProxyModelSources::setDisplayMode(TreeViewSources::DisplayMode mode)
{
    mDisplayMode = mode;
    invalidate();
}

bool ZapFR::Client::SortFilterProxyModelSources::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    auto leftType = left.data(TreeViewSources::Role::Type).toULongLong();
    auto rightType = right.data(TreeViewSources::Role::Type).toULongLong();

    // make sure folders and feeds are sorted separately within a folder, and folders go at the top
    if (leftType == TreeViewSources::EntryType::Folder && rightType == TreeViewSources::EntryType::Feed)
    {
        return true;
    }
    else if (leftType == TreeViewSources::EntryType::Feed && rightType == TreeViewSources::EntryType::Folder)
    {
        return false;
    }

    return left.data(TreeViewSources::Role::SortOrder).toULongLong() < right.data(TreeViewSources::Role::SortOrder).toULongLong();
}
