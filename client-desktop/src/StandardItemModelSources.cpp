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

#include "StandardItemModelSources.h"
#include "MainWindow.h"
#include "Source.h"

ZapFR::Client::StandardItemModelSources::StandardItemModelSources(MainWindow* mainWindow, QObject* parent) : QStandardItemModel(parent), mMainWindow(mainWindow)
{
}

Qt::ItemFlags ZapFR::Client::StandardItemModelSources::flags(const QModelIndex& index) const
{
    auto flags = QStandardItemModel::flags(index);

    if (index.isValid())
    {
        // feeds should be drag enabled but not drop enabled
        // everything else should be drop enabled but not drag enabled
        if (index.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FEED)
        {
            flags = flags | Qt::ItemIsDragEnabled;
            flags = flags & ~(Qt::ItemIsDropEnabled);
        }
        else
        {
            flags = flags & ~(Qt::ItemIsDragEnabled);
            flags = flags | Qt::ItemIsDropEnabled;
        }
    }

    return flags;
}

bool ZapFR::Client::StandardItemModelSources::dropMimeData(const QMimeData* data, Qt::DropAction /*action*/, int row, int /*column*/, const QModelIndex& parent)
{
    auto json = data->data(MIMETYPE_DRAGGABLE_FEED);
    auto jsonDoc = QJsonDocument::fromJson(json);
    auto o = jsonDoc.array().at(0).toObject();
    auto childSourceID = o.value("source").toVariant().toULongLong();
    auto childID = o.value("id").toVariant().toULongLong();

    // can only move around feeds within the same source
    auto parentSourceID = parent.data(SourceTreeEntryParentSourceIDRole).toULongLong();
    if (parentSourceID != childSourceID)
    {
        return false;
    }

    uint64_t newSortOrder = static_cast<uint64_t>(std::max(0, (row * 10) + 5));
    auto newFolderHierarchy = QString("");
    if (parent.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FOLDER)
    {
        // build the tree of subfolders all the way to the parent
        auto cur = parent;
        QStringList subfolders;
        while (true)
        {
            subfolders << cur.data(Qt::DisplayRole).toString();
            cur = cur.parent();
            if (!cur.isValid() || cur.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_SOURCE)
            {
                break;
            }
        }
        std::reverse(subfolders.begin(), subfolders.end());
        newFolderHierarchy = subfolders.join("/");
    }

    // perform the move
    auto source = ZapFR::Engine::Source::getSource(parentSourceID);
    if (!source.has_value())
    {
        return false;
    }
    source.value()->moveFeed(childID, newFolderHierarchy.toStdString(), newSortOrder);
    QTimer::singleShot(100, [&]() { mMainWindow->reloadSources(); });
    return true;
}

QStringList ZapFR::Client::StandardItemModelSources::mimeTypes() const
{
    return QStringList() << MIMETYPE_DRAGGABLE_FEED;
}

QMimeData* ZapFR::Client::StandardItemModelSources::mimeData(const QModelIndexList& indexes) const
{
    if (indexes.length() == 0)
    {
        return nullptr;
    }

    QJsonArray arr;
    for (const auto& index : indexes)
    {
        QJsonObject o;
        o.insert("source", QJsonValue::fromVariant(index.data(SourceTreeEntryParentSourceIDRole)));
        o.insert("id", QJsonValue::fromVariant(index.data(SourceTreeEntryIDRole)));
        arr.append(o);
    }

    auto mimeData = new QMimeData();
    mimeData->setData(MIMETYPE_DRAGGABLE_FEED, QJsonDocument(arr).toJson());

    return mimeData;
}
