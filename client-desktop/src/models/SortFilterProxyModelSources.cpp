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

#include <Poco/JSON/Parser.h>
#include <QMessageBox>
#include <QMimeData>

#include "ClientGlobal.h"
#include "ZapFR/Agent.h"
#include "models/SortFilterProxyModelSources.h"
#include "widgets/MainWindow.h"
#include "widgets/TreeViewSources.h"

ZapFR::Client::SortFilterProxyModelSources::SortFilterProxyModelSources(MainWindow* mainWindow, QObject* parent) : QSortFilterProxyModel(parent), mMainWindow(mainWindow)
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

Qt::ItemFlags ZapFR::Client::SortFilterProxyModelSources::flags(const QModelIndex& index) const
{
    auto flags = QSortFilterProxyModel::flags(index);

    if (!mAllowDragAndDrop)
    {
        flags = flags & ~(Qt::ItemIsDragEnabled);
        flags = flags & ~(Qt::ItemIsDropEnabled);
        return flags;
    }

    if (index.isValid())
    {
        auto type = index.data(TreeViewSources::Role::Type).toULongLong();
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

bool ZapFR::Client::SortFilterProxyModelSources::canDropMimeData(const QMimeData* data, Qt::DropAction /*action*/, int /*row*/, int /*column*/,
                                                                 const QModelIndex& /*parent*/) const
{
    return data->hasFormat(MimeType::DraggableFeed) || data->hasFormat(MimeType::DraggableFolder); // TODO: expand
}

bool ZapFR::Client::SortFilterProxyModelSources::dropMimeData(const QMimeData* data, Qt::DropAction /*action*/, int row, int /*column*/, const QModelIndex& parent)
{
    if (!mAllowDragAndDrop || !parent.isValid())
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

    auto jsonStr = std::string(json.constData(), static_cast<size_t>(json.length()));

    Poco::JSON::Parser parser;
    auto root = parser.parse(jsonStr);
    auto serialized = root.extract<Poco::JSON::Object::Ptr>();
    if (serialized.isNull())
    {
        return false;
    }

    // can only move around feeds and folders within the same source
    auto childSourceID = serialized->getValue<uint64_t>("ParentSourceID");
    auto parentSourceID = parent.data(TreeViewSources::Role::ParentSourceID).toULongLong();
    if (parentSourceID != childSourceID)
    {
        QMessageBox::warning(mMainWindow, tr("Can't drop here"), tr("Feeds and folders can't be dragged and dropped across sources."));
        return false;
    }

    auto unserialized = unserializeItem(serialized);
    auto parentItem = mMainWindow->treeViewSources()->sourcesItemModel()->itemFromIndex(mapToSource(parent));
    if (parentItem != nullptr)
    {
        // it doesn't matter that we put the unserialized item at the end, as the sort orders are updated in the callback of the agent move function
        parentItem->appendRow(unserialized);
    }

    auto droppedOnIndex = index(row, 0, parent);
    int64_t delta = -5;
    if (unserialized->data(TreeViewSources::Role::SortOrder).toULongLong() < droppedOnIndex.data(TreeViewSources::Role::SortOrder).toULongLong())
    {
        delta = 5;
    }
    int64_t newSortOrder = droppedOnIndex.data(TreeViewSources::Role::SortOrder).toLongLong() + delta;

    uint64_t newFolder{0};
    if (parent.data(TreeViewSources::Role::Type).toULongLong() == TreeViewSources::EntryType::Folder)
    {
        newFolder = parent.data(TreeViewSources::Role::ID).toULongLong();
    }

    if (data->hasFormat(MimeType::DraggableFeed))
    {
        ZapFR::Engine::Agent::getInstance()->queueMoveFeed(
            parentSourceID, unserialized->data(TreeViewSources::Role::ID).toULongLong(), newFolder, static_cast<uint64_t>(newSortOrder),
            [&](uint64_t affectedSourceID, const std::unordered_map<uint64_t, uint64_t>& affectedFeeds)
            { QMetaObject::invokeMethod(mMainWindow, [=, this]() { mMainWindow->treeViewSources()->updateFeedSortOrders(affectedSourceID, affectedFeeds); }); });
    }
    else if (data->hasFormat(MimeType::DraggableFolder))
    {
        ZapFR::Engine::Agent::getInstance()->queueMoveFolder(parentSourceID, unserialized->data(TreeViewSources::Role::ID).toULongLong(), newFolder,
                                                             static_cast<uint64_t>(newSortOrder),
                                                             [&]() { QMetaObject::invokeMethod(mMainWindow, [this]() { mMainWindow->treeViewSources()->reload(); }); });
    }
    return true;
}

QStringList ZapFR::Client::SortFilterProxyModelSources::mimeTypes() const
{
    return QStringList() << MimeType::DraggableFeed << MimeType::DraggableFolder;
}

QMimeData* ZapFR::Client::SortFilterProxyModelSources::mimeData(const QModelIndexList& indexes) const
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

        auto serialized = serializeItem(index);
        auto serializedBA = QByteArray(serialized.c_str(), static_cast<ssize_t>(serialized.length()));

        switch (type)
        {
            case TreeViewSources::EntryType::Feed:
            {
                mimeData->setData(MimeType::DraggableFeed, serializedBA);
                break;
            }
            case TreeViewSources::EntryType::Folder:
            {
                mimeData->setData(MimeType::DraggableFolder, serializedBA);
                break;
            }
        }
    }

    return mimeData;
}

