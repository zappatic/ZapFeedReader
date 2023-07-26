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

#include "DialogAddFolder.h"
#include "Source.h"
#include "ui_DialogAddFolder.h"

ZapFR::Client::DialogAddFolder::DialogAddFolder(QWidget* parent) : QDialog(parent), ui(new Ui::DialogAddFolder)
{
    ui->setupUi(this);

    mSourcesModel = std::make_unique<QStandardItemModel>(this);
    ui->comboBoxSources->setModel(mSourcesModel.get());

    mFoldersModel = std::make_unique<QStandardItemModel>(this);
    ui->comboBoxAddUnderFolder->setModel(mFoldersModel.get());

    for (const auto& button : ui->buttonBox->buttons())
    {
        if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ButtonRole::AcceptRole)
        {
            button->setText(tr("Add folder"));
            break;
        }
    }

    connect(ui->comboBoxSources, &QComboBox::currentIndexChanged, this, &DialogAddFolder::currentSourceChanged);
}

ZapFR::Client::DialogAddFolder::~DialogAddFolder()
{
    delete ui;
}

QString ZapFR::Client::DialogAddFolder::title() const
{
    return ui->lineEditTitle->text();
}

uint64_t ZapFR::Client::DialogAddFolder::addUnderFolder() const
{
    if (ui->comboBoxAddUnderFolder->currentIndex() > -1)
    {
        return ui->comboBoxAddUnderFolder->currentData(FolderIDRole).toULongLong();
    }
    return 0;
}

uint64_t ZapFR::Client::DialogAddFolder::sourceID() const
{
    return ui->comboBoxSources->currentData(SourceIDRole).toULongLong();
}

void ZapFR::Client::DialogAddFolder::reset(const std::vector<std::unique_ptr<ZapFR::Engine::Source>>& sources, uint64_t selectedSourceID, uint64_t selectedFolderID)
{
    ui->lineEditTitle->setText("");

    mSourcesModel->clear();
    int32_t toSelect{-1};
    int32_t counter{0};
    for (const auto& source : sources)
    {
        auto item = new QStandardItem(QString::fromUtf8(source->title()));
        auto sourceID = source->id();
        item->setData(QVariant::fromValue<uint64_t>(sourceID), SourceIDRole);
        if (sourceID == selectedSourceID)
        {
            toSelect = counter;
        }
        mSourcesModel->appendRow(item);
        counter++;
    }

    if (toSelect != -1)
    {
        ui->comboBoxSources->setCurrentIndex(toSelect);
    }

    mFolderIDToPreselect = static_cast<int64_t>(selectedFolderID);
    currentSourceChanged(-1);
}

void ZapFR::Client::DialogAddFolder::currentSourceChanged(int /*index*/)
{
    mFoldersModel->clear();

    // insert a blank folder item, to be able to add to the root folder
    auto blankFolderItem = new QStandardItem("");
    blankFolderItem->setData(QVariant::fromValue<uint64_t>(0), FolderIDRole);
    mFoldersModel->appendRow(blankFolderItem);

    static QString space(" ");
    int32_t currentIndex = 1;
    int32_t indexToPreselect = -1;
    // lambda to recursively create folder items
    std::function<void(ZapFR::Engine::Folder*, ssize_t)> createFolderItems;
    createFolderItems = [&](ZapFR::Engine::Folder* folder, ssize_t depth)
    {
        auto folderItem = new QStandardItem(QString("%1%2").arg(space.repeated(depth * 4)).arg(QString::fromUtf8(folder->title())));
        folderItem->setData(QVariant::fromValue<uint64_t>(folder->id()), FolderIDRole);
        mFoldersModel->appendRow(folderItem);

        if (mFolderIDToPreselect == static_cast<int64_t>(folder->id()))
        {
            indexToPreselect = currentIndex;
        }

        currentIndex++;

        if (folder->hasSubfolders())
        {
            for (const auto& subfolder : folder->subfolders())
            {
                createFolderItems(subfolder, depth + 1);
            }
        }
    };

    auto sourceID = ui->comboBoxSources->currentData(SourceIDRole).toULongLong();

    auto source = ZapFR::Engine::Source::getSource(sourceID);
    if (source.has_value())
    {
        auto rootFolders = source.value()->getFolders(0);
        for (const auto& folder : rootFolders)
        {
            createFolderItems(folder.get(), 0);
        }
    }

    if (indexToPreselect != -1)
    {
        ui->comboBoxAddUnderFolder->setCurrentIndex(indexToPreselect);
        mFolderIDToPreselect = -1;
    }
}
