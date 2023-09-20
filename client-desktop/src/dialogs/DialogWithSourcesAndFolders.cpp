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

#include "dialogs/DialogWithSourcesAndFolders.h"
#include "ZapFR/Agent.h"
#include "ZapFR/base/Folder.h"
#include "ZapFR/base/Source.h"

ZapFR::Client::DialogWithSourcesAndFolders::DialogWithSourcesAndFolders(QWidget* parent) : QDialog(parent)
{
}

void ZapFR::Client::DialogWithSourcesAndFolders::setComboBoxSources(QComboBox* cb)
{
    mComboBoxSources = cb;
    mSourcesModel = std::make_unique<QStandardItemModel>(this);
    mComboBoxSources->setModel(mSourcesModel.get());
    connect(mComboBoxSources, &QComboBox::currentIndexChanged, this, &DialogWithSourcesAndFolders::currentSourceChanged);
}

void ZapFR::Client::DialogWithSourcesAndFolders::setComboBoxFolders(QComboBox* cb)
{
    mComboBoxFolders = cb;
    mFoldersModel = std::make_unique<QStandardItemModel>(this);
    mComboBoxFolders->setModel(mFoldersModel.get());
}

uint64_t ZapFR::Client::DialogWithSourcesAndFolders::selectedFolderID() const
{
    if (mComboBoxFolders->currentIndex() > -1)
    {
        return mComboBoxFolders->currentData(FolderIDRole).toULongLong();
    }
    return 0;
}

uint64_t ZapFR::Client::DialogWithSourcesAndFolders::selectedSourceID() const
{
    return mComboBoxSources->currentData(SourceIDRole).toULongLong();
}

void ZapFR::Client::DialogWithSourcesAndFolders::setPreselectedSourceAndFolderIDs(uint64_t selectedSourceID, uint64_t selectedFolderID)
{
    mSourcesModel->clear();

    int32_t toSelect{-1};
    int32_t counter{0};

    mIsPopulatingSourcesModel = true;
    auto sources = ZapFR::Engine::Source::getSources({});
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
    // we have to force to -1 here, because if the folders for the first source have to be populated
    // the current index would already be 0 and setCurrentIndex() wouldn't trigger a change event
    mComboBoxSources->setCurrentIndex(-1);
    mIsPopulatingSourcesModel = false;

    mFolderIDToPreselect = static_cast<int64_t>(selectedFolderID);
    mComboBoxSources->setCurrentIndex(toSelect);
}

void ZapFR::Client::DialogWithSourcesAndFolders::currentSourceChanged(int index)
{
    if (mComboBoxFolders == nullptr || mComboBoxSources == nullptr || index == -1 || mIsPopulatingSourcesModel)
    {
        return;
    }

    auto sourceID = mComboBoxSources->currentData(SourceIDRole).toULongLong();
    ZapFR::Engine::Agent::getInstance()->queueGetFolders(sourceID, 0,
                                                         [&](const std::vector<ZapFR::Engine::Folder*>& rootFolders)
                                                         {
                                                             static QString space(" ");
                                                             QList<QStandardItem*> items;

                                                             // lambda to recursively create folder items
                                                             std::function<void(ZapFR::Engine::Folder*, ssize_t)> createFolderItems;
                                                             createFolderItems = [&](ZapFR::Engine::Folder* folder, ssize_t depth)
                                                             {
                                                                 auto folderItem =
                                                                     new QStandardItem(QString("%1%2").arg(space.repeated(depth * 4)).arg(QString::fromUtf8(folder->title())));
                                                                 folderItem->setData(QVariant::fromValue<uint64_t>(folder->id()), FolderIDRole);
                                                                 items.append(folderItem);

                                                                 for (const auto& subfolder : folder->subfolders())
                                                                 {
                                                                     createFolderItems(subfolder.get(), depth + 1);
                                                                 }
                                                             };

                                                             for (const auto& folder : rootFolders)
                                                             {
                                                                 createFolderItems(folder, 0);
                                                             }
                                                             QMetaObject::invokeMethod(this, [=, this]() { populateFolders(items); });
                                                         });
}

void ZapFR::Client::DialogWithSourcesAndFolders::populateFolders(const QList<QStandardItem*>& folders)
{
    mFoldersModel->clear();

    // insert a blank folder item, to be able to add to the root folder
    auto blankFolderItem = new QStandardItem("");
    blankFolderItem->setData(QVariant::fromValue<uint64_t>(0), FolderIDRole);
    mFoldersModel->appendRow(blankFolderItem);

    int32_t currentIndex = 1;
    int32_t indexToPreselect = -1;
    for (const auto& folder : folders)
    {
        mFoldersModel->appendRow(folder);

        auto folderID = folder->data(FolderIDRole).toULongLong();
        if (mFolderIDToPreselect == static_cast<int64_t>(folderID))
        {
            indexToPreselect = currentIndex;
        }
        currentIndex++;
    }

    if (indexToPreselect != -1)
    {
        mComboBoxFolders->setCurrentIndex(indexToPreselect);
        mFolderIDToPreselect = -1;
    }
}
