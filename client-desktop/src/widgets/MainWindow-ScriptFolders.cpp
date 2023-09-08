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
#include "ZapFR/base/ScriptFolder.h"
#include "dialogs/DialogEditScriptFolder.h"
#include "widgets/MainWindow.h"

void ZapFR::Client::MainWindow::reloadScriptFolders(bool forceReload)
{
    // lambda for the callback, retrieving the script folders
    auto processScriptFolders = [&](uint64_t sourceID, const std::vector<ZapFR::Engine::ScriptFolder*>& scriptFolders)
    {
        QList<QList<QStandardItem*>> rows;
        for (const auto& scriptFolder : scriptFolders)
        {
            auto titleItem = new QStandardItem(QString::fromUtf8(scriptFolder->title()));
            titleItem->setData(QVariant::fromValue<uint64_t>(scriptFolder->id()), ScriptFolderIDRole);
            titleItem->setData(QVariant::fromValue<uint64_t>(sourceID), ScriptFolderSourceIDRole);
            titleItem->setData(QString("ID: %1").arg(scriptFolder->id()), Qt::ToolTipRole);

            QList<QStandardItem*> rowData;
            rowData << titleItem;
            rows << rowData;
        }
        QMetaObject::invokeMethod(this, "populateScriptFolders", Qt::AutoConnection, sourceID, rows);
    };

    auto index = ui->treeViewSources->currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
        if (forceReload || sourceID != mPreviouslySelectedSourceID)
        {
            ZapFR::Engine::Agent::getInstance()->queueGetScriptFolders(sourceID, processScriptFolders);
        }
    }
}

void ZapFR::Client::MainWindow::populateScriptFolders(uint64_t sourceID, const QList<QList<QStandardItem*>>& scriptFolders)
{
    mPreviouslySelectedSourceID = sourceID;
    mItemModelScriptFolders = std::make_unique<QStandardItemModel>(this);
    ui->tableViewScriptFolders->setModel(mItemModelScriptFolders.get());
    auto headerItem = new QStandardItem(tr("Script folders"));
    headerItem->setTextAlignment(Qt::AlignLeft);
    mItemModelScriptFolders->setHorizontalHeaderItem(ScriptFolderColumnTitle, headerItem);
    for (const auto& scriptFolder : scriptFolders)
    {
        mItemModelScriptFolders->appendRow(scriptFolder);
    }
    ui->tableViewScriptFolders->horizontalHeader()->setSectionResizeMode(ScriptFolderColumnTitle, QHeaderView::Stretch);
}

void ZapFR::Client::MainWindow::addScriptFolder()
{
    auto index = ui->treeViewSources->currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();

        auto dialog = editScriptFolderDialog();
        dialog->reset(DialogEditScriptFolder::DisplayMode::Add, sourceID, 0, "");
        dialog->open();
    }
}

void ZapFR::Client::MainWindow::editScriptFolder()
{
    auto index = ui->tableViewScriptFolders->currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(ScriptFolderSourceIDRole).toULongLong();
        auto scriptFolderID = index.data(ScriptFolderIDRole).toULongLong();
        auto title = index.data(Qt::DisplayRole).toString();

        auto dialog = editScriptFolderDialog();
        dialog->reset(DialogEditScriptFolder::DisplayMode::Edit, sourceID, scriptFolderID, title);
        dialog->open();
    }
}

void ZapFR::Client::MainWindow::removeScriptFolder()
{
    auto index = ui->tableViewScriptFolders->currentIndex();
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
            auto scriptSourceID = index.data(ScriptFolderSourceIDRole).toULongLong();
            auto scriptFolderID = index.data(ScriptFolderIDRole).toULongLong();
            ZapFR::Engine::Agent::getInstance()->queueRemoveScriptFolder(
                scriptSourceID, scriptFolderID,
                [&](uint64_t removedSourceID, uint64_t removedScriptFolderID)
                { QMetaObject::invokeMethod(this, "scriptFolderRemoved", Qt::AutoConnection, removedSourceID, removedScriptFolderID); });
        }
    }
}

