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

    auto cb = ui->comboBoxLogLevel;
    cb->addItem(tr("Debug"));
    cb->setItemData(0, QVariant::fromValue<uint64_t>(static_cast<uint64_t>(ZapFR::Engine::LogLevel::Debug)), Role::LogLevel);
    cb->addItem(tr("Info"));
    cb->setItemData(1, QVariant::fromValue<uint64_t>(static_cast<uint64_t>(ZapFR::Engine::LogLevel::Info)), Role::LogLevel);
    cb->addItem(tr("Warning"));
    cb->setItemData(2, QVariant::fromValue<uint64_t>(static_cast<uint64_t>(ZapFR::Engine::LogLevel::Warning)), Role::LogLevel);
    cb->addItem(tr("Error"));
    cb->setItemData(3, QVariant::fromValue<uint64_t>(static_cast<uint64_t>(ZapFR::Engine::LogLevel::Error)), Role::LogLevel);
}

ZapFR::Client::DialogPreferences::~DialogPreferences()
{
    delete ui;
}

void ZapFR::Client::DialogPreferences::reset()
{
    auto mainWindow = qobject_cast<MainWindow*>(parent());
    switch (mainWindow->preferences()->theme)
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

    switch (mainWindow->preferences()->refreshBehaviour)
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
    ui->spinBoxUIFontSize->setValue(mainWindow->preferences()->uiFontSize);
    ui->spinBoxPostFontSize->setValue(mainWindow->preferences()->postFontSize);
    ui->checkBoxDetectBrowsers->setChecked(mainWindow->preferences()->detectBrowsers);
    ui->checkBoxHideLocalSource->setChecked(mainWindow->preferences()->hideLocalSource);
    ui->checkBoxMinimizeInsteadOfClose->setChecked(mainWindow->preferences()->minimizeInsteadOfClose);
    ui->checkBoxUnreadPostsAtTop->setChecked(mainWindow->preferences()->showUnreadPostsAtTop);

    auto ar = ZapFR::Engine::AutoRefresh::getInstance();
    ui->spinBoxAutoRefreshInterval->setValue(static_cast<int32_t>(ar->feedRefreshInterval() / 60));
    ui->checkBoxAutoRefreshEnabled->setChecked(ar->isEnabled());

    switch (ZapFR::Engine::Log::logLevel())
    {
        case ZapFR::Engine::LogLevel::Debug:
        {
            ui->comboBoxLogLevel->setCurrentIndex(0);
            break;
        }
        case ZapFR::Engine::LogLevel::Info:
        {
            ui->comboBoxLogLevel->setCurrentIndex(1);
            break;
        }
        case ZapFR::Engine::LogLevel::Warning:
        {
            ui->comboBoxLogLevel->setCurrentIndex(2);
            break;
        }
        case ZapFR::Engine::LogLevel::Error:
        {
            ui->comboBoxLogLevel->setCurrentIndex(3);
            break;
        }
    }

    ui->tabWidget->setCurrentIndex(0);
}

ZapFR::Client::Theme ZapFR::Client::DialogPreferences::theme() const
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

uint16_t ZapFR::Client::DialogPreferences::uiFontSize() const
{
    return static_cast<uint16_t>(ui->spinBoxUIFontSize->value());
}

uint16_t ZapFR::Client::DialogPreferences::postFontSize() const
{
    return static_cast<uint16_t>(ui->spinBoxPostFontSize->value());
}

ZapFR::Client::RefreshBehaviour ZapFR::Client::DialogPreferences::refreshBehaviour() const
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

uint64_t ZapFR::Client::DialogPreferences::autoRefreshInterval() const
{
    return static_cast<uint64_t>(ui->spinBoxAutoRefreshInterval->value() * 60);
}

bool ZapFR::Client::DialogPreferences::autoRefreshEnabled() const
{
    return ui->checkBoxAutoRefreshEnabled->isChecked();
}

bool ZapFR::Client::DialogPreferences::detectBrowsersEnabled() const
{
    return ui->checkBoxDetectBrowsers->isChecked();
}

bool ZapFR::Client::DialogPreferences::hideLocalSource() const
{
    return ui->checkBoxHideLocalSource->isChecked();
}

bool ZapFR::Client::DialogPreferences::minimizeInsteadOfClose() const
{
    return ui->checkBoxMinimizeInsteadOfClose->isChecked();
}

bool ZapFR::Client::DialogPreferences::showUnreadPostsAtTop() const
{
    return ui->checkBoxUnreadPostsAtTop->isChecked();
}

ZapFR::Engine::LogLevel ZapFR::Client::DialogPreferences::logLevel() const
{
    return static_cast<ZapFR::Engine::LogLevel>(ui->comboBoxLogLevel->currentData(LogLevel).toULongLong());
}
