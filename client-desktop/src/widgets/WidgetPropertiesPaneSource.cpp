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

#include "widgets/WidgetPropertiesPaneSource.h"
#include "FeedIconCache.h"
#include "Utilities.h"
#include "ZapFR/Agent.h"
#include "ZapFR/base/Source.h"
#include "ui_WidgetPropertiesPaneSource.h"

ZapFR::Client::WidgetPropertiesPaneSource::WidgetPropertiesPaneSource(QWidget* parent) : QWidget(parent), ui(new Ui::WidgetPropertiesPaneSource)
{
    ui->setupUi(this);
    ui->labelRemotePortInfo->setText(ui->labelRemotePortInfo->text().arg(ZapFR::Engine::DefaultServerPort));

    auto f = ui->labelCaption->font();
    f.setWeight(QFont::DemiBold);
    ui->labelCaption->setFont(f);

    connect(ui->pushButtonSave, &QPushButton::clicked, this, &WidgetPropertiesPaneSource::save);
}

ZapFR::Client::WidgetPropertiesPaneSource::~WidgetPropertiesPaneSource()
{
    delete ui;
}

void ZapFR::Client::WidgetPropertiesPaneSource::reset(const QMap<QString, QVariant>& props)
{
    mSourceID = props["sourceID"].toULongLong();

    ui->labelCaption->setText(props["title"].toString());
    ui->lineEditTitle->setText(props["title"].toString());

    auto type = props["type"].toString();
    ui->labelTypeValue->setText(type);

    ui->lineEditRemoteHost->setText("");
    ui->lineEditRemotePort->setText("");
    ui->lineEditRemoteLogin->setText("");
    ui->lineEditRemotePassword->setText("");
    ui->checkBoxRemoteUseHTTPS->setChecked(false);

    auto remoteFieldsEnabled{false};
    if (type == ZapFR::Engine::IdentifierRemoteServer)
    {
        remoteFieldsEnabled = true;
        try
        {
            Poco::JSON::Parser parser;
            auto rootObj = parser.parse(props["configData"].toString().toStdString()).extract<Poco::JSON::Object::Ptr>();
            if (rootObj->has(ZapFR::Engine::JSONIdentifierRemoteConfigDataHost))
            {
                ui->lineEditRemoteHost->setText(QString::fromUtf8(rootObj->getValue<std::string>(ZapFR::Engine::JSONIdentifierRemoteConfigDataHost)));
            }
            if (rootObj->has(ZapFR::Engine::JSONIdentifierRemoteConfigDataPort))
            {
                ui->lineEditRemotePort->setText(QString::number(rootObj->getValue<uint16_t>(ZapFR::Engine::JSONIdentifierRemoteConfigDataPort)));
            }
            if (rootObj->has(ZapFR::Engine::JSONIdentifierRemoteConfigDataLogin))
            {
                ui->lineEditRemoteLogin->setText(QString::fromUtf8(rootObj->getValue<std::string>(ZapFR::Engine::JSONIdentifierRemoteConfigDataLogin)));
            }
            if (rootObj->has(ZapFR::Engine::JSONIdentifierRemoteConfigDataPassword))
            {
                ui->lineEditRemotePassword->setText(QString::fromUtf8(rootObj->getValue<std::string>(ZapFR::Engine::JSONIdentifierRemoteConfigDataPassword)));
            }
            if (rootObj->has(ZapFR::Engine::JSONIdentifierRemoteConfigDataUseHTTPS))
            {
                ui->checkBoxRemoteUseHTTPS->setChecked(rootObj->getValue<bool>(ZapFR::Engine::JSONIdentifierRemoteConfigDataUseHTTPS));
            }
        }
        catch (...)
        {
        }
    }
    ui->lineEditRemoteHost->setEnabled(remoteFieldsEnabled);
    ui->lineEditRemotePort->setEnabled(remoteFieldsEnabled);
    ui->lineEditRemoteLogin->setEnabled(remoteFieldsEnabled);
    ui->lineEditRemotePassword->setEnabled(remoteFieldsEnabled);
    ui->checkBoxRemoteUseHTTPS->setEnabled(remoteFieldsEnabled);

    auto stats = props["statistics"].value<QMap<uint64_t, QString>>();

    auto feedCountKey = static_cast<std::underlying_type_t<ZapFR::Engine::Source::Statistic>>(ZapFR::Engine::Source::Statistic::FeedCount);
    if (stats.contains(feedCountKey))
    {
        ui->labelFeedCountValue->setText(stats[feedCountKey]);
    }
    else
    {
        ui->labelFeedCountValue->setText(tr("Unknown"));
    }
    auto postCountKey = static_cast<std::underlying_type_t<ZapFR::Engine::Source::Statistic>>(ZapFR::Engine::Source::Statistic::PostCount);
    if (stats.contains(postCountKey))
    {
        ui->labelPostCountValue->setText(stats[postCountKey]);
    }
    else
    {
        ui->labelPostCountValue->setText(tr("Unknown"));
    }
    auto flaggedPostCountKey = static_cast<std::underlying_type_t<ZapFR::Engine::Source::Statistic>>(ZapFR::Engine::Source::Statistic::FlaggedPostCount);
    if (stats.contains(flaggedPostCountKey))
    {
        ui->labelFlaggedPostCountValue->setText(stats[flaggedPostCountKey]);
    }
    else
    {
        ui->labelFlaggedPostCountValue->setText(tr("Unknown"));
    }
    auto oldestPostKey = static_cast<std::underlying_type_t<ZapFR::Engine::Source::Statistic>>(ZapFR::Engine::Source::Statistic::OldestPost);
    if (stats.contains(oldestPostKey) && !stats[oldestPostKey].isEmpty())
    {
        ui->labelOldestPostValue->setText(Utilities::prettyDate(stats[oldestPostKey]));
    }
    else
    {
        ui->labelOldestPostValue->setText(tr("Unknown"));
    }
    auto newestPostKey = static_cast<std::underlying_type_t<ZapFR::Engine::Source::Statistic>>(ZapFR::Engine::Source::Statistic::NewestPost);
    if (stats.contains(newestPostKey) && !stats[newestPostKey].isEmpty())
    {
        ui->labelNewestPostValue->setText(Utilities::prettyDate(stats[newestPostKey]));
    }
    else
    {
        ui->labelNewestPostValue->setText(tr("Unknown"));
    }
}

void ZapFR::Client::WidgetPropertiesPaneSource::save()
{
    auto newTitle = ui->lineEditTitle->text().trimmed();
    if (!newTitle.isEmpty())
    {
        // seeing as the properties of the source are stored locally, even in cases of remove zapfr instances
        // we don't have to go over an agent to update the source
        auto source = ZapFR::Engine::Source::getSource(mSourceID);
        if (source.has_value())
        {
            std::string configData;
            if (source.value()->type() == ZapFR::Engine::IdentifierRemoteServer)
            {
                auto portStr = ui->lineEditRemotePort->text().toStdString();
                uint32_t port{0};
                Poco::NumberParser::tryParseUnsigned(portStr, port);

                auto configDataObj = QJsonObject();
                configDataObj[ZapFR::Engine::JSONIdentifierRemoteConfigDataHost] = ui->lineEditRemoteHost->text();
                configDataObj[ZapFR::Engine::JSONIdentifierRemoteConfigDataPort] = QJsonValue::fromVariant(QVariant::fromValue<uint32_t>(port));
                configDataObj[ZapFR::Engine::JSONIdentifierRemoteConfigDataLogin] = ui->lineEditRemoteLogin->text();
                configDataObj[ZapFR::Engine::JSONIdentifierRemoteConfigDataPassword] = ui->lineEditRemotePassword->text();
                configDataObj[ZapFR::Engine::JSONIdentifierRemoteConfigDataUseHTTPS] =
                    QJsonValue::fromVariant(QVariant::fromValue<bool>(ui->checkBoxRemoteUseHTTPS->isChecked()));
                configData = QJsonDocument(configDataObj).toJson(QJsonDocument::Compact).toStdString();
            }

            source.value()->update(newTitle.toStdString(), configData);

            emit sourceUpdated(source.value()->id());
        }
    }
}
