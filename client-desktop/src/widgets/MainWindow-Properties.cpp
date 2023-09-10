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
#include "ZapFR/base/Feed.h"
#include "ZapFR/base/Folder.h"
#include "ZapFR/base/Source.h"
#include "models/StandardItemModelSources.h"
#include "widgets/MainWindow.h"
#include "widgets/WidgetPropertiesPaneFeed.h"
#include "widgets/WidgetPropertiesPaneFolder.h"
#include "widgets/WidgetPropertiesPaneSource.h"

void ZapFR::Client::MainWindow::connectPropertiesStuff()
{
    connect(ui->action_View_properties, &QAction::triggered, [&]() { reloadPropertiesPane(); });
    connect(ui->widgetPropertiesPaneSource, &WidgetPropertiesPaneSource::sourceTitleUpdated,
            [&](uint64_t sourceID, const QString& newTitle)
            {
                auto root = mItemModelSources->invisibleRootItem();
                for (int32_t i = 0; i < root->rowCount(); ++i)
                {
                    auto child = root->child(i);
                    if (child->data(SourceTreeEntryTypeRole).toULongLong() == SOURCETREE_ENTRY_TYPE_SOURCE && child->data(SourceTreeEntryIDRole).toULongLong() == sourceID)
                    {
                        child->setData(newTitle, Qt::DisplayRole);
                    }
                }
            });
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
                ZapFR::Engine::Agent::getInstance()->queueGetSource(sourceID,
                                                                    [&](ZapFR::Engine::Source* source)
                                                                    {
                                                                        QMap<QString, QVariant> props;
                                                                        props["sourceID"] = QVariant::fromValue<uint64_t>(source->id());
                                                                        props["title"] = QString::fromUtf8(source->title());
                                                                        props["type"] = QString::fromUtf8(source->type());

                                                                        QMap<uint64_t, QString> stats;
                                                                        for (const auto& [s, v] : source->statistics())
                                                                        {
                                                                            stats[static_cast<std::underlying_type_t<ZapFR::Engine::Folder::Statistic>>(s)] =
                                                                                QString::fromUtf8(v);
                                                                        }
                                                                        props["statistics"] = QVariant::fromValue<QMap<uint64_t, QString>>(stats);

                                                                        QMetaObject::invokeMethod(this, "sourcePropertiesReceived", Qt::AutoConnection, props);
                                                                    });
                break;
            }
            case SOURCETREE_ENTRY_TYPE_FOLDER:
            {
                auto folderID = currentIndex.data(SourceTreeEntryIDRole).toULongLong();
                ZapFR::Engine::Agent::getInstance()->queueGetFolder(sourceID, folderID,
                                                                    [&](ZapFR::Engine::Folder* folder)
                                                                    {
                                                                        QMap<QString, QVariant> props;

                                                                        props["sourceID"] = QVariant::fromValue<uint64_t>(sourceID);
                                                                        props["folderID"] = QVariant::fromValue<uint64_t>(folder->id());
                                                                        props["title"] = QString::fromUtf8(folder->title());

                                                                        QMap<uint64_t, QString> stats;
                                                                        for (const auto& [s, v] : folder->statistics())
                                                                        {
                                                                            stats[static_cast<std::underlying_type_t<ZapFR::Engine::Folder::Statistic>>(s)] =
                                                                                QString::fromUtf8(v);
                                                                        }
                                                                        props["statistics"] = QVariant::fromValue<QMap<uint64_t, QString>>(stats);

                                                                        QMetaObject::invokeMethod(this, "folderPropertiesReceived", Qt::AutoConnection, props);
                                                                    });
                break;
            }
            case SOURCETREE_ENTRY_TYPE_FEED:
            {
                auto feedID = currentIndex.data(SourceTreeEntryIDRole).toULongLong();
                ZapFR::Engine::Agent::getInstance()->queueGetFeed(sourceID, feedID,
                                                                  [&](uint64_t retrievedSourceID, ZapFR::Engine::Feed* feed)
                                                                  {
                                                                      QMap<QString, QVariant> props;

                                                                      props["sourceID"] = QVariant::fromValue<uint64_t>(retrievedSourceID);
                                                                      props["feedID"] = QVariant::fromValue<uint64_t>(feed->id());
                                                                      props["title"] = QString::fromUtf8(feed->title());
                                                                      props["subtitle"] = QString::fromUtf8(feed->subtitle());
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

                                                                      QMap<uint64_t, QString> stats;
                                                                      for (const auto& [s, v] : feed->statistics())
                                                                      {
                                                                          stats[static_cast<std::underlying_type_t<ZapFR::Engine::Feed::Statistic>>(s)] = QString::fromUtf8(v);
                                                                      }
                                                                      props["statistics"] = QVariant::fromValue<QMap<uint64_t, QString>>(stats);

                                                                      QMetaObject::invokeMethod(this, "feedPropertiesReceived", Qt::AutoConnection, props);
                                                                  });
                break;
            }
        }
    }
}

void ZapFR::Client::MainWindow::feedPropertiesReceived(const QMap<QString, QVariant>& props)
{
    ui->stackedWidgetContentPanes->setCurrentIndex(StackedPaneProperties);
    ui->stackedWidgetProperties->setCurrentIndex(StackedPanePropertiesFeed);
    ui->widgetPropertiesPaneFeed->reset(props);
}

void ZapFR::Client::MainWindow::folderPropertiesReceived(const QMap<QString, QVariant>& props)
{
    ui->stackedWidgetContentPanes->setCurrentIndex(StackedPaneProperties);
    ui->stackedWidgetProperties->setCurrentIndex(StackedPanePropertiesFolder);
    ui->widgetPropertiesPaneFolder->reset(props);
}

void ZapFR::Client::MainWindow::sourcePropertiesReceived(const QMap<QString, QVariant>& props)
{
    ui->stackedWidgetContentPanes->setCurrentIndex(StackedPaneProperties);
    ui->stackedWidgetProperties->setCurrentIndex(StackedPanePropertiesSource);
    ui->widgetPropertiesPaneSource->reset(props);
}
