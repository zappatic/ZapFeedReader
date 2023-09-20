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

#include "widgets/TableViewScripts.h"
#include "./ui_MainWindow.h"
#include "ZapFR/Agent.h"
#include "ZapFR/base/Script.h"
#include "delegates/ItemDelegateScript.h"
#include "dialogs/DialogEditScript.h"
#include "widgets/MainWindow.h"
#include "widgets/TreeViewSources.h"

ZapFR::Client::TableViewScripts::TableViewScripts(QWidget* parent) : TableViewPaletteCorrected(parent)
{
    setItemDelegate(new ItemDelegateScript(this));

    mActionViewScripts = std::make_unique<QAction>(tr("View scripts"), this);
    mActionAddScript = std::make_unique<QAction>(tr("Add script"), this);
    mActionEditScript = std::make_unique<QAction>(tr("Edit script"), this);
    mActionRemoveScript = std::make_unique<QAction>(tr("Remove script"), this);

    connect(mActionViewScripts.get(), &QAction::triggered, [&]() { reload(true); });
    connect(mActionAddScript.get(), &QAction::triggered, this, &TableViewScripts::addScript);
    connect(mActionEditScript.get(), &QAction::triggered, this, &TableViewScripts::editScript);
    connect(mActionRemoveScript.get(), &QAction::triggered, this, &TableViewScripts::removeScript);

    connect(this, &TableViewScripts::customContextMenuRequested, [&](const QPoint& p) { mScriptContextMenu->popup(viewport()->mapToGlobal(p)); });
    connect(this, &TableViewScripts::deletePressed, [&]() { removeScript(); });
    connect(this, &QTableView::doubleClicked, this, &TableViewScripts::editScript);

    mScriptContextMenu = std::make_unique<QMenu>(nullptr);
    mScriptContextMenu->addAction(mActionAddScript.get());
    mScriptContextMenu->addAction(mActionEditScript.get());
    mScriptContextMenu->addSeparator();
    mScriptContextMenu->addAction(mActionRemoveScript.get());
}

void ZapFR::Client::TableViewScripts::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
        emit deletePressed();
    }
    else
    {
        QTableView::keyPressEvent(event);
    }
}

void ZapFR::Client::TableViewScripts::reload(bool forceReload)
{
    // lambda to assign the correct role data to the table entries
    auto setItemData = [&](QStandardItem* item, ZapFR::Engine::Script* script, uint64_t sourceID)
    {
        item->setData(QVariant::fromValue<uint64_t>(script->id()), Role::ID);
        item->setData(QVariant::fromValue<uint64_t>(sourceID), Role::SourceID);
        item->setData(script->isEnabled(), Role::IsEnabled);
        item->setData(QString::fromUtf8(script->title()), Role::Title);
        item->setData(QString::fromUtf8(script->script()), Role::Script);

        QVariantList events;
        for (const auto& event : script->runOnEvents())
        {
            events << QVariant(static_cast<std::underlying_type_t<ZapFR::Engine::Script::Event>>(event));
        }
        item->setData(events, Role::RunOnEvents);

        QVariantList feeds;
        const auto& runOnFeedIDs = script->runOnFeedIDs();
        if (runOnFeedIDs.has_value())
        {
            for (const auto& feed : runOnFeedIDs.value())
            {
                feeds << QVariant::fromValue<uint64_t>(feed);
            }
            item->setData(feeds, Role::RunOnFeedIDs);
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
            runOnEventsItem->setData(QVariant::fromValue<uint64_t>(events.size()), Role::EventCount);
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

        QMetaObject::invokeMethod(this, [=, this]() { populateScripts(rows); });
        mMainWindow->setPreviouslySelectedSourceID(sourceID);
    };

    auto index = mMainWindow->getUI()->treeViewSources->currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
        if (forceReload || sourceID != mMainWindow->previouslySelectedSourceID())
        {
            ZapFR::Engine::Agent::getInstance()->queueGetScripts(sourceID, processScripts);
        }
    }
}

