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

#include "widgets/TableViewPosts.h"
#include "widgets/PopupFlagChooser.h"

ZapFR::Client::TableViewPosts::TableViewPosts(QWidget* parent) : TableViewPaletteCorrected(parent)
{
    mPopupFlagChooser = std::make_unique<PopupFlagChooser>(this);

    connect(this, &QTableView::doubleClicked, this, &TableViewPosts::doubleClickedRow);
    connect(mPopupFlagChooser.get(), &PopupFlagChooser::flagToggled, this, &TableViewPosts::processFlagToggle);

    viewport()->setAttribute(Qt::WA_Hover);
    setMouseTracking(true);
}

void ZapFR::Client::TableViewPosts::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    QTableView::selectionChanged(selected, deselected);

    QModelIndexList list;
    for (const auto& index : selectedIndexes())
    {
        if (index.column() == 0)
        {
            list.append(index);
        }
    }
    emit selectedPostsChanged(list);
}

void ZapFR::Client::TableViewPosts::mouseMoveEvent(QMouseEvent* event)
{
    auto index = indexAt(event->pos());
    if (index.isValid() && index.column() == PostColumnFlag)
    {
        setCursor(Qt::PointingHandCursor);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
    }
    QTableView::mouseMoveEvent(event);
}

void ZapFR::Client::TableViewPosts::mouseReleaseEvent(QMouseEvent* event)
{
    auto index = indexAt(event->pos());
    if (index.isValid() && index.column() == PostColumnFlag)
    {
        auto clickLocation = mapToGlobal(event->pos());
        if ((QGuiApplication::keyboardModifiers() & Qt::ControlModifier) == Qt::ControlModifier)
        {
            auto sourceID = index.data(PostSourceIDRole).toULongLong();
            auto feedID = index.data(PostFeedIDRole).toULongLong();
            auto postID = index.data(PostIDRole).toULongLong();
            emit clearAllFlagsRequested(sourceID, feedID, postID);

            qobject_cast<QStandardItemModel*>(model())->itemFromIndex(index)->setData(QVariantList(), PostAppliedFlagsRole);
            return;
        }
        else
        {
            mPopupFlagChooser->setGeometry(clickLocation.x(), clickLocation.y(), 200, 75);
            mPopupFlagChooser->showWithSelectedColors(index.data(PostAppliedFlagsRole).toList());
        }
    }
    QTableView::mouseReleaseEvent(event);
}

void ZapFR::Client::TableViewPosts::doubleClickedRow(const QModelIndex& index)
{
    if (index.isValid())
    {
        auto link = index.data(PostLinkRole).toString();
        if (!link.isEmpty() && link.startsWith("http"))
        {
            QDesktopServices::openUrl(link);
        }
    }
}

void ZapFR::Client::TableViewPosts::processFlagToggle(ZapFR::Engine::FlagColor flagColor, Utilities::FlagStyle flagStyle)
{
    QModelIndex index;
    auto sm = selectionModel();
    if (sm != nullptr)
    {
        auto selectedIndexes = sm->selectedIndexes();
        for (const auto& selectedIndex : selectedIndexes)
        {
            if (selectedIndex.column() == PostColumnFlag)
            {
                index = selectedIndex;
                break;
            }
        }
    }

    if (!index.isValid())
    {
        return;
    }

    auto sourceID = index.data(PostSourceIDRole).toULongLong();
    auto feedID = index.data(PostFeedIDRole).toULongLong();
    auto postID = index.data(PostIDRole).toULongLong();
    auto flags = index.data(PostAppliedFlagsRole).toList();

    switch (flagStyle)
    {
        case Utilities::FlagStyle::Filled:
        {
            emit postMarkedFlagged(sourceID, feedID, postID, flagColor);
            flags << QVariant(static_cast<std::underlying_type_t<ZapFR::Engine::FlagColor>>(flagColor));
            qobject_cast<QStandardItemModel*>(model())->itemFromIndex(index)->setData(flags, PostAppliedFlagsRole);
            break;
        }
        case Utilities::FlagStyle::Unfilled:
        {
            emit postMarkedUnflagged(sourceID, feedID, postID, flagColor);
            flags.removeIf([&](const QVariant& v) { return flagColor == static_cast<ZapFR::Engine::FlagColor>(v.toInt()); });
            qobject_cast<QStandardItemModel*>(model())->itemFromIndex(index)->setData(flags, PostAppliedFlagsRole);
            break;
        }
    }
}