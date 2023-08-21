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

#include "dialogs/DialogEditScriptFolder.h"
#include "ui_DialogEditScriptFolder.h"

ZapFR::Client::DialogEditScriptFolder::DialogEditScriptFolder(QWidget* parent) : QDialog(parent), ui(new Ui::DialogEditScriptFolder)
{
    ui->setupUi(this);
}

ZapFR::Client::DialogEditScriptFolder::~DialogEditScriptFolder()
{
    delete ui;
}

ZapFR::Client::DialogEditScriptFolder::DisplayMode ZapFR::Client::DialogEditScriptFolder::displayMode() const noexcept
{
    return mDisplayMode;
}

uint64_t ZapFR::Client::DialogEditScriptFolder::sourceID() const noexcept
{
    return mCurrentSourceID;
}

uint64_t ZapFR::Client::DialogEditScriptFolder::id() const noexcept
{
    return mCurrentID;
}

void ZapFR::Client::DialogEditScriptFolder::reset(DisplayMode dm, uint64_t sourceID, uint64_t id, const QString& title)
{
    mDisplayMode = dm;
    mCurrentSourceID = sourceID;
    mCurrentID = id;
    ui->lineEditTitle->setText(title);

    QString buttonCaption;
    switch (dm)
    {
        case DisplayMode::Add:
        {
            setWindowTitle(tr("Add script folder"));
            buttonCaption = tr("Add script folder");
            break;
        }
        case DisplayMode::Edit:
        {
            setWindowTitle(tr("Edit script folder"));
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

    ui->lineEditTitle->setFocus();
}

QString ZapFR::Client::DialogEditScriptFolder::title() const noexcept
{
    return ui->lineEditTitle->text();
}
