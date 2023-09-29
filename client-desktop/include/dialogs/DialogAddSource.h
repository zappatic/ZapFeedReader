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

#ifndef ZAPFR_CLIENT_DIALOGADDSOURCE_H
#define ZAPFR_CLIENT_DIALOGADDSOURCE_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "../ClientGlobal.h"

namespace Ui
{
    class DialogAddSource;
}

namespace ZapFR
{
    namespace Client
    {
        class DialogAddSource : public QDialog
        {
            Q_OBJECT

          public:
            explicit DialogAddSource(QWidget* parent = nullptr);
            ~DialogAddSource();
            DialogAddSource(const DialogAddSource& e) = delete;
            DialogAddSource& operator=(const DialogAddSource&) = delete;
            DialogAddSource(DialogAddSource&&) = delete;
            DialogAddSource& operator=(DialogAddSource&&) = delete;

            void reset();

            QString sourceType() const noexcept;
            QString hostName() const noexcept;
            uint16_t port() const;
            QString login() const noexcept;
            QString password() const noexcept;
            bool useHTTPS() const noexcept;

            QString serverName() const noexcept;

          private slots:
            void accept();
            void connectionTestFinished(QNetworkReply* reply);
            void connectionTestErrorOccurred(QNetworkReply::NetworkError code);

          private:
            Ui::DialogAddSource* ui;
            QString mServerName{""};

            QNetworkReply* mCurrentReply{nullptr};
            bool mConnectTestPerformed{false};
            std::unique_ptr<QNetworkAccessManager> mNetworkAccessManager{nullptr};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_DIALOGADDSOURCE_H
