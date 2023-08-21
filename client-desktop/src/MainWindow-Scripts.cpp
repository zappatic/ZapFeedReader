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
#include "DialogEditScript.h"
#include "MainWindow.h"
#include "ZapFR/Agent.h"
#include "ZapFR/Script.h"

void ZapFR::Client::MainWindow::reloadScripts(bool forceReload)
{
    // lambda to assign the correct role data to the table entries
    auto setItemData = [&](QStandardItem* item, ZapFR::Engine::Script* script, uint64_t sourceID)
    {
        item->setData(QVariant::fromValue<uint64_t>(script->id()), ScriptIDRole);
        item->setData(QVariant::fromValue<uint64_t>(sourceID), ScriptSourceIDRole);
        item->setData(script->isEnabled(), ScriptIsEnabledRole);
        item->setData(QString::fromUtf8(script->filename()), ScriptFilenameRole);

        QVariantList events;
        for (const auto& event : script->runOnEvents())
        {
            events << QVariant(static_cast<std::underlying_type_t<ZapFR::Engine::Script::Event>>(event));
        }
        item->setData(events, ScriptRunOnEventsRole);

        QVariantList feeds;
        const auto& runOnFeedIDs = script->runOnFeedIDs();
        if (runOnFeedIDs.has_value())
        {
            for (const auto& feed : runOnFeedIDs.value())
            {
                feeds << QVariant::fromValue<uint64_t>(feed);
            }
            item->setData(feeds, ScriptRunOnFeedIDsRole);
        }
    };

    // lambda for the callback, retrieving the scripts
    auto processScripts = [&](uint64_t sourceID, const std::vector<ZapFR::Engine::Script*> scripts)
    {
        QList<QList<QStandardItem*>> rows;
        for (const auto& script : scripts)
        {
            auto idItem = new QStandardItem(QString::number(script->id()));
            setItemData(idItem, script, sourceID);

            auto typeItem = new QStandardItem();
            switch (script->type())
            {
                case ZapFR::Engine::Script::Type::Lua:
                {
                    typeItem->setText("Lua");
                    break;
                }
            }
            setItemData(typeItem, script, sourceID);

            auto titleItem = new QStandardItem(QString::fromUtf8(script->filename()));
            setItemData(titleItem, script, sourceID);

            auto isEnabledItem = new QStandardItem();
            setItemData(isEnabledItem, script, sourceID);

            auto runOnEventsItem = new QStandardItem("todo");
            setItemData(runOnEventsItem, script, sourceID);

            auto runOnFeedIDsItem = new QStandardItem("todo");
            setItemData(runOnFeedIDsItem, script, sourceID);

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
        if (forceReload || sourceID != mPreviouslySelectedSourceID)
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

void ZapFR::Client::MainWindow::connectScriptStuff()
{
    connect(ui->tableViewScripts, &QTableView::doubleClicked,
            [&](const QModelIndex& index)
            {
                if (index.isValid())
                {
                    if (mDialogEditScript == nullptr)
                    {
                        mDialogEditScript = std::make_unique<DialogEditScript>(this);
                        connect(mDialogEditScript.get(), &DialogEditScript::accepted,
                                [&]()
                                {
                                    auto scriptSourceID = mDialogEditScript->scriptSourceID();
                                    auto scriptID = mDialogEditScript->scriptID();
                                    auto type = ZapFR::Engine::Script::Type::Lua; // forced to Lua for now
                                    auto filename = mDialogEditScript->filename().toStdString();
                                    auto enabled = mDialogEditScript->isScriptEnabled();
                                    auto selectedEvents = mDialogEditScript->runOnEvents();
                                    std::optional<std::unordered_set<uint64_t>> selectedFeedIDs;
                                    if (!mDialogEditScript->runOnAllFeeds())
                                    {
                                        selectedFeedIDs = mDialogEditScript->runOnFeedIDs();
                                    }

                                    ZapFR::Engine::Agent::getInstance()->queueUpdateScript(
                                        scriptSourceID, scriptID, type, filename, enabled, selectedEvents, selectedFeedIDs,
                                        [&](uint64_t updatedSourceID, uint64_t updatedScriptID)
                                        { QMetaObject::invokeMethod(this, "scriptUpdated", Qt::AutoConnection, updatedSourceID, updatedScriptID); });
                                });
                    }
                    auto scriptID = index.data(ScriptIDRole).toULongLong();
                    auto scriptSourceID = index.data(ScriptSourceIDRole).toULongLong();
                    auto scriptFilename = index.data(ScriptFilenameRole).toString();
                    auto scriptIsEnabled = index.data(ScriptIsEnabledRole).toBool();
                    std::unordered_set<ZapFR::Engine::Script::Event> events;
                    for (const auto& eventVariant : index.data(ScriptRunOnEventsRole).toList())
                    {
                        events.insert(eventVariant.value<ZapFR::Engine::Script::Event>());
                    }
                    std::optional<std::unordered_set<uint64_t>> feedIDs{};
                    auto feedIDsVariants = index.data(ScriptRunOnFeedIDsRole);
                    if (!feedIDsVariants.isNull() && feedIDsVariants.isValid())
                    {
                        feedIDs = std::unordered_set<uint64_t>();
                        for (const auto& feedIDVariant : feedIDsVariants.toList())
                        {
                            feedIDs.value().insert(feedIDVariant.value<uint64_t>());
                        }
                    }

                    mDialogEditScript->reset(DialogEditScript::DisplayMode::Edit, scriptSourceID, scriptID, scriptFilename, scriptIsEnabled, events, feedIDs);
                    mDialogEditScript->open();
                }
            });
}

void ZapFR::Client::MainWindow::scriptUpdated(uint64_t /*sourceID*/, uint64_t /*scriptID*/)
{
    reloadScripts(true);
}
