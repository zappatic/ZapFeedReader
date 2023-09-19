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

#include "./ui_MainWindow.h"
#include "ZapFR/Agent.h"
#include "dialogs/DialogAddFolder.h"
#include "dialogs/DialogEditFolder.h"
#include "models/StandardItemModelSources.h"
#include "widgets/MainWindow.h"

void ZapFR::Client::MainWindow::addFolder()
{
    if (mDialogAddFolder == nullptr)
    {
        mDialogAddFolder = std::make_unique<DialogAddFolder>(this);
        connect(mDialogAddFolder.get(), &QDialog::finished,
                [&](int result)
                {
                    if (result == QDialog::DialogCode::Accepted)
                    {
                        auto sourceID = mDialogAddFolder->selectedSourceID();
                        auto folderID = mDialogAddFolder->selectedFolderID();
                        auto title = mDialogAddFolder->title().toStdString();
                        if (!title.empty())
                        {
                            ZapFR::Engine::Agent::getInstance()->queueAddFolder(sourceID, folderID, title,
                                                                                [&]() { QMetaObject::invokeMethod(this, [=]() { folderAdded(); }); });
                        }
                    }
                });
    }

    auto [sourceID, folderID] = getCurrentlySelectedSourceAndFolderID();
    mDialogAddFolder->reset(sourceID, folderID);
    mDialogAddFolder->open();
}

void ZapFR::Client::MainWindow::editFolder()
{
    if (mDialogEditFolder == nullptr)
    {
        mDialogEditFolder = std::make_unique<DialogEditFolder>(this);
        connect(mDialogEditFolder.get(), &QDialog::finished,
                [&](int result)
                {
                    if (result == QDialog::DialogCode::Accepted)
                    {
                        auto sourceID = mDialogEditFolder->sourceID();
                        auto folderID = mDialogEditFolder->id();
                        auto title = mDialogEditFolder->title().toStdString();
                        if (!title.empty())
                        {
                            ZapFR::Engine::Agent::getInstance()->queueUpdateFolder(
                                sourceID, folderID, title,
                                [&](uint64_t affectedSourceID, uint64_t affectedFolderID, const std::string& updatedTitle)
                                { QMetaObject::invokeMethod(this, [=]() { folderUpdated(affectedSourceID, affectedFolderID, updatedTitle); }); });
                        }
                    }
                });
    }

    auto currentIndex = ui->treeViewSources->currentIndex();
    if (currentIndex.isValid())
    {
        auto type = currentIndex.data(SourceTreeEntryTypeRole);
        if (type == SOURCETREE_ENTRY_TYPE_FOLDER)
        {
            auto sourceID = currentIndex.data(SourceTreeEntryParentSourceIDRole).toULongLong();
            auto folderID = currentIndex.data(SourceTreeEntryIDRole).toULongLong();
            auto title = currentIndex.data(Qt::DisplayRole).toString();

            mDialogEditFolder->reset(sourceID, folderID, title);
            mDialogEditFolder->open();
        }
    }
}

void ZapFR::Client::MainWindow::removeFolder()
{
    QMessageBox messageBox(this);
    messageBox.setText(tr("Remove folder"));
    messageBox.setWindowTitle(tr("Remove folder"));
    messageBox.setInformativeText(tr("Are you sure you want to remove this folder, all its subfolders, and all feeds they contain? All associated posts will be removed!"));
    messageBox.setIcon(QMessageBox::Warning);
    auto yesButton = messageBox.addButton(QMessageBox::StandardButton::Yes);
    yesButton->setText(tr("Remove"));
    messageBox.addButton(QMessageBox::StandardButton::Cancel);
    auto messageBoxLayout = qobject_cast<QGridLayout*>(messageBox.layout());
    messageBoxLayout->addItem(new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), messageBoxLayout->rowCount(), 0, 1, messageBoxLayout->columnCount());
    messageBox.exec();
    if (messageBox.buttonRole(messageBox.clickedButton()) == QMessageBox::YesRole)
    {
        auto index = ui->treeViewSources->currentIndex();
        if (index.isValid())
        {
            auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
            auto folder = index.data(SourceTreeEntryIDRole).toULongLong();
            ZapFR::Engine::Agent::getInstance()->queueRemoveFolder(sourceID, folder, [&]() { QMetaObject::invokeMethod(this, [=]() { folderRemoved(); }); });
        }
    }
}

