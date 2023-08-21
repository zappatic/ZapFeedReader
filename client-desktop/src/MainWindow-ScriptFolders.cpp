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
#include "MainWindow.h"
#include "ZapFR/Agent.h"
#include "ZapFR/ScriptFolder.h"

void ZapFR::Client::MainWindow::reloadScriptFolders()
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
        if (sourceID != mPreviouslySelectedSourceID)
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

void ZapFR::Client::MainWindow::connectScriptFolderStuff()
{
    connect(ui->tableViewScriptFolders, &TableViewScriptFolders::selectedScriptFolderChanged,
            [&](const QModelIndex& index)
            {
                if (index.isValid())
                {
                    reloadPosts();
                }
            });
}