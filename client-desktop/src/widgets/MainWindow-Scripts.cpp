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
#include "ZapFR/base/Script.h"
#include "delegates/ItemDelegateScript.h"
#include "dialogs/DialogEditScript.h"
#include "widgets/MainWindow.h"

void ZapFR::Client::MainWindow::reloadScripts(bool forceReload)
{
    // lambda to assign the correct role data to the table entries
    auto setItemData = [&](QStandardItem* item, ZapFR::Engine::Script* script, uint64_t sourceID)
    {
        item->setData(QVariant::fromValue<uint64_t>(script->id()), ScriptIDRole);
        item->setData(QVariant::fromValue<uint64_t>(sourceID), ScriptSourceIDRole);
        item->setData(script->isEnabled(), ScriptIsEnabledRole);
        item->setData(QString::fromUtf8(script->title()), ScriptTitleRole);
        item->setData(QString::fromUtf8(script->script()), ScriptScriptRole);

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

            auto titleItem = new QStandardItem(QString::fromUtf8(script->title()));
            setItemData(titleItem, script, sourceID);

            auto isEnabledItem = new QStandardItem();
            setItemData(isEnabledItem, script, sourceID);

            QStringList eventsList;
            auto events = script->runOnEvents();
            if (events.contains(ZapFR::Engine::Script::Event::NewPost))
            {
                eventsList << tr("New post");
            }
            if (events.contains(ZapFR::Engine::Script::Event::UpdatePost))
            {
                eventsList << tr("Update post");
            }
            if (eventsList.isEmpty())
            {
                eventsList << tr("None");
            }
            auto runOnEventsItem = new QStandardItem(eventsList.join(", "));
            runOnEventsItem->setData(QVariant::fromValue<uint64_t>(events.size()), ScriptEventCountRole);
            setItemData(runOnEventsItem, script, sourceID);

            QStandardItem* runOnFeedIDsItem{nullptr};
            auto feeds = script->runOnFeedIDs();
            if (feeds.has_value())
            {
                runOnFeedIDsItem = new QStandardItem(tr("%n feed(s)", "", static_cast<int32_t>(feeds.value().size())));
            }
            else
            {
                runOnFeedIDsItem = new QStandardItem(tr("All feeds"));
            }
            setItemData(runOnFeedIDsItem, script, sourceID);

            QList<QStandardItem*> rowData;
            rowData << typeItem << titleItem << isEnabledItem << runOnEventsItem << runOnFeedIDsItem;
            rows << rowData;
        }

        QMetaObject::invokeMethod(this, [&]() { populateScripts(rows); });
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
    ui->stackedWidgetContentPanes->setCurrentIndex(StackedPaneScripts);

    mItemModelScripts = std::make_unique<QStandardItemModel>(this);
    ui->tableViewScripts->setModel(mItemModelScripts.get());
    mItemModelScripts->setHorizontalHeaderItem(ScriptsColumnType, new QStandardItem(tr("Type")));
    mItemModelScripts->setHorizontalHeaderItem(ScriptsColumnTitle, new QStandardItem(tr("Title")));
    mItemModelScripts->setHorizontalHeaderItem(ScriptsColumnIsEnabled, new QStandardItem(tr("Enabled")));
    mItemModelScripts->setHorizontalHeaderItem(ScriptsColumnRunOnEvents, new QStandardItem(tr("Run on event(s)")));
    mItemModelScripts->setHorizontalHeaderItem(ScriptsColumnRunOnFeedIDs, new QStandardItem(tr("Run on feed(s)")));
    for (const auto& script : scripts)
    {
        mItemModelScripts->appendRow(script);
    }
    ui->tableViewScripts->horizontalHeader()->setSectionResizeMode(ScriptsColumnTitle, QHeaderView::Stretch);
    ui->tableViewScripts->horizontalHeader()->resizeSection(ScriptsColumnType, 50);
    ui->tableViewScripts->horizontalHeader()->resizeSection(ScriptsColumnIsEnabled, 75);
    ui->tableViewScripts->horizontalHeader()->resizeSection(ScriptsColumnRunOnEvents, 175);
    ui->tableViewScripts->horizontalHeader()->resizeSection(ScriptsColumnRunOnFeedIDs, 150);
}

ZapFR::Client::DialogEditScript* ZapFR::Client::MainWindow::editScriptDialog()
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
                    auto title = mDialogEditScript->title().toStdString();
                    auto enabled = mDialogEditScript->isScriptEnabled();
                    auto selectedEvents = mDialogEditScript->runOnEvents();
                    std::optional<std::unordered_set<uint64_t>> selectedFeedIDs;
                    if (!mDialogEditScript->runOnAllFeeds())
                    {
                        selectedFeedIDs = mDialogEditScript->runOnFeedIDs();
                    }
                    auto script = mDialogEditScript->script().toStdString();

                    switch (mDialogEditScript->displayMode())
                    {
                        case DialogEditScript::DisplayMode::Add:
                        {
                            ZapFR::Engine::Agent::getInstance()->queueAddScript(scriptSourceID, type, title, enabled, selectedEvents, selectedFeedIDs, script,
                                                                                [&](uint64_t addedSourceID)
                                                                                { QMetaObject::invokeMethod(this, [&]() { scriptAdded(addedSourceID); }); });
                            break;
                        }
                        case DialogEditScript::DisplayMode::Edit:
                        {
                            ZapFR::Engine::Agent::getInstance()->queueUpdateScript(
                                scriptSourceID, scriptID, type, title, enabled, selectedEvents, selectedFeedIDs, script,
                                [&](uint64_t updatedSourceID, uint64_t updatedScriptID)
                                { QMetaObject::invokeMethod(this, [&]() { scriptUpdated(updatedSourceID, updatedScriptID); }); });
                            break;
                        }
                    }
                });
    }
    return mDialogEditScript.get();
}

