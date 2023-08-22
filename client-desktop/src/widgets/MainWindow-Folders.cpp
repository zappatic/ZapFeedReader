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
#include "widgets/MainWindow.h"
#include "ZapFR/Agent.h"
#include "dialogs/DialogAddFolder.h"

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
                        ZapFR::Engine::Agent::getInstance()->queueAddFolder(mDialogAddFolder->selectedSourceID(), mDialogAddFolder->selectedFolderID(),
                                                                            mDialogAddFolder->title().toStdString(),
                                                                            [&]() { QMetaObject::invokeMethod(this, "folderAdded", Qt::AutoConnection); });
                    }
                });
    }

    auto [sourceID, folderID] = getCurrentlySelectedSourceAndFolderID();
    mDialogAddFolder->reset(sourceID, folderID);
    mDialogAddFolder->open();
}

void ZapFR::Client::MainWindow::removeFolder()
{
    QMessageBox messageBox;
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
            ZapFR::Engine::Agent::getInstance()->queueRemoveFolder(sourceID, folder, [&]() { QMetaObject::invokeMethod(this, "folderRemoved", Qt::AutoConnection); });
        }
    }
}

void ZapFR::Client::MainWindow::folderMoved()
{
    reloadSources();
}

void ZapFR::Client::MainWindow::folderAdded()
{
    reloadSources();
}

void ZapFR::Client::MainWindow::folderRemoved()
{
    reloadSources();
    populatePosts();
}

void ZapFR::Client::MainWindow::folderMarkedRead(uint64_t sourceID, std::unordered_set<uint64_t> feedIDs)
{
    updateFeedUnreadCountBadge(sourceID, feedIDs, false, 0);
    mCurrentPostPage = 1;
    reloadPosts();
}

void ZapFR::Client::MainWindow::connectFolderStuff()
{
    connect(ui->action_Add_folder, &QAction::triggered, this, &MainWindow::addFolder);
    connect(ui->action_Remove_folder, &QAction::triggered, this, &MainWindow::removeFolder);
}

void ZapFR::Client::MainWindow::createFolderContextMenus()
{
    mSourceContextMenuFolder = std::make_unique<QMenu>(nullptr);
    mSourceContextMenuFolder->addAction(ui->action_Refresh_feeds);
    mSourceContextMenuFolder->addAction(ui->action_Mark_as_read);
    mSourceContextMenuFolder->addSeparator();
    mSourceContextMenuFolder->addAction(ui->action_Add_feed);
    mSourceContextMenuFolder->addAction(ui->action_Add_folder);
    mSourceContextMenuFolder->addAction(ui->action_View_logs);
    mSourceContextMenuFolder->addSeparator();
    mSourceContextMenuFolder->addAction(ui->action_Remove_folder);
}
