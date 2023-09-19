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
#include "ZapFR/AutoRefresh.h"
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
            ui->radioButtonThemeSystem->setChecked(true);
#else
            ui->radioButtonThemeLight->setChecked(true);
#endif
            break;
        }
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
    ui->radioButtonThemeSystem->setVisible(false);
#endif

    switch (mainWindow->currentPreferenceRefreshBehaviour())
    {
        case RefreshBehaviour::CurrentSelection:
        {
            ui->radioButtonRefreshBehaviourAsSelected->setChecked(true);
            break;
        }
        case RefreshBehaviour::EntireSource:
        {
            ui->radioButtonradioButtonRefreshBehaviourForceSource->setChecked(true);
            break;
        }
    }
    ui->spinBoxUIFontSize->setValue(mainWindow->currentPreferenceUIFontSize());
    ui->spinBoxPostFontSize->setValue(mainWindow->currentPreferencePostFontSize());

    auto ar = ZapFR::Engine::AutoRefresh::getInstance();
    ui->spinBoxAutoRefreshInterval->setValue(static_cast<int32_t>(ar->feedRefreshInterval() / 60));
    ui->checkBoxAutoRefreshEnabled->setChecked(ar->isEnabled());

    ui->tabWidget->setCurrentIndex(0);
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        return Theme::UseSystem;
#else
        return Theme::Light;
#endif
    }
}

uint16_t ZapFR::Client::DialogPreferences::chosenUIFontSize() const
{
    return static_cast<uint16_t>(ui->spinBoxUIFontSize->value());
}

uint16_t ZapFR::Client::DialogPreferences::chosenPostFontSize() const
{
    return static_cast<uint16_t>(ui->spinBoxPostFontSize->value());
}

ZapFR::Client::RefreshBehaviour ZapFR::Client::DialogPreferences::chosenRefreshBehaviour() const
{
    if (ui->radioButtonradioButtonRefreshBehaviourForceSource->isChecked())
    {
        return RefreshBehaviour::EntireSource;
    }
    else
    {
        return RefreshBehaviour::CurrentSelection;
    }
}

uint64_t ZapFR::Client::DialogPreferences::chosenAutoRefreshInterval() const
{
    return static_cast<uint64_t>(ui->spinBoxAutoRefreshInterval->value() * 60);
}

bool ZapFR::Client::DialogPreferences::chosenAutoRefreshEnabled() const
{
    return ui->checkBoxAutoRefreshEnabled->isChecked();
}
