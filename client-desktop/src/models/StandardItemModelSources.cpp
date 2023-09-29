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

#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QMimeData>

#include "ZapFR/Agent.h"
#include "models/StandardItemModelSources.h"
#include "widgets/MainWindow.h"
#include "widgets/TreeViewSources.h"

ZapFR::Client::StandardItemModelSources::StandardItemModelSources(MainWindow* mainWindow, QObject* parent) : QStandardItemModel(parent), mMainWindow(mainWindow)
{
}

Qt::ItemFlags ZapFR::Client::StandardItemModelSources::flags(const QModelIndex& index) const
{
    auto flags = QStandardItemModel::flags(index);

    if (!mAllowDragAndDrop)
    {
        flags = flags & ~(Qt::ItemIsDragEnabled);
        flags = flags & ~(Qt::ItemIsDropEnabled);
        return flags;
    }

    if (index.isValid())
    {
        auto type = index.data(TreeViewSources::Role::Type);
        if (type == TreeViewSources::EntryType::Feed) // feeds should be drag enabled but not drop enabled
        {
            flags = flags | Qt::ItemIsDragEnabled;
            flags = flags & ~(Qt::ItemIsDropEnabled);
        }
        else if (type == TreeViewSources::EntryType::Folder) // folders should be drag and drop enabled
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
    if (!mAllowDragAndDrop)
    {
        return false;
    }

    QByteArray json;
    if (data->hasFormat(MimeType::DraggableFeed))
    {
        json = data->data(MimeType::DraggableFeed);
    }
    else if (data->hasFormat(MimeType::DraggableFolder))
    {
        json = data->data(MimeType::DraggableFolder);
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
    auto parentSourceID = parent.data(TreeViewSources::Role::ParentSourceID).toULongLong();
    if (parentSourceID != childSourceID)
    {
        QMessageBox::warning(mMainWindow, tr("Can't drop here"), tr("Feeds and folders can't be dragged and dropped across sources."));
        return false;
    }

    uint64_t newSortOrder = static_cast<uint64_t>(std::max(0, (row * 10) + 5));
    uint64_t newFolder{0};
    if (parent.data(TreeViewSources::Role::Type) == TreeViewSources::EntryType::Folder)
    {
        newFolder = parent.data(TreeViewSources::Role::ID).toULongLong();
    }

    if (data->hasFormat(MimeType::DraggableFeed))
    {
        ZapFR::Engine::Agent::getInstance()->queueMoveFeed(parentSourceID, childID, newFolder, newSortOrder,
                                                           [&]() { QMetaObject::invokeMethod(mMainWindow, [this]() { mMainWindow->treeViewSources()->reload(); }); });
    }
    else if (data->hasFormat(MimeType::DraggableFolder))
    {
        ZapFR::Engine::Agent::getInstance()->queueMoveFolder(parentSourceID, childID, newFolder, newSortOrder,
                                                             [&]() { QMetaObject::invokeMethod(mMainWindow, [this]() { mMainWindow->treeViewSources()->reload(); }); });
    }
    return true;
}

QStringList ZapFR::Client::StandardItemModelSources::mimeTypes() const
{
    return QStringList() << MimeType::DraggableFeed << MimeType::DraggableFolder;
}

QMimeData* ZapFR::Client::StandardItemModelSources::mimeData(const QModelIndexList& indexes) const
{
    if (!mAllowDragAndDrop || indexes.length() == 0)
    {
        return nullptr;
    }

    auto mimeData = new QMimeData();

    if (indexes.count() == 1)
    {
        auto index = indexes.at(0);
        auto type = index.data(TreeViewSources::Role::Type).toUInt();

        QJsonObject o;
        o.insert("source", QJsonValue::fromVariant(index.data(TreeViewSources::Role::ParentSourceID)));
        o.insert("id", QJsonValue::fromVariant(index.data(TreeViewSources::Role::ID)));

        switch (type)
        {
            case TreeViewSources::EntryType::Feed:
            {
                mimeData->setData(MimeType::DraggableFeed, QJsonDocument(o).toJson());
                break;
            }
            case TreeViewSources::EntryType::Folder:
            {
                mimeData->setData(MimeType::DraggableFolder, QJsonDocument(o).toJson());
                break;
            }
        }
    }

    return mimeData;
}
