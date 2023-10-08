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

#include <QMessageBox>

#include "delegates/ItemDelegateDiscoveredFeed.h"
#include "dialogs/DialogAddFeed.h"
#include "ui_DialogAddFeed.h"

ZapFR::Client::DialogAddFeed::DialogAddFeed(QWidget* parent) : DialogWithSourcesAndFolders(parent), ui(new Ui::DialogAddFeed)
{
    ui->setupUi(this);
    setComboBoxSources(ui->comboBoxSource);
    setComboBoxFolders(ui->comboBoxFolder);

    for (const auto& button : ui->buttonBox->buttons())
    {
        if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ButtonRole::AcceptRole)
        {
            mAddFeedButton = button;
            break;
        }
    }

    mDiscoveredFeedsModel = std::make_unique<QStandardItemModel>(this);
    ui->tableViewDiscoveredFeeds->setModel(mDiscoveredFeedsModel.get());
    ui->tableViewDiscoveredFeeds->setItemDelegate(new ItemDelegateDiscoveredFeed(ui->tableViewDiscoveredFeeds));

    auto font = ui->tableViewDiscoveredFeeds->font();
    font.setPointSize(9);
    ui->tableViewDiscoveredFeeds->setFont(font);

    connect(ui->pushButtonDiscover, &QPushButton::clicked, this, &DialogAddFeed::discoverFeeds);
    connect(ui->tableViewDiscoveredFeeds, &QTableView::clicked,
            [&](const QModelIndex& index)
            {
                if (index.isValid() && index.column() == Column::CheckboxCol)
                {
                    auto item = mDiscoveredFeedsModel->itemFromIndex(index);
                    auto toggledState = item->data(Role::IsChecked).toBool() ? false : true;
                    item->setData(QVariant::fromValue<bool>(toggledState), Role::IsChecked);
                    updateAddButtonState();
                }
            });
}

ZapFR::Client::DialogAddFeed::~DialogAddFeed()
{
    delete ui;
}

void ZapFR::Client::DialogAddFeed::reset(uint64_t selectedSourceID, uint64_t selectedFolderID)
{
    clearDiscoveredFeeds();
    updateAddButtonState();
    setPreselectedSourceAndFolderIDs(selectedSourceID, selectedFolderID);
    ui->lineEditURL->setText("");
    ui->lineEditURL->setFocus();
}

QString ZapFR::Client::DialogAddFeed::url() const
{
    return ui->lineEditURL->text().trimmed();
}

std::vector<std::string> ZapFR::Client::DialogAddFeed::feedURLsToAdd() const
{
    std::vector<std::string> feeds;

    for (int32_t i = 0; i < mDiscoveredFeedsModel->rowCount(); ++i)
    {
        auto index = mDiscoveredFeedsModel->index(i, Column::CheckboxCol);
        if (index.data(Role::IsChecked).toBool())
        {
            feeds.emplace_back(index.data(Role::URL).toString().toStdString());
        }
    }

    return feeds;
}

void ZapFR::Client::DialogAddFeed::discoverFeeds()
{
    clearDiscoveredFeeds();
    ui->pushButtonDiscover->setEnabled(false);

    auto url = this->url();
    if (url.isEmpty())
    {
        return;
    }
    auto fd = ZapFR::Engine::FeedDiscovery(url.toStdString());
    fd.discover();
    const auto& discoveredFeeds = fd.discoveredFeeds();

    if (discoveredFeeds.empty())
    {
        QMessageBox mb(this);
        mb.setWindowTitle(tr("Discovery failed"));
        mb.setText(tr("No feeds were found at the provided location"));
        mb.setIcon(QMessageBox::Icon::Warning);
        mb.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        mb.button(QMessageBox::StandardButton::Ok)->setText(tr("Add anyway"));
        auto mbLayout = qobject_cast<QGridLayout*>(mb.layout());
        mbLayout->addItem(new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), mbLayout->rowCount(), 0, 1, mbLayout->columnCount());
        mb.exec();
        if (mb.clickedButton() == mb.button(QMessageBox::StandardButton::Ok))
        {
            ZapFR::Engine::DiscoveredFeed forcedFeed;
            forcedFeed.title = tr("Unknown feed").toStdString();
            forcedFeed.type = ZapFR::Engine::Feed::Type::Unknown;
            forcedFeed.url = this->url().toStdString();
            addDiscoveredFeed(forcedFeed);
        }
    }
    else
    {
        for (const auto& discoveredFeed : discoveredFeeds)
        {
            addDiscoveredFeed(discoveredFeed);
        }
    }
    updateAddButtonState();
    ui->pushButtonDiscover->setEnabled(true);
}

