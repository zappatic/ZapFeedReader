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
            button->setText(tr("Add feed"));
            break;
        }
    }
}

ZapFR::Client::DialogAddFeed::~DialogAddFeed()
{
    delete ui;
}

void ZapFR::Client::DialogAddFeed::reset(uint64_t selectedSourceID, uint64_t selectedFolderID)
{
    setPreselectedSourceAndFolderIDs(selectedSourceID, selectedFolderID);
    ui->lineEditURL->setText("");
    ui->lineEditURL->setFocus();
}

QString ZapFR::Client::DialogAddFeed::url() const
{
    return ui->lineEditURL->text().trimmed();
}
