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

#include <QAbstractButton>

#include "dialogs/DialogEditFolder.h"
#include "ui_DialogEditFolder.h"

ZapFR::Client::DialogEditFolder::DialogEditFolder(QWidget* parent) : QDialog(parent), ui(new Ui::DialogEditFolder)
{
    ui->setupUi(this);
}

ZapFR::Client::DialogEditFolder::~DialogEditFolder()
{
    delete ui;
}

uint64_t ZapFR::Client::DialogEditFolder::sourceID() const noexcept
{
    return mCurrentSourceID;
}

uint64_t ZapFR::Client::DialogEditFolder::id() const noexcept
{
    return mCurrentID;
}

void ZapFR::Client::DialogEditFolder::reset(uint64_t sourceID, uint64_t id, const QString& title)
{
    mCurrentSourceID = sourceID;
    mCurrentID = id;
    ui->lineEditTitle->setText(title);

    setWindowTitle(tr("Edit folder"));
    auto buttonCaption = tr("Save");

    for (const auto& button : ui->buttonBox->buttons())
    {
        if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ButtonRole::AcceptRole)
        {
            button->setText(buttonCaption);
            break;
        }
    }

    ui->lineEditTitle->setFocus();
}

QString ZapFR::Client::DialogEditFolder::title() const noexcept
{
    return ui->lineEditTitle->text();
}
