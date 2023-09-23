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

#include "widgets/TableViewScriptFolders.h"
#include "./ui_MainWindow.h"
#include "ZapFR/Agent.h"
#include "ZapFR/base/ScriptFolder.h"
#include "delegates/ItemDelegateScriptFolder.h"
#include "dialogs/DialogEditScriptFolder.h"
#include "widgets/MainWindow.h"
#include "widgets/TreeViewSources.h"

ZapFR::Client::TableViewScriptFolders::TableViewScriptFolders(QWidget* parent) : TableViewPaletteCorrected(parent)
{
    setItemDelegate(new ItemDelegateScriptFolder(this));

    mActionAddScriptFolder = std::make_unique<QAction>(tr("Add script folder"), this);
    mActionEditScriptFolder = std::make_unique<QAction>(tr("Edit script folder"), this);
    mActionRemoveScriptFolder = std::make_unique<QAction>(tr("Remove script folder"), this);
    connect(mActionEditScriptFolder.get(), &QAction::triggered, this, &TableViewScriptFolders::editScriptFolder);
    connect(mActionRemoveScriptFolder.get(), &QAction::triggered, this, &TableViewScriptFolders::removeScriptFolder);
    connect(mActionAddScriptFolder.get(), &QAction::triggered, this, &TableViewScriptFolders::addScriptFolder);

    mScriptFolderContextMenu = std::make_unique<QMenu>(nullptr);
    mScriptFolderContextMenu->addAction(mActionAddScriptFolder.get());
    mScriptFolderContextMenu->addAction(mActionEditScriptFolder.get());
    mScriptFolderContextMenu->addSeparator();
    mScriptFolderContextMenu->addAction(mActionRemoveScriptFolder.get());

    connect(this, &QTableView::doubleClicked, this, &TableViewScriptFolders::editScriptFolder);
    connect(this, &TableViewScriptFolders::customContextMenuRequested, [&](const QPoint& p) { mScriptFolderContextMenu->popup(viewport()->mapToGlobal(p)); });
}

void ZapFR::Client::TableViewScriptFolders::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    auto ui = mMainWindow->getUI();

    QTableView::selectionChanged(selected, deselected);
    for (const auto& index : selectedIndexes())
    {
        if (index.isValid() && index.column() == Column::TitleCol)
        {
            ui->tableViewPosts->reload();
        }
    }
    ui->tableViewPosts->updateActivePostFilter();
}

void ZapFR::Client::TableViewScriptFolders::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
        removeScriptFolder();
    }
    else
    {
        QTableView::keyPressEvent(event);
    }
}

void ZapFR::Client::TableViewScriptFolders::reload(bool forceReload)
{
    // lambda for the callback, retrieving the script folders
    auto processScriptFolders = [&](uint64_t sourceID, const std::vector<ZapFR::Engine::ScriptFolder*>& scriptFolders)
    {
        QList<QList<QStandardItem*>> rows;
        for (const auto& scriptFolder : scriptFolders)
        {
            auto titleItem = new QStandardItem(QString::fromUtf8(scriptFolder->title()));
            titleItem->setData(QVariant::fromValue<uint64_t>(scriptFolder->id()), Role::ID);
            titleItem->setData(QVariant::fromValue<uint64_t>(sourceID), Role::SourceID);
            titleItem->setData(QVariant::fromValue<bool>(scriptFolder->showTotal()), Role::ShowTotal);
            titleItem->setData(QVariant::fromValue<bool>(scriptFolder->showUnread()), Role::ShowUnread);
            titleItem->setData(QVariant::fromValue<uint64_t>(scriptFolder->totalPostCount()), Role::TotalPostCount);
            titleItem->setData(QVariant::fromValue<uint64_t>(scriptFolder->totalUnreadCount()), Role::TotalUnreadCount);
            titleItem->setData(QString("ID: %1").arg(scriptFolder->id()), Qt::ToolTipRole);

            QList<QStandardItem*> rowData;
            rowData << titleItem;
            rows << rowData;
        }
        QMetaObject::invokeMethod(this, [=, this]() { populateScriptFolders(sourceID, rows); });
    };

    auto index = mMainWindow->treeViewSources()->currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(TreeViewSources::Role::ParentSourceID).toULongLong();
        if (forceReload || sourceID != mMainWindow->treeViewSources()->previouslySelectedSourceID())
        {
            // store currently selected script folder
            auto scriptFolderIndex = currentIndex();
            if (scriptFolderIndex.isValid())
            {
                mPreviouslySelectedScriptFolderID = scriptFolderIndex.data(Role::ID).toULongLong();
            }
            ZapFR::Engine::Agent::getInstance()->queueGetScriptFolders(sourceID, processScriptFolders);
        }
    }
}