void ZapFR::Client::MainWindow::folderAdded()
{
    reloadSources();
    ui->statusbar->showMessage(tr("Folder added"), StatusBarDefaultTimeout);
}

void ZapFR::Client::MainWindow::folderUpdated(uint64_t sourceID, uint64_t folderID, const std::string& newTitle)
{
    std::function<void(QStandardItem*)> updateFolderTitle;
    updateFolderTitle = [&](QStandardItem* item)
    {
        for (int32_t i = 0; i < item->rowCount(); ++i)
        {
            auto child = item->child(i);
            auto childSourceID = child->data(SourceTreeEntryParentSourceIDRole).toULongLong();
            if (childSourceID != sourceID)
            {
                continue;
            }

            auto childType = child->data(SourceTreeEntryTypeRole).toULongLong();
            if (childType == SOURCETREE_ENTRY_TYPE_FOLDER)
            {
                auto childFolderID = child->data(SourceTreeEntryIDRole).toULongLong();
                if (childFolderID == folderID)
                {
                    child->setData(QString::fromUtf8(newTitle), Qt::DisplayRole);
                    return;
                }
                else
                {
                    updateFolderTitle(child);
                }
            }
            else if (childType == SOURCETREE_ENTRY_TYPE_SOURCE)
            {
                updateFolderTitle(child);
            }
        }
    };
    updateFolderTitle(mItemModelSources->invisibleRootItem());
}

void ZapFR::Client::MainWindow::folderRemoved()
{
    reloadSources();
    populatePosts();
    ui->statusbar->showMessage(tr("Folder removed"), StatusBarDefaultTimeout);
}

void ZapFR::Client::MainWindow::folderMarkedRead(uint64_t sourceID, std::unordered_set<uint64_t> feedIDs)
{
    updateFeedUnreadCountBadge(sourceID, feedIDs, false, 0);
    mCurrentPostPage = 1;
    reloadPosts();
    reloadScriptFolders(true);
    ui->statusbar->showMessage(tr("Folder marked as read"), StatusBarDefaultTimeout);
}

void ZapFR::Client::MainWindow::connectFolderStuff()
{
    connect(ui->action_Add_folder, &QAction::triggered, this, &MainWindow::addFolder);
    connect(ui->action_Remove_folder, &QAction::triggered, this, &MainWindow::removeFolder);
    connect(ui->action_Edit_folder, &QAction::triggered, this, &MainWindow::editFolder);
    connect(ui->treeViewSources, &TreeViewSources::folderDoubleClicked, this, &MainWindow::editFolder);
}

void ZapFR::Client::MainWindow::createFolderContextMenus()
{
    mSourceContextMenuFolder = std::make_unique<QMenu>(nullptr);
    mSourceContextMenuFolder->addAction(ui->action_Refresh);
    mSourceContextMenuFolder->addAction(ui->action_Mark_as_read);
    mSourceContextMenuFolder->addSeparator();
    mSourceContextMenuFolder->addAction(ui->action_Add_feed);
    mSourceContextMenuFolder->addAction(ui->action_Add_folder);
    mSourceContextMenuFolder->addAction(ui->action_Edit_folder);
    mSourceContextMenuFolder->addAction(ui->action_Remove_folder);
    mSourceContextMenuFolder->addSeparator();
    mSourceContextMenuFolder->addAction(ui->action_View_logs);
    mSourceContextMenuFolder->addAction(ui->action_View_properties);
}
