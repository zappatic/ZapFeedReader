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
#include "ZapFR/Source.h"
#include "ui_WidgetPropertiesPaneSource.h"

ZapFR::Client::WidgetPropertiesPaneSource::WidgetPropertiesPaneSource(QWidget* parent) : QWidget(parent), ui(new Ui::WidgetPropertiesPaneSource)
{
    ui->setupUi(this);

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
    ui->labelTypeValue->setText(props["type"].toString());

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
        // seeing as the title of the source is stored locally, even in cases of remove zapfr instances
        // we don't have to go over an agent to update the source
        auto source = ZapFR::Engine::Source::getSource(mSourceID);
        if (source.has_value())
        {
            source.value()->updateTitle(newTitle.toStdString());
            emit sourceTitleUpdated(source.value()->id(), newTitle);
        }
    }
}
