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

#include "models/StandardItemModelSources.h"
#include "ZapFR/Agent.h"
#include "widgets/MainWindow.h"

ZapFR::Client::StandardItemModelSources::StandardItemModelSources(MainWindow* mainWindow, QObject* parent) : QStandardItemModel(parent), mMainWindow(mainWindow)
{
}

Qt::ItemFlags ZapFR::Client::StandardItemModelSources::flags(const QModelIndex& index) const
{
    auto flags = QStandardItemModel::flags(index);

    if (index.isValid())
    {
        auto type = index.data(SourceTreeEntryTypeRole);
        if (type == SOURCETREE_ENTRY_TYPE_FEED) // feeds should be drag enabled but not drop enabled
        {
            flags = flags | Qt::ItemIsDragEnabled;
            flags = flags & ~(Qt::ItemIsDropEnabled);
        }
        else if (type == SOURCETREE_ENTRY_TYPE_FOLDER) // folders should be drag and drop enabled
        {
            flags = flags | Qt::ItemIsDragEnabled;
            flags = flags | Qt::ItemIsDropEnabled;
        }
        else // everything else should be drop enabled but not drag enabled
        {
            flags = flags & ~(Qt::ItemIsDragEnabled);
            flags = flags | Qt::ItemIsDropEnabled;
        }
    }

    return flags;
}

bool ZapFR::Client::StandardItemModelSources::dropMimeData(const QMimeData* data, Qt::DropAction /*action*/, int row, int /*column*/, const QModelIndex& parent)
{
    QByteArray json;
    if (data->hasFormat(MIMETYPE_DRAGGABLE_FEED))
    {
        json = data->data(MIMETYPE_DRAGGABLE_FEED);
    }
    else if (data->hasFormat(MIMETYPE_DRAGGABLE_FOLDER))
    {
        json = data->data(MIMETYPE_DRAGGABLE_FOLDER);
    }
    else
    {
        return false;
    }

    auto jsonDoc = QJsonDocument::fromJson(json);
    auto o = jsonDoc.object();
    auto childSourceID = o.value("source").toVariant().toULongLong();
    auto childID = o.value("id").toVariant().toULongLong();

    // can only move around feeds and folders within the same source
    auto parentSourceID = parent.data(SourceTreeEntryParentSourceIDRole).toULongLong();
    if (parentSourceID != childSourceID)
    {
        return false;
    }

    uint64_t newSortOrder = static_cast<uint64_t>(std::max(0, (row * 10) + 5));
    uint64_t newFolder{0};
    if (parent.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FOLDER)
    {
        newFolder = parent.data(SourceTreeEntryIDRole).toULongLong();
    }

    if (data->hasFormat(MIMETYPE_DRAGGABLE_FEED))
    {
        ZapFR::Engine::Agent::getInstance()->queueMoveFeed(parentSourceID, childID, newFolder, newSortOrder,
                                                           [&]() { QMetaObject::invokeMethod(mMainWindow, "feedMoved", Qt::AutoConnection); });
    }
    else if (data->hasFormat(MIMETYPE_DRAGGABLE_FOLDER))
    {
        ZapFR::Engine::Agent::getInstance()->queueMoveFolder(parentSourceID, childID, newFolder, newSortOrder,
                                                             [&]() { QMetaObject::invokeMethod(mMainWindow, "folderMoved", Qt::AutoConnection); });
    }
    return true;
}

QStringList ZapFR::Client::StandardItemModelSources::mimeTypes() const
{
    return QStringList() << MIMETYPE_DRAGGABLE_FEED << MIMETYPE_DRAGGABLE_FOLDER;
}

QMimeData* ZapFR::Client::StandardItemModelSources::mimeData(const QModelIndexList& indexes) const
{
    if (indexes.length() == 0)
    {
        return nullptr;
    }

    auto mimeData = new QMimeData();

    if (indexes.count() == 1)
    {
        auto index = indexes.at(0);
        auto type = index.data(SourceTreeEntryTypeRole).toUInt();

        QJsonObject o;
        o.insert("source", QJsonValue::fromVariant(index.data(SourceTreeEntryParentSourceIDRole)));
        o.insert("id", QJsonValue::fromVariant(index.data(SourceTreeEntryIDRole)));

        switch (type)
        {
            case SOURCETREE_ENTRY_TYPE_FEED:
            {
                mimeData->setData(MIMETYPE_DRAGGABLE_FEED, QJsonDocument(o).toJson());
                break;
            }
            case SOURCETREE_ENTRY_TYPE_FOLDER:
            {
                mimeData->setData(MIMETYPE_DRAGGABLE_FOLDER, QJsonDocument(o).toJson());
                break;
            }
        }
    }

    return mimeData;
}