void ZapFR::Client::MainWindow::editScript()
{
    auto index = ui->tableViewScripts->currentIndex();
    if (index.isValid())
    {
        auto scriptID = index.data(ScriptIDRole).toULongLong();
        auto scriptSourceID = index.data(ScriptSourceIDRole).toULongLong();
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
        auto scriptTitle = index.data(ScriptTitleRole).toString();
        auto script = index.data(ScriptScriptRole).toString();

        // TODO :better to agent->getScript()

        auto dialog = editScriptDialog();
        dialog->reset(DialogEditScript::DisplayMode::Edit, scriptSourceID, scriptID, scriptTitle, scriptIsEnabled, events, feedIDs, script);
        dialog->open();
    }
}

void ZapFR::Client::MainWindow::removeScript()
{
    auto index = ui->tableViewScripts->currentIndex();
    if (index.isValid())
    {
        QMessageBox mb(this);
        mb.setWindowTitle(tr("Remove script"));
        mb.setInformativeText(
            tr("Are you sure you want to remove this script? Only the reference will be removed, the script file itself will remain in the scripts folder."));
        mb.setIcon(QMessageBox::Icon::Warning);
        mb.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        mb.button(QMessageBox::StandardButton::Ok)->setText(tr("Remove"));
        auto mbLayout = qobject_cast<QGridLayout*>(mb.layout());
        mbLayout->addItem(new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), mbLayout->rowCount(), 0, 1, mbLayout->columnCount());
        mb.exec();
        if (mb.clickedButton() == mb.button(QMessageBox::StandardButton::Ok))
        {
            auto scriptID = index.data(ScriptIDRole).toULongLong();
            auto scriptSourceID = index.data(ScriptSourceIDRole).toULongLong();
            ZapFR::Engine::Agent::getInstance()->queueRemoveScript(scriptSourceID, scriptID,
                                                                   [&](uint64_t removedSourceID, uint64_t removedScriptID)
                                                                   { QMetaObject::invokeMethod(this, [&]() { scriptRemoved(removedSourceID, removedScriptID); }); });
        }
    }
}

void ZapFR::Client::MainWindow::addScript()
{
    auto index = ui->treeViewSources->currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();

        auto dialog = editScriptDialog();
        dialog->reset(DialogEditScript::DisplayMode::Add, sourceID, 0, "", true, {ZapFR::Engine::Script::Event::NewPost, ZapFR::Engine::Script::Event::UpdatePost}, {}, "");
        dialog->open();
    }
}

void ZapFR::Client::MainWindow::scriptUpdated(uint64_t /*sourceID*/, uint64_t /*scriptID*/)
{
    reloadScripts(true);
}

void ZapFR::Client::MainWindow::scriptRemoved(uint64_t /*sourceID*/, uint64_t /*scriptID*/)
{
    reloadScripts(true);
    ui->statusbar->showMessage(tr("Script removed"), StatusBarDefaultTimeout);
}

void ZapFR::Client::MainWindow::scriptAdded(uint64_t /*sourceID*/)
{
    reloadScripts(true);
}

void ZapFR::Client::MainWindow::connectScriptStuff()
{
    connect(ui->action_View_scripts, &QAction::triggered,
            [&]()
            {
                mPreviouslySelectedSourceID = 0;
                reloadScripts();
            });

    connect(ui->tableViewScripts, &TableViewScripts::deletePressed, [&]() { removeScript(); });
    connect(ui->tableViewScripts, &QTableView::doubleClicked, this, &MainWindow::editScript);
    connect(ui->action_Edit_script, &QAction::triggered, this, &MainWindow::editScript);
    connect(ui->action_Remove_script, &QAction::triggered, this, &MainWindow::removeScript);
    connect(ui->action_Add_script, &QAction::triggered, this, &MainWindow::addScript);

    connect(ui->tableViewScripts, &TableViewScripts::customContextMenuRequested,
            [&](const QPoint& p) { mScriptContextMenu->popup(ui->tableViewScripts->viewport()->mapToGlobal(p)); });
}

void ZapFR::Client::MainWindow::createScriptContextMenus()
{
    mScriptContextMenu = std::make_unique<QMenu>(nullptr);
    mScriptContextMenu->addAction(ui->action_Add_script);
    mScriptContextMenu->addAction(ui->action_Edit_script);
    mScriptContextMenu->addSeparator();
    mScriptContextMenu->addAction(ui->action_Remove_script);
}

void ZapFR::Client::MainWindow::initializeUIScripts()
{
    ui->tableViewScripts->setItemDelegate(new ItemDelegateScript(ui->tableViewScripts));
}
