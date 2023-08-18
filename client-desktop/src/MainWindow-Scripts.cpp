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
#include "ZapFR/Script.h"

void ZapFR::Client::MainWindow::reloadScripts()
{
    // lambda for the callback, retrieving the scripts
    auto processScripts = [&](uint64_t sourceID, const std::vector<ZapFR::Engine::Script*> scripts)
    {
        QList<QList<QStandardItem*>> rows;
        for (const auto& script : scripts)
        {
            auto idItem = new QStandardItem(QString::number(script->id()));
            idItem->setData(QVariant::fromValue<uint64_t>(script->id()), ScriptIDRole);

            auto typeItem = new QStandardItem();
            switch (script->type())
            {
                case ZapFR::Engine::Script::Type::Lua:
                {
                    typeItem->setText("Lua");
                    typeItem->setData(QVariant::fromValue<uint64_t>(script->id()), ScriptIDRole);
                    break;
                }
            }

            auto titleItem = new QStandardItem(QString::fromUtf8(script->filename()));
            titleItem->setData(QVariant::fromValue<uint64_t>(script->id()), ScriptIDRole);

            auto isEnabledItem = new QStandardItem();
            isEnabledItem->setData(QVariant::fromValue<uint64_t>(script->id()), ScriptIDRole);
            isEnabledItem->setData(script->isEnabled(), ScriptIsEnabledRole);

            auto runOnEventsItem = new QStandardItem("todo");
            runOnEventsItem->setData(QVariant::fromValue<uint64_t>(script->id()), ScriptIDRole);

            auto runOnFeedIDsItem = new QStandardItem("todo");
            runOnFeedIDsItem->setData(QVariant::fromValue<uint64_t>(script->id()), ScriptIDRole);

            QList<QStandardItem*> rowData;
            rowData << idItem << typeItem << titleItem << isEnabledItem << runOnEventsItem << runOnFeedIDsItem;
            rows << rowData;
        }

        QMetaObject::invokeMethod(this, "populateScripts", Qt::AutoConnection, rows);
        mPreviouslySelectedSourceID = sourceID;
    };

    auto index = ui->treeViewSources->currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
        if (sourceID != mPreviouslySelectedSourceID)
        {
            ZapFR::Engine::Agent::getInstance()->queueGetScripts(sourceID, processScripts);
        }
    }
}

void ZapFR::Client::MainWindow::populateScripts(const QList<QList<QStandardItem*>>& scripts)
{
    ui->stackedWidgetRight->setCurrentIndex(StackedPaneScripts);

    mItemModelScripts = std::make_unique<QStandardItemModel>(this);
    ui->tableViewScripts->setModel(mItemModelScripts.get());
    mItemModelScripts->setHorizontalHeaderItem(ScriptsColumnID, new QStandardItem(tr("ID")));
    mItemModelScripts->setHorizontalHeaderItem(ScriptsColumnType, new QStandardItem(tr("Type")));
    mItemModelScripts->setHorizontalHeaderItem(ScriptsColumnFilename, new QStandardItem(tr("Filename")));
    mItemModelScripts->setHorizontalHeaderItem(ScriptsColumnIsEnabled, new QStandardItem(tr("Enabled")));
    mItemModelScripts->setHorizontalHeaderItem(ScriptsColumnRunOnEvents, new QStandardItem(tr("Run on event(s)")));
    mItemModelScripts->setHorizontalHeaderItem(ScriptsColumnRunOnFeedIDs, new QStandardItem(tr("Run on feed ID(s)")));
    for (const auto& script : scripts)
    {
        mItemModelScripts->appendRow(script);
    }
    ui->tableViewScripts->horizontalHeader()->setSectionResizeMode(ScriptsColumnFilename, QHeaderView::Stretch);
    ui->tableViewScripts->horizontalHeader()->resizeSection(ScriptsColumnID, 40);
    ui->tableViewScripts->horizontalHeader()->resizeSection(ScriptsColumnType, 50);
    ui->tableViewScripts->horizontalHeader()->resizeSection(ScriptsColumnIsEnabled, 75);
    ui->tableViewScripts->horizontalHeader()->resizeSection(ScriptsColumnRunOnEvents, 150);
    ui->tableViewScripts->horizontalHeader()->resizeSection(ScriptsColumnRunOnFeedIDs, 150);
}