void ZapFR::Client::TableViewScriptFolders::populateScriptFolders(uint64_t sourceID, const QList<QList<QStandardItem*>>& scriptFolders)
{
    mMainWindow->treeViewSources()->setPreviouslySelectedSourceID(sourceID);
    mItemModelScriptFolders = std::make_unique<QStandardItemModel>(this);
    setModel(mItemModelScriptFolders.get());
    auto headerItem = new QStandardItem(tr("Script folders"));
    headerItem->setTextAlignment(Qt::AlignLeft);
    mItemModelScriptFolders->setHorizontalHeaderItem(Column::TitleCol, headerItem);
    for (const auto& scriptFolder : scriptFolders)
    {
        mItemModelScriptFolders->appendRow(scriptFolder);
    }
    horizontalHeader()->setSectionResizeMode(Column::TitleCol, QHeaderView::Stretch);

    // restore previously selected script folder
    if (mPreviouslySelectedScriptFolderID != 0)
    {
        for (auto i = 0; i < mItemModelScriptFolders->rowCount(); ++i)
        {
            auto row = mItemModelScriptFolders->index(i, 0);
            if (row.isValid() && row.data(Role::ID).toULongLong() == mPreviouslySelectedScriptFolderID)
            {
                setCurrentIndex(row);
                break;
            }
        }

        mPreviouslySelectedScriptFolderID = 0;
    }
}

void ZapFR::Client::TableViewScriptFolders::updateBadges(uint64_t sourceID, const std::unordered_map<uint64_t, std::tuple<uint64_t, uint64_t>>& totalAndUnreadCounts)
{
    for (int32_t i = 0; i < mItemModelScriptFolders->rowCount(); ++i)
    {
        auto index = mItemModelScriptFolders->index(i, 0);
        if (index.isValid())
        {
            auto item = mItemModelScriptFolders->itemFromIndex(index);
            if (sourceID == item->data(Role::SourceID).toULongLong())
            {
                if (item->data(Role::ShowTotal).toBool())
                {
                    item->setData(QVariant::fromValue<uint64_t>(0), Role::TotalPostCount);
                }
                if (item->data(Role::ShowUnread).toBool())
                {
                    item->setData(QVariant::fromValue<uint64_t>(0), Role::TotalUnreadCount);
                }

                auto currentScriptFolderID = item->data(Role::ID).toULongLong();
                if (totalAndUnreadCounts.contains(currentScriptFolderID))
                {
                    auto [newTotalCount, newUnreadCount] = totalAndUnreadCounts.at(currentScriptFolderID);
                    item->setData(QVariant::fromValue<uint64_t>(newTotalCount), Role::TotalPostCount);
                    item->setData(QVariant::fromValue<uint64_t>(newUnreadCount), Role::TotalUnreadCount);
                }
            }
        }
    }
}

void ZapFR::Client::TableViewScriptFolders::addScriptFolder()
{
    auto index = mMainWindow->treeViewSources()->currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(TreeViewSources::Role::ParentSourceID).toULongLong();

        auto dialog = editScriptFolderDialog();
        dialog->reset(DialogEditScriptFolder::DisplayMode::Add, sourceID, 0, "", false, false);
        dialog->open();
    }
}

void ZapFR::Client::TableViewScriptFolders::editScriptFolder()
{
    auto index = currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(Role::SourceID).toULongLong();
        auto scriptFolderID = index.data(Role::ID).toULongLong();
        auto title = index.data(Qt::DisplayRole).toString();
        auto showTotal = index.data(Role::ShowTotal).toBool();
        auto showUnread = index.data(Role::ShowUnread).toBool();

        auto dialog = editScriptFolderDialog();
        dialog->reset(DialogEditScriptFolder::DisplayMode::Edit, sourceID, scriptFolderID, title, showTotal, showUnread);
        dialog->open();
    }
}

void ZapFR::Client::TableViewScriptFolders::removeScriptFolder()
{
    auto index = currentIndex();
    if (index.isValid())
    {
        QMessageBox mb(this);
        mb.setWindowTitle(tr("Remove script folder"));
        mb.setInformativeText(tr("Are you sure you want to remove this script folder? All posts in this folder will remain available, only the folder will be removed."));
        mb.setIcon(QMessageBox::Icon::Warning);
        mb.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        mb.button(QMessageBox::StandardButton::Ok)->setText(tr("Remove"));
        auto mbLayout = qobject_cast<QGridLayout*>(mb.layout());
        mbLayout->addItem(new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), mbLayout->rowCount(), 0, 1, mbLayout->columnCount());
        mb.exec();
        if (mb.clickedButton() == mb.button(QMessageBox::StandardButton::Ok))
        {
            auto scriptSourceID = index.data(Role::SourceID).toULongLong();
            auto scriptFolderID = index.data(Role::ID).toULongLong();
            ZapFR::Engine::Agent::getInstance()->queueRemoveScriptFolder(scriptSourceID, scriptFolderID,
                                                                         [&](uint64_t /*removedSourceID*/, uint64_t /*removedScriptFolderID*/)
                                                                         {
                                                                             QMetaObject::invokeMethod(this,
                                                                                                       [=, this]()
                                                                                                       {
                                                                                                           reload(true);
                                                                                                           setCurrentIndex(QModelIndex());
                                                                                                           mMainWindow->getUI()->tableViewPosts->reload();
                                                                                                           mMainWindow->setStatusBarMessage(tr("Scriptfolder removed"));
                                                                                                       });
                                                                         });
        }
    }
}

ZapFR::Client::DialogEditScriptFolder* ZapFR::Client::TableViewScriptFolders::editScriptFolderDialog()
{
    if (mDialogEditScriptFolder == nullptr)
    {
        mDialogEditScriptFolder = std::make_unique<DialogEditScriptFolder>(mMainWindow);
        connect(mDialogEditScriptFolder.get(), &DialogEditScriptFolder::accepted,
                [&]()
                {
                    auto sourceID = mDialogEditScriptFolder->sourceID();
                    auto scriptFolderID = mDialogEditScriptFolder->id();
                    auto title = mDialogEditScriptFolder->title().toStdString();
                    auto showTotal = mDialogEditScriptFolder->showTotal();
                    auto showUnread = mDialogEditScriptFolder->showUnread();
                    if (!title.empty())
                    {
                        switch (mDialogEditScriptFolder->displayMode())
                        {
                            case DialogEditScriptFolder::DisplayMode::Add:
                            {
                                ZapFR::Engine::Agent::getInstance()->queueAddScriptFolder(sourceID, title, showTotal, showUnread,
                                                                                          [&](uint64_t /*addedSourceID*/)
                                                                                          { QMetaObject::invokeMethod(this, [=, this]() { reload(true); }); });
                                break;
                            }
                            case DialogEditScriptFolder::DisplayMode::Edit:
                            {
                                ZapFR::Engine::Agent::getInstance()->queueUpdateScriptFolder(sourceID, scriptFolderID, title, showTotal, showUnread,
                                                                                             [&](uint64_t /*updatedSourceID*/, uint64_t /*updatedScriptFolderID*/)
                                                                                             { QMetaObject::invokeMethod(this, [=, this]() { reload(true); }); });
                                break;
                            }
                        }
                    }
                });
    }
    return mDialogEditScriptFolder.get();
}

std::unordered_map<uint64_t, QString> ZapFR::Client::TableViewScriptFolders::getIDToTitleMapping() const
{
    std::unordered_map<uint64_t, QString> map;
    for (int32_t i = 0; i < mItemModelScriptFolders->rowCount(); ++i)
    {
        auto child = mItemModelScriptFolders->index(i, 0);
        auto scriptFolderTitle = child.data(Qt::DisplayRole).toString();
        auto scriptFolderID = child.data(Role::ID).toULongLong();
        map[scriptFolderID] = scriptFolderTitle;
    }
    return map;
}