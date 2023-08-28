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

#include "./ui_MainWindow.h"
#include "ZapFR/Agent.h"
#include "ZapFR/Feed.h"
#include "widgets/MainWindow.h"
#include "widgets/WidgetPropertiesPaneFeed.h"

void ZapFR::Client::MainWindow::connectPropertiesStuff()
{
    connect(ui->action_View_properties, &QAction::triggered, [&]() { reloadPropertiesPane(); });
}

void ZapFR::Client::MainWindow::reloadPropertiesPane()
{
    auto currentIndex = ui->treeViewSources->currentIndex();
    if (currentIndex.isValid())
    {
        auto sourceID = currentIndex.data(SourceTreeEntryParentSourceIDRole).toULongLong();
        switch (currentIndex.data(SourceTreeEntryTypeRole).toULongLong())
        {
            case SOURCETREE_ENTRY_TYPE_SOURCE:
            {
                ui->stackedWidgetProperties->setCurrentIndex(StackedPanePropertiesSource);
                break;
            }
            case SOURCETREE_ENTRY_TYPE_FOLDER:
            {
                ui->stackedWidgetProperties->setCurrentIndex(StackedPanePropertiesFolder);
                break;
            }
            case SOURCETREE_ENTRY_TYPE_FEED:
            {
                auto feedID = currentIndex.data(SourceTreeEntryIDRole).toULongLong();
                ZapFR::Engine::Agent::getInstance()->queueGetFeed(sourceID, feedID,
                                                                  [&](ZapFR::Engine::Feed* feed)
                                                                  {
                                                                      QMap<QString, QVariant> props;

                                                                      props["sourceID"] = QVariant::fromValue<uint64_t>(sourceID);
                                                                      props["feedID"] = QVariant::fromValue<uint64_t>(feed->id());
                                                                      props["title"] = QString::fromUtf8(feed->title());
                                                                      props["url"] = QString::fromUtf8(feed->url());
                                                                      props["link"] = QString::fromUtf8(feed->link());
                                                                      props["description"] = QString::fromUtf8(feed->description());
                                                                      props["copyright"] = QString::fromUtf8(feed->copyright());
                                                                      props["lastRefreshed"] = QString::fromUtf8(feed->lastChecked());
                                                                      props["refreshInterval"] = "";
                                                                      if (feed->refreshInterval().has_value())
                                                                      {
                                                                          // convert seconds to minutes for the interface
                                                                          props["refreshInterval"] = QString::number(feed->refreshInterval().value() / 60);
                                                                      }
                                                                      props["lastError"] = "";
                                                                      if (feed->lastRefreshError().has_value())
                                                                      {
                                                                          props["lastError"] = QString::fromUtf8(feed->lastRefreshError().value());
                                                                      }

                                                                      QMetaObject::invokeMethod(this, "feedPropertiesReceived", Qt::AutoConnection, props);
                                                                  });
                break;
            }
        }
    }
}

void ZapFR::Client::MainWindow::feedPropertiesReceived(const QMap<QString, QVariant>& props)
{
    ui->stackedWidgetRight->setCurrentIndex(StackedPaneProperties);
    ui->stackedWidgetProperties->setCurrentIndex(StackedPanePropertiesFeed);
    ui->widgetPropertiesPaneFeed->reset(props);
}