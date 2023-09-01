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

ZapFR::Client::SortFilterProxyModelSources::SortFilterProxyModelSources(QObject* parent) : QSortFilterProxyModel(parent)
{
    setSortRole(SourceTreeEntrySortOrderRole);
}

bool ZapFR::Client::SortFilterProxyModelSources::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (mSourceTreeDisplayMode == SourceTreeDisplayMode::ShowAll)
    {
        return true;
    }
    else
    {
        auto ix = sourceModel()->index(sourceRow, 0, sourceParent);
        if (ix.isValid())
        {
            auto type = ix.data(SourceTreeEntryTypeRole).toULongLong();
            return (type == SOURCETREE_ENTRY_TYPE_SOURCE);
        }
    }
    return true;
}

void ZapFR::Client::SortFilterProxyModelSources::setDisplayMode(SourceTreeDisplayMode mode)
{
    mSourceTreeDisplayMode = mode;
    invalidate();
}

ZapFR::Client::SortFilterProxyModelSources::SourceTreeDisplayMode ZapFR::Client::SortFilterProxyModelSources::displayMode() const noexcept
{
    return mSourceTreeDisplayMode;
}
