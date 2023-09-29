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

#include <Poco/NumberParser.h>

#include "ZapFR/Global.h"
#include "dialogs/DialogAddSource.h"
#include "ui_DialogAddSource.h"

ZapFR::Client::DialogAddSource::DialogAddSource(QWidget* parent) : QDialog(parent), ui(new Ui::DialogAddSource)
{
    ui->setupUi(this);
    ui->comboBoxSourceType->addItem("ZapFeedReader Server", ZapFR::Engine::IdentifierRemoteServer);

    mNetworkAccessManager = std::make_unique<QNetworkAccessManager>();
    connect(mNetworkAccessManager.get(), &QNetworkAccessManager::finished, this, &DialogAddSource::connectionTestFinished);

    for (const auto& button : ui->buttonBox->buttons())
    {
        if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ButtonRole::AcceptRole)
        {
            button->setText(tr("Add source"));
            break;
        }
    }
}

ZapFR::Client::DialogAddSource::~DialogAddSource()
{
    delete ui;
}

void ZapFR::Client::DialogAddSource::reset()
{
    ui->lineEditHostname->setText("");
    ui->lineEditLogin->setText("");
    ui->lineEditPassword->setText("");
    ui->checkBoxUseHTTPS->setChecked(true);
    mServerName = "";
    mConnectTestPerformed = false;
    ui->lineEditHostname->setFocus();
}

void ZapFR::Client::DialogAddSource::accept()
{
    if (mConnectTestPerformed)
    {
        QDialog::accept();
        return;
    }

    // attempt to connect to the source and retrieve the title
    auto hostName = this->hostName();
    auto port = this->port();
    bool useHTTPS = this->useHTTPS();

    if (!hostName.isEmpty())
    {
        if (sourceType() == ZapFR::Engine::IdentifierRemoteServer)
        {
            QUrl url;
            url.setScheme(useHTTPS ? "https" : "http");
            url.setHost(hostName);
            url.setPort(port);
            url.setPath("/about");
            QNetworkRequest request;
            request.setUrl(url);
            auto reply = mNetworkAccessManager->get(request);
            connect(reply, &QNetworkReply::errorOccurred, [&](QNetworkReply::NetworkError) { QMessageBox::warning(this, tr("Connection error"), reply->errorString()); });
        }
    }
    else
    {
        QMessageBox::warning(this, tr("Hostname missing"), tr("Please provide the host name or IP address of the server"));
        ui->lineEditHostname->setFocus();
    }
}

void ZapFR::Client::DialogAddSource::connectionTestFinished(QNetworkReply* reply)
{
    auto httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (httpStatus.isValid() && httpStatus.toInt() == 200)
    {
        auto response = reply->readAll();

        QJsonParseError error;
        auto json = QJsonDocument::fromJson(response, &error);
        if (json.isNull())
        {
            QMessageBox::warning(this, tr("Invalid server reply"), tr("The server responded with an invalid reply (JSON parse error: %1)").arg(error.errorString()));
            return;
        }
        auto root = json.object();
        if (root.contains("version"))
        {
            auto serverVersion = root["version"];
            if (!serverVersion.isDouble())
            {
                QMessageBox::warning(this, tr("Invalid server reply"), tr("The server responded with an invalid reply (Unknown version)"));
                return;
            }
            if (serverVersion.toInt() != ZapFR::Engine::APIVersion)
            {
                QMessageBox::warning(this, tr("Mismatching version"),
                                     tr("The server runs on a different version as this client. Please use the same version on both client and server."));
                return;
            }
        }
        else
        {
            QMessageBox::warning(this, tr("Invalid server reply"), tr("The server responded with an invalid reply (No version)"));
            return;
        }

        if (root.contains("name"))
        {
            auto name = root["name"];
            if (name.isString())
            {
                mServerName = name.toString();
            }
        }

        mConnectTestPerformed = true;
        accept();
    }
}

void ZapFR::Client::DialogAddSource::connectionTestErrorOccurred([[maybe_unused]] QNetworkReply::NetworkError code)
{
}

QString ZapFR::Client::DialogAddSource::sourceType() const noexcept
{
    return ui->comboBoxSourceType->currentData().toString();
}

QString ZapFR::Client::DialogAddSource::hostName() const noexcept
{
    return ui->lineEditHostname->text();
}

uint16_t ZapFR::Client::DialogAddSource::port() const
{
    uint32_t p{0};
    if (Poco::NumberParser::tryParseUnsigned(ui->lineEditPort->text().toStdString(), p))
    {
        return static_cast<uint16_t>(p);
    }
    return ZapFR::Engine::DefaultServerPort;
}

QString ZapFR::Client::DialogAddSource::login() const noexcept
{
    return ui->lineEditLogin->text();
}

QString ZapFR::Client::DialogAddSource::password() const noexcept
{
    return ui->lineEditPassword->text();
}

bool ZapFR::Client::DialogAddSource::useHTTPS() const noexcept
{
    return (ui->checkBoxUseHTTPS->checkState() == Qt::Checked);
}

QString ZapFR::Client::DialogAddSource::serverName() const noexcept
{
    return mServerName;
}