void ZapFR::Client::DialogAddFeed::addDiscoveredFeed(const ZapFR::Engine::DiscoveredFeed& discoveredFeed)
{
    auto discoveredURL = QString::fromUtf8(discoveredFeed.url);

    auto checkboxItem = new QStandardItem();
    checkboxItem->setData(discoveredURL, Role::URL);
    checkboxItem->setData(QVariant::fromValue<bool>(true), Role::IsChecked);

    QString type;
    switch (discoveredFeed.type)
    {
        case ZapFR::Engine::Feed::Type::Atom:
        {
            type = "Atom";
            break;
        }
        case ZapFR::Engine::Feed::Type::RSS:
        {
            type = "RSS";
            break;
        }
        case ZapFR::Engine::Feed::Type::JSON:
        {
            type = "JSON";
            break;
        }
        case ZapFR::Engine::Feed::Type::Unknown:
        {
            type = "Unknown";
            break;
        }
    }
    auto typeItem = new QStandardItem(type);
    typeItem->setData(discoveredURL, Role::URL);

    QString title;
    if (discoveredFeed.title.empty())
    {
        title = tr("Unnamed feed");
    }
    else
    {
        title = QString::fromUtf8(discoveredFeed.title);
    }
    auto titleItem = new QStandardItem(title);
    titleItem->setData(discoveredURL, Role::URL);
    titleItem->setData(title, Qt::ToolTipRole);

    auto urlItem = new QStandardItem(discoveredURL);
    urlItem->setData(discoveredURL, Role::URL);
    urlItem->setData(discoveredURL, Qt::ToolTipRole);

    QList<QStandardItem*> rowData;
    rowData << checkboxItem << typeItem << titleItem << urlItem;

    mDiscoveredFeedsModel->appendRow(rowData);
}

void ZapFR::Client::DialogAddFeed::clearDiscoveredFeeds()
{
    mDiscoveredFeedsModel->clear();
    mDiscoveredFeedsModel->setHorizontalHeaderItem(Column::CheckboxCol, new QStandardItem(tr("Add")));
    mDiscoveredFeedsModel->setHorizontalHeaderItem(Column::TypeCol, new QStandardItem(tr("Type")));
    mDiscoveredFeedsModel->setHorizontalHeaderItem(Column::TitleCol, new QStandardItem(tr("Title")));
    mDiscoveredFeedsModel->setHorizontalHeaderItem(Column::URLCol, new QStandardItem(tr("URL")));

    ui->tableViewDiscoveredFeeds->horizontalHeader()->setSectionResizeMode(Column::URLCol, QHeaderView::Stretch);
    ui->tableViewDiscoveredFeeds->horizontalHeader()->resizeSection(Column::CheckboxCol, 25);
    ui->tableViewDiscoveredFeeds->horizontalHeader()->resizeSection(Column::TypeCol, 65);
    ui->tableViewDiscoveredFeeds->horizontalHeader()->resizeSection(Column::TitleCol, 175);

    ui->tableViewDiscoveredFeeds->verticalHeader()->setDefaultSectionSize(22);
}

void ZapFR::Client::DialogAddFeed::updateAddButtonState()
{
    mAddFeedButton->setEnabled(false);

    auto checkedFeedCount{0};
    for (int32_t i = 0; i < mDiscoveredFeedsModel->rowCount(); ++i)
    {
        auto checkboxItem = mDiscoveredFeedsModel->index(i, Column::CheckboxCol);
        if (checkboxItem.isValid() && checkboxItem.data(Role::IsChecked).toBool())
        {
            checkedFeedCount++;
        }
    }

    mAddFeedButton->setEnabled(checkedFeedCount > 0);
    if (checkedFeedCount <= 1)
    {
        mAddFeedButton->setText(tr("Add feed"));
    }
    else if (checkedFeedCount > 1)
    {
        mAddFeedButton->setText(tr("Add feeds"));
    }
}