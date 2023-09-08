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

#include "widgets/WidgetPropertiesPaneFolder.h"
#include "Utilities.h"
#include "ZapFR/Agent.h"
#include "ZapFR/base/Folder.h"
#include "ui_WidgetPropertiesPaneFolder.h"

ZapFR::Client::WidgetPropertiesPaneFolder::WidgetPropertiesPaneFolder(QWidget* parent) : QWidget(parent), ui(new Ui::WidgetPropertiesPaneFolder)
{
    ui->setupUi(this);

    auto f = ui->labelCaption->font();
    f.setWeight(QFont::DemiBold);
    ui->labelCaption->setFont(f);
}

ZapFR::Client::WidgetPropertiesPaneFolder::~WidgetPropertiesPaneFolder()
{
    delete ui;
}

void ZapFR::Client::WidgetPropertiesPaneFolder::reset(const QMap<QString, QVariant>& props)
{
    mSourceID = props["sourceID"].toULongLong();
    mFolderID = props["folderID"].toULongLong();

    ui->labelCaption->setText(props["title"].toString());

    auto stats = props["statistics"].value<QMap<uint64_t, QString>>();
    auto feedCountKey = static_cast<std::underlying_type_t<ZapFR::Engine::Folder::Statistic>>(ZapFR::Engine::Folder::Statistic::FeedCount);
    if (stats.contains(feedCountKey))
    {
        ui->labelFeedCountValue->setText(stats[feedCountKey]);
    }
    else
    {
        ui->labelFeedCountValue->setText(tr("Unknown"));
    }
    auto postCountKey = static_cast<std::underlying_type_t<ZapFR::Engine::Folder::Statistic>>(ZapFR::Engine::Folder::Statistic::PostCount);
    if (stats.contains(postCountKey))
    {
        ui->labelPostCountValue->setText(stats[postCountKey]);
    }
    else
    {
        ui->labelPostCountValue->setText(tr("Unknown"));
    }
    auto flaggedPostCountKey = static_cast<std::underlying_type_t<ZapFR::Engine::Folder::Statistic>>(ZapFR::Engine::Folder::Statistic::FlaggedPostCount);
    if (stats.contains(flaggedPostCountKey))
    {
        ui->labelFlaggedPostCountValue->setText(stats[flaggedPostCountKey]);
    }
    else
    {
        ui->labelFlaggedPostCountValue->setText(tr("Unknown"));
    }
    auto oldestPostKey = static_cast<std::underlying_type_t<ZapFR::Engine::Folder::Statistic>>(ZapFR::Engine::Folder::Statistic::OldestPost);
    if (stats.contains(oldestPostKey) && !stats[oldestPostKey].isEmpty())
    {
        ui->labelOldestPostValue->setText(Utilities::prettyDate(stats[oldestPostKey]));
    }
    else
    {
        ui->labelOldestPostValue->setText(tr("Unknown"));
    }
    auto newestPostKey = static_cast<std::underlying_type_t<ZapFR::Engine::Folder::Statistic>>(ZapFR::Engine::Folder::Statistic::NewestPost);
    if (stats.contains(newestPostKey) && !stats[newestPostKey].isEmpty())
    {
        ui->labelNewestPostValue->setText(Utilities::prettyDate(stats[newestPostKey]));
    }
    else
    {
        ui->labelNewestPostValue->setText(tr("Unknown"));
    }
}
