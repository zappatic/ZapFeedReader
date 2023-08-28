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

#include "widgets/WidgetPropertiesPaneFeed.h"
#include "FeedIconCache.h"
#include "Utilities.h"
#include "ZapFR/Agent.h"
#include "ui_WidgetPropertiesPaneFeed.h"

ZapFR::Client::WidgetPropertiesPaneFeed::WidgetPropertiesPaneFeed(QWidget* parent) : QWidget(parent), ui(new Ui::WidgetPropertiesPaneFeed)
{
    ui->setupUi(this);

    auto f = ui->labelCaption->font();
    f.setWeight(QFont::DemiBold);
    ui->labelCaption->setFont(f);
    ui->labelLinkValue->setOpenExternalLinks(true);
    ui->labelLastErrorValue->setStyleSheet("QLabel {color: darkred;}");
    connect(ui->pushButtonSave, &QPushButton::clicked, this, &WidgetPropertiesPaneFeed::save);
}

ZapFR::Client::WidgetPropertiesPaneFeed::~WidgetPropertiesPaneFeed()
{
    delete ui;
}

void ZapFR::Client::WidgetPropertiesPaneFeed::reset(const QMap<QString, QVariant>& props)
{
    mSourceID = props["sourceID"].toULongLong();
    mFeedID = props["feedID"].toULongLong();
    auto pixmap = FeedIconCache::icon(mFeedID);
    if (!pixmap.isNull())
    {
        ui->labelIcon->setPixmap(pixmap);
        ui->labelIcon->setFixedWidth(ui->labelCaption->height());
        ui->labelIcon->setFixedHeight(ui->labelCaption->height());
    }
    ui->labelCaption->setText(props["title"].toString());
    ui->lineEditURL->setText(props["url"].toString());

    auto link = props["link"].toString();
    if (link.startsWith("http"))
    {
        link = QString("<a href=\"%1\">%1</a>").arg(link);
    }
    else if (link.isEmpty())
    {
        link = QString("<i>%1</i>").arg(tr("No link provided"));
    }
    ui->labelLinkValue->setText(link);

    auto description = props["description"].toString();
    if (description.isEmpty())
    {
        description = "<i>No description provided</i>";
    }
    ui->labelDescriptionValue->setText(description);

    auto copyright = props["copyright"].toString();
    if (copyright.isEmpty())
    {
        copyright = "<i>No copyright provided</i>";
    }
    ui->labelCopyrightValue->setText(copyright);
    ui->lineEditRefreshInterval->setText(props["refreshInterval"].toString());

    auto lastError = props["lastError"].toString();
    if (lastError.isEmpty())
    {
        ui->labelLastError->setVisible(false);
        ui->labelLastErrorValue->setVisible(false);
    }
    else
    {
        ui->labelLastError->setVisible(true);
        ui->labelLastErrorValue->setVisible(true);
        ui->labelLastErrorValue->setText(lastError);
    }
    ui->labelLastRefreshedValue->setText(Utilities::prettyDate(props["lastRefreshed"].toString()));
}

void ZapFR::Client::WidgetPropertiesPaneFeed::save()
{
    auto newURL = ui->lineEditURL->text().toStdString();

    std::optional<uint64_t> newRefreshInterval;
    auto enteredString = ui->lineEditRefreshInterval->text().toStdString();
    if (!enteredString.empty())
    {
        uint64_t enteredRefreshInterval{0};
        if (Poco::NumberParser::tryParseUnsigned64(enteredString, enteredRefreshInterval))
        {
            newRefreshInterval = enteredRefreshInterval * 60; // minutes to seconds
            if (newRefreshInterval == 0)
            {
                newRefreshInterval = {};
            }
        }
    }

    ZapFR::Engine::Agent::getInstance()->queueSetFeedProperties(mSourceID, mFeedID, newURL, newRefreshInterval, [&]() {});
}
