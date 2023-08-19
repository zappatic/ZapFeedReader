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

#include "DialogEditScript.h"
#include "ItemDelegateEditScriptDialogSource.h"
#include "MainWindow.h"
#include "StandardItemModelSources.h"
#include "ui_DialogEditScript.h"

ZapFR::Client::DialogEditScript::DialogEditScript(QWidget* parent) : QDialog(parent), ui(new Ui::DialogEditScript)
{
    ui->setupUi(this);
    mFeedsModel = std::make_unique<QStandardItemModel>();
    ui->treeViewRunOnFeedIDs->setModel(mFeedsModel.get());
    ui->treeViewRunOnFeedIDs->setItemDelegate(new ItemDelegateEditScriptDialogSource(ui->treeViewRunOnFeedIDs));
}

ZapFR::Client::DialogEditScript::~DialogEditScript()
{
    delete ui;
}

void ZapFR::Client::DialogEditScript::reset(DisplayMode dm, uint64_t sourceID, uint64_t id, const QString& filename, bool isEnabled,
                                            const std::unordered_set<ZapFR::Engine::Script::Event>& runOnEvents,
                                            const std::optional<std::unordered_set<uint64_t>>& runOnFeedIDs)
{
    QString buttonCaption;
    switch (dm)
    {
        case DisplayMode::Add:
        {
            setWindowTitle(tr("Add script"));
            buttonCaption = tr("Add script");
            break;
        }
        case DisplayMode::Edit:
        {
            setWindowTitle(tr("Edit script"));
            buttonCaption = tr("Save");
            break;
        }
    }

    for (const auto& button : ui->buttonBox->buttons())
    {
        if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ButtonRole::AcceptRole)
        {
            button->setText(buttonCaption);
            break;
        }
    }

    mCurrentID = id;
    ui->lineEditFilename->setText(filename);
    ui->checkBoxEnabled->setCheckState(isEnabled ? Qt::Checked : Qt::Unchecked);

    ui->checkBoxRunOnNewPost->setCheckState(Qt::Unchecked);
    if (runOnEvents.contains(ZapFR::Engine::Script::Event::NewPost))
    {
        ui->checkBoxRunOnNewPost->setCheckState(Qt::Checked);
    }

    // clone the feeds and folders from the sources tree view, so we don't have to requery
    mFeedsModel->clear();
    auto parentModel = qobject_cast<MainWindow*>(parent())->sourcesItemModel();

    std::function<void(const QModelIndex&, QStandardItem*)> cloneItemAndChildren;
    cloneItemAndChildren = [&](const QModelIndex& parent, QStandardItem* feedsModelParentItem)
    {
        for (int32_t i = 0; i < parentModel->rowCount(parent); ++i)
        {
            auto child = parentModel->index(i, 0, parent);
            if (child.data(SourceTreeEntryParentSourceIDRole).toULongLong() == sourceID)
            {
                auto clone = parentModel->itemFromIndex(child)->clone();

                if (runOnFeedIDs.has_value() && clone->data(SourceTreeEntryTypeRole).toULongLong() == SOURCETREE_ENTRY_TYPE_FEED &&
                    runOnFeedIDs.value().contains(clone->data(SourceTreeEntryIDRole).toULongLong()))
                {
                    clone->setData(Qt::Checked, Qt::CheckStateRole);
                }
                else
                {
                    clone->setData(Qt::Unchecked, Qt::CheckStateRole);
                }
                feedsModelParentItem->appendRow(clone);
                cloneItemAndChildren(child, clone);
            }
        }
    };
    for (int32_t j = 0; j < parentModel->invisibleRootItem()->rowCount(); ++j)
    {
        auto index = parentModel->index(j, 0);
        cloneItemAndChildren(index, mFeedsModel->invisibleRootItem());
    }

    // auto expand all the folders
    std::function<void(QStandardItem*)> expandChildren;
    expandChildren = [&](QStandardItem* parent)
    {
        for (int32_t k = 0; k < parent->rowCount(); ++k)
        {
            auto child = parent->child(k, 0);
            ui->treeViewRunOnFeedIDs->setExpanded(mFeedsModel->indexFromItem(child), true);
            expandChildren(child);
        }
    };
    expandChildren(mFeedsModel->invisibleRootItem());
}
