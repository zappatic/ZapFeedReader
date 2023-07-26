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
#include "ui_DialogAddFolder.h"

ZapFR::Client::DialogAddFolder::DialogAddFolder(QWidget* parent) : DialogWithSourcesAndFolders(parent), ui(new Ui::DialogAddFolder)
{
    ui->setupUi(this);
    setComboBoxSources(ui->comboBoxSources);
    setComboBoxFolders(ui->comboBoxAddUnderFolder);

    for (const auto& button : ui->buttonBox->buttons())
    {
        if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ButtonRole::AcceptRole)
        {
            button->setText(tr("Add folder"));
            break;
        }
    }
}

ZapFR::Client::DialogAddFolder::~DialogAddFolder()
{
    delete ui;
}

QString ZapFR::Client::DialogAddFolder::title() const
{
    return ui->lineEditTitle->text();
}

void ZapFR::Client::DialogAddFolder::reset(uint64_t selectedSourceID, uint64_t selectedFolderID)
{
    ui->lineEditTitle->setText("");
    setPreselectedSourceAndFolderIDs(selectedSourceID, selectedFolderID);
}