std::string ZapFR::Client::SortFilterProxyModelSources::serializeItem(const QModelIndex& index) const
{
    Poco::JSON::Object o;

    o.set("DisplayRole", index.data(Qt::DisplayRole).toString().toStdString());
    if (!index.data(Qt::ToolTipRole).isNull())
    {
        o.set("ToolTipRole", index.data(Qt::ToolTipRole).toString().toStdString());
    }
    o.set("Type", index.data(TreeViewSources::Role::Type).toULongLong());
    o.set("ID", index.data(TreeViewSources::Role::ID).toULongLong());
    o.set("ParentSourceID", index.data(TreeViewSources::Role::ParentSourceID).toULongLong());
    o.set("DisplayUnreadCountBadge", index.data(TreeViewSources::Role::DisplayUnreadCountBadge).toBool());
    o.set("ParentFolderID", index.data(TreeViewSources::Role::ParentFolderID).toULongLong());
    auto error = index.data(TreeViewSources::Role::Error);
    if (error.isValid())
    {
        o.set("Error", error.toString().toStdString());
    }
    o.set("FeedURL", index.data(TreeViewSources::Role::FeedURL).toString().toStdString());
    o.set("FeedLink", index.data(TreeViewSources::Role::FeedLink).toString().toStdString());
    o.set("SortOrder", index.data(TreeViewSources::Role::SortOrder).toULongLong());

    // qDebug() << "before:";
    // qDebug() << "Type" << index.data(TreeViewSources::Role::Type);
    // qDebug() << "ID" << index.data(TreeViewSources::Role::ID);
    // qDebug() << "ParentSourceID" << index.data(TreeViewSources::Role::ParentSourceID);
    // qDebug() << "DisplayUnreadCountBadge" << index.data(TreeViewSources::Role::DisplayUnreadCountBadge);
    // qDebug() << "ParentFolderID" << index.data(TreeViewSources::Role::ParentFolderID);
    // qDebug() << "Error" << index.data(TreeViewSources::Role::Error);
    // qDebug() << "FeedURL" << index.data(TreeViewSources::Role::FeedURL);
    // qDebug() << "FeedLink" << index.data(TreeViewSources::Role::FeedLink);
    // qDebug() << "SortOrder" << index.data(TreeViewSources::Role::SortOrder);

    std::stringstream ss;
    Poco::JSON::Stringifier::stringify(o, ss);
    return ss.str();
}

QStandardItem* ZapFR::Client::SortFilterProxyModelSources::unserializeItem(const Poco::JSON::Object::Ptr o) const
{
    auto item = new QStandardItem(QString::fromUtf8(o->getValue<std::string>("DisplayRole")));
    if (o->has("ToolTipRole"))
    {
        item->setData(QString::fromUtf8(o->getValue<std::string>("ToolTipRole")), Qt::ToolTipRole);
    }

    item->setData(QVariant::fromValue<uint64_t>(o->getValue<uint64_t>("Type")), TreeViewSources::Role::Type);
    item->setData(QVariant::fromValue<uint64_t>(o->getValue<uint64_t>("ID")), TreeViewSources::Role::ID);
    item->setData(QVariant::fromValue<uint64_t>(o->getValue<uint64_t>("ParentSourceID")), TreeViewSources::Role::ParentSourceID);
    item->setData(QVariant::fromValue<bool>(o->getValue<bool>("DisplayUnreadCountBadge")), TreeViewSources::Role::DisplayUnreadCountBadge);
    item->setData(QVariant::fromValue<uint64_t>(o->getValue<uint64_t>("ParentFolderID")), TreeViewSources::Role::ParentFolderID);
    if (o->has("Error"))
    {
        item->setData(QString::fromUtf8(o->getValue<std::string>("Error")), TreeViewSources::Role::Error);
    }
    item->setData(QString::fromUtf8(o->getValue<std::string>("FeedURL")), TreeViewSources::Role::FeedURL);
    item->setData(QString::fromUtf8(o->getValue<std::string>("FeedLink")), TreeViewSources::Role::FeedLink);
    item->setData(QVariant::fromValue<uint64_t>(o->getValue<uint64_t>("SortOrder")), TreeViewSources::Role::SortOrder);

    // qDebug() << "after:";
    // qDebug() << "Type" << item->data(TreeViewSources::Role::Type);
    // qDebug() << "ID" << item->data(TreeViewSources::Role::ID);
    // qDebug() << "ParentSourceID" << item->data(TreeViewSources::Role::ParentSourceID);
    // qDebug() << "DisplayUnreadCountBadge" << item->data(TreeViewSources::Role::DisplayUnreadCountBadge);
    // qDebug() << "ParentFolderID" << item->data(TreeViewSources::Role::ParentFolderID);
    // qDebug() << "Error" << item->data(TreeViewSources::Role::Error);
    // qDebug() << "FeedURL" << item->data(TreeViewSources::Role::FeedURL);
    // qDebug() << "FeedLink" << item->data(TreeViewSources::Role::FeedLink);
    // qDebug() << "SortOrder" << item->data(TreeViewSources::Role::SortOrder);

    return item;
}
