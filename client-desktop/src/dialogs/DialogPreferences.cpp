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

#include "dialogs/DialogPreferences.h"
#include "ui_DialogPreferences.h"

ZapFR::Client::DialogPreferences::DialogPreferences(QWidget* parent) : QDialog(parent), ui(new Ui::DialogPreferences)
{
    ui->setupUi(this);
}

ZapFR::Client::DialogPreferences::~DialogPreferences()
{
    delete ui;
}

void ZapFR::Client::DialogPreferences::reset()
{
    auto mainWindow = qobject_cast<MainWindow*>(parent());
    switch (mainWindow->currentPreferenceTheme())
    {
        case Theme::Dark:
        {
            ui->radioButtonThemeDark->setChecked(true);
            break;
        }
        case Theme::Light:
        {
            ui->radioButtonThemeLight->setChecked(true);
            break;
        }
        default:
        {
            ui->radioButtonThemeSystem->setChecked(true);
            break;
        }
    }
    ui->spinBoxFontSize->setValue(mainWindow->currentPreferenceUIFontSize());
}

ZapFR::Client::Theme ZapFR::Client::DialogPreferences::chosenTheme() const
{
    if (ui->radioButtonThemeDark->isChecked())
    {
        return Theme::Dark;
    }
    else if (ui->radioButtonThemeLight->isChecked())
    {
        return Theme::Light;
    }
    else
    {
        return Theme::UseSystem;
    }
}

uint16_t ZapFR::Client::DialogPreferences::chosenUIFontSize() const
{
    return static_cast<uint16_t>(ui->spinBoxFontSize->value());
}