ZapFR::Client::DialogEditScriptFolder* ZapFR::Client::MainWindow::editScriptFolderDialog()
{
    if (mDialogEditScriptFolder == nullptr)
    {
        mDialogEditScriptFolder = std::make_unique<DialogEditScriptFolder>(this);
        connect(mDialogEditScriptFolder.get(), &DialogEditScriptFolder::accepted,
                [&]()
                {
                    auto sourceID = mDialogEditScriptFolder->sourceID();
                    auto scriptFolderID = mDialogEditScriptFolder->id();
                    auto title = mDialogEditScriptFolder->title().toStdString();
                    if (!title.empty())
                    {
                        switch (mDialogEditScriptFolder->displayMode())
                        {
                            case DialogEditScriptFolder::DisplayMode::Add:
                            {
                                ZapFR::Engine::Agent::getInstance()->queueAddScriptFolder(
                                    sourceID, title, [&](uint64_t addedSourceID) { QMetaObject::invokeMethod(this, "scriptFolderAdded", Qt::AutoConnection, addedSourceID); });
                                break;
                            }
                            case DialogEditScriptFolder::DisplayMode::Edit:
                            {
                                ZapFR::Engine::Agent::getInstance()->queueUpdateScriptFolder(
                                    sourceID, scriptFolderID, title,
                                    [&](uint64_t updatedSourceID, uint64_t updatedScriptFolderID)
                                    { QMetaObject::invokeMethod(this, "scriptFolderUpdated", Qt::AutoConnection, updatedSourceID, updatedScriptFolderID); });
                                break;
                            }
                        }
                    }
                });
    }
    return mDialogEditScriptFolder.get();
}

void ZapFR::Client::MainWindow::scriptFolderAdded(uint64_t /*sourceID*/)
{
    reloadScriptFolders(true);
}

void ZapFR::Client::MainWindow::scriptFolderUpdated(uint64_t /*sourceID*/, uint64_t /*scriptFolderID*/)
{
    reloadScriptFolders(true);
}

void ZapFR::Client::MainWindow::scriptFolderRemoved(uint64_t /*sourceID*/, uint64_t /*scriptFolderID*/)
{
    reloadScriptFolders(true);
    ui->tableViewScriptFolders->setCurrentIndex(QModelIndex());
    reloadPosts();
}

void ZapFR::Client::MainWindow::connectScriptFolderStuff()
{
    connect(ui->tableViewScriptFolders, &TableViewScriptFolders::deletePressed, [&]() { removeScriptFolder(); });

    connect(ui->tableViewScriptFolders, &TableViewScriptFolders::selectedScriptFolderChanged,
            [&](const QModelIndex& index)
            {
                if (index.isValid())
                {
                    reloadPosts();
                }
                updateActivePostFilter();
            });

    connect(ui->tableViewScriptFolders, &QTableView::doubleClicked, this, &MainWindow::editScriptFolder);
    connect(ui->action_Edit_script_folder, &QAction::triggered, this, &MainWindow::editScriptFolder);
    connect(ui->action_Remove_script_folder, &QAction::triggered, this, &MainWindow::removeScriptFolder);
    connect(ui->action_Add_script_folder, &QAction::triggered, this, &MainWindow::addScriptFolder);

    connect(ui->tableViewScriptFolders, &TableViewScripts::customContextMenuRequested,
            [&](const QPoint& p) { mScriptFolderContextMenu->popup(ui->tableViewScriptFolders->viewport()->mapToGlobal(p)); });
}

void ZapFR::Client::MainWindow::createScriptFolderContextMenus()
{
    mScriptFolderContextMenu = std::make_unique<QMenu>(nullptr);
    mScriptFolderContextMenu->addAction(ui->action_Add_script_folder);
    mScriptFolderContextMenu->addAction(ui->action_Edit_script_folder);
    mScriptFolderContextMenu->addSeparator();
    mScriptFolderContextMenu->addAction(ui->action_Remove_script_folder);
}