void ZapFR::Client::TableViewScripts::populateScripts(const QList<QList<QStandardItem*>>& scripts)
{
    mMainWindow->setContentPane(StackedPaneScripts);

    mItemModelScripts = std::make_unique<QStandardItemModel>(this);
    setModel(mItemModelScripts.get());
    mItemModelScripts->setHorizontalHeaderItem(Column::TypeCol, new QStandardItem(tr("Type")));
    mItemModelScripts->setHorizontalHeaderItem(Column::TitleCol, new QStandardItem(tr("Title")));
    mItemModelScripts->setHorizontalHeaderItem(Column::IsEnabledCol, new QStandardItem(tr("Enabled")));
    mItemModelScripts->setHorizontalHeaderItem(Column::RunOnEventsCol, new QStandardItem(tr("Run on event(s)")));
    mItemModelScripts->setHorizontalHeaderItem(Column::RunOnFeedIDsCol, new QStandardItem(tr("Run on feed(s)")));
    for (const auto& script : scripts)
    {
        mItemModelScripts->appendRow(script);
    }
    horizontalHeader()->setSectionResizeMode(Column::TitleCol, QHeaderView::Stretch);
    horizontalHeader()->resizeSection(Column::TypeCol, 50);
    horizontalHeader()->resizeSection(Column::IsEnabledCol, 75);
    horizontalHeader()->resizeSection(Column::RunOnEventsCol, 175);
    horizontalHeader()->resizeSection(Column::RunOnFeedIDsCol, 150);
}

ZapFR::Client::DialogEditScript* ZapFR::Client::TableViewScripts::editScriptDialog()
{
    if (mDialogEditScript == nullptr)
    {
        mDialogEditScript = std::make_unique<DialogEditScript>(mMainWindow);
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
                                                                                [&](uint64_t /*addedSourceID*/)
                                                                                { QMetaObject::invokeMethod(this, [=, this]() { reload(true); }); });
                            break;
                        }
                        case DialogEditScript::DisplayMode::Edit:
                        {
                            ZapFR::Engine::Agent::getInstance()->queueUpdateScript(scriptSourceID, scriptID, type, title, enabled, selectedEvents, selectedFeedIDs, script,
                                                                                   [&](uint64_t /*updatedSourceID*/, uint64_t /*updatedScriptID*/)
                                                                                   { QMetaObject::invokeMethod(this, [=, this]() { reload(true); }); });
                            break;
                        }
                    }
                });
    }
    return mDialogEditScript.get();
}

void ZapFR::Client::TableViewScripts::editScript()
{
    auto index = currentIndex();
    if (index.isValid())
    {
        auto scriptID = index.data(Role::ID).toULongLong();
        auto scriptSourceID = index.data(Role::SourceID).toULongLong();
        auto scriptIsEnabled = index.data(Role::IsEnabled).toBool();
        std::unordered_set<ZapFR::Engine::Script::Event> events;
        for (const auto& eventVariant : index.data(Role::RunOnEvents).toList())
        {
            events.insert(eventVariant.value<ZapFR::Engine::Script::Event>());
        }
        std::optional<std::unordered_set<uint64_t>> feedIDs{};
        auto feedIDsVariants = index.data(Role::RunOnFeedIDs);
        if (!feedIDsVariants.isNull() && feedIDsVariants.isValid())
        {
            feedIDs = std::unordered_set<uint64_t>();
            for (const auto& feedIDVariant : feedIDsVariants.toList())
            {
                feedIDs.value().insert(feedIDVariant.value<uint64_t>());
            }
        }
        auto scriptTitle = index.data(Role::Title).toString();
        auto script = index.data(Role::Script).toString();

        // TODO :better to agent->getScript()

        auto dialog = editScriptDialog();
        dialog->reset(DialogEditScript::DisplayMode::Edit, scriptSourceID, scriptID, scriptTitle, scriptIsEnabled, events, feedIDs, script);
        dialog->open();
    }
}

void ZapFR::Client::TableViewScripts::removeScript()
{
    auto index = currentIndex();
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
            auto scriptID = index.data(Role::ID).toULongLong();
            auto scriptSourceID = index.data(Role::SourceID).toULongLong();
            ZapFR::Engine::Agent::getInstance()->queueRemoveScript(scriptSourceID, scriptID,
                                                                   [&](uint64_t /*removedSourceID*/, uint64_t /*removedScriptID*/)
                                                                   {
                                                                       QMetaObject::invokeMethod(this,
                                                                                                 [=, this]()
                                                                                                 {
                                                                                                     reload(true);
                                                                                                     mMainWindow->setStatusBarMessage(tr("Script removed"));
                                                                                                 });
                                                                   });
        }
    }
}

void ZapFR::Client::TableViewScripts::addScript()
{
    auto index = mMainWindow->getUI()->treeViewSources->currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();

        auto dialog = editScriptDialog();
        dialog->reset(DialogEditScript::DisplayMode::Add, sourceID, 0, "", true, {ZapFR::Engine::Script::Event::NewPost, ZapFR::Engine::Script::Event::UpdatePost}, {}, "");
        dialog->open();
    }
}
