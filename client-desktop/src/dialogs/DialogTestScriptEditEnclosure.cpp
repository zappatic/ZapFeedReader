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

#include "dialogs/DialogTestScriptEditEnclosure.h"
#include "ui_DialogTestScriptEditEnclosure.h"

ZapFR::Client::DialogTestScriptEditEnclosure::DialogTestScriptEditEnclosure(QWidget* parent) : QDialog(parent), ui(new Ui::DialogTestScriptEditEnclosure)
{
    ui->setupUi(this);
}

ZapFR::Client::DialogTestScriptEditEnclosure::~DialogTestScriptEditEnclosure()
{
    delete ui;
}

void ZapFR::Client::DialogTestScriptEditEnclosure::reset(DisplayMode dm, const QString& url, const QString& mimeType, uint64_t size)
{
    mDisplayMode = dm;

    ui->lineEditURL->setText(url);
    ui->lineEditMimetype->setText(mimeType);
    ui->lineEditSize->setText(QString::number(size));

    QString buttonCaption;
    switch (dm)
    {
        case DisplayMode::Add:
        {
            setWindowTitle(tr("Add enclosure"));
            buttonCaption = tr("Add enclosure");
            break;
        }
        case DisplayMode::Edit:
        {
            setWindowTitle(tr("Edit enclosure"));
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

    ui->lineEditURL->setFocus();
}

QString ZapFR::Client::DialogTestScriptEditEnclosure::url() const noexcept
{
    return ui->lineEditURL->text();
}

QString ZapFR::Client::DialogTestScriptEditEnclosure::mimeType() const noexcept
{
    return ui->lineEditMimetype->text();
}

uint64_t ZapFR::Client::DialogTestScriptEditEnclosure::size() const
{
    return ui->lineEditSize->text().toUInt();
}
