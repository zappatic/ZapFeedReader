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

#include "DialogAddFeed.h"
#include "ui_DialogAddFeed.h"

ZapFR::Client::DialogAddFeed::DialogAddFeed(QWidget* parent) : QDialog(parent), ui(new Ui::DialogAddFeed)
{
    ui->setupUi(this);

    mSourcesModel = std::make_unique<QStandardItemModel>(this);
    ui->comboBoxSource->setModel(mSourcesModel.get());

    for (const auto& button : ui->buttonBox->buttons())
    {
        if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ButtonRole::AcceptRole)
        {
            button->setText(tr("Add feed"));
            break;
        }
    }
}

// TODO: check chosen feed url before 'accept'ing

ZapFR::Client::DialogAddFeed::~DialogAddFeed()
{
    delete ui;
}

void ZapFR::Client::DialogAddFeed::reset(const std::vector<std::unique_ptr<ZapFR::Engine::Source>>& sources)
{
    ui->lineEditURL->setText("");

    mSourcesModel->clear();
    for (const auto& source : sources)
    {
        auto item = new QStandardItem(QString::fromUtf8(source->title()));
        item->setData(QVariant::fromValue<uint64_t>(source->id()), SourceIDRole);
        mSourcesModel->appendRow(item);
    }
}

QString ZapFR::Client::DialogAddFeed::url() const
{
    return ui->lineEditURL->text().trimmed();
}

uint64_t ZapFR::Client::DialogAddFeed::sourceID() const
{
    return ui->comboBoxSource->currentData(SourceIDRole).toULongLong();
}
