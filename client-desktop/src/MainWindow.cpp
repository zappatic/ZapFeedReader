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

#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "Feed.h"
#include "ItemDelegateSource.h"
#include "Source.h"
#include <QDir>
#include <QStandardItem>
#include <QStandardPaths>

ZapFR::Client::MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    mDatabase = std::make_unique<ZapFR::Engine::Database>(QDir::cleanPath(dataDir() + QDir::separator() + "zapfeedreader-client.db").toStdString());
    ZapFR::Engine::Source::registerDatabaseInstance(mDatabase.get());
    ZapFR::Engine::Feed::registerDatabaseInstance(mDatabase.get());

    ui->setupUi(this);
    connect(ui->action_Add_source, &QAction::triggered, this, &MainWindow::addSource);

    reloadSources();
    ui->treeViewSources->setModel(mItemModelSources.get());
    ui->treeViewSources->setItemDelegate(new ItemDelegateSource(ui->treeViewSources));
}

ZapFR::Client::MainWindow::~MainWindow()
{
    delete ui;
}

void ZapFR::Client::MainWindow::addSource()
{
    std::cout << "tst\n";
}

void ZapFR::Client::MainWindow::reloadSources()
{
    mItemModelSources = std::make_unique<QStandardItemModel>(this);

    auto sources = ZapFR::Engine::Source::getSources({});
    for (const auto& source : sources)
    {
        QStandardItem* sourceItem = new QStandardItem(QString::fromUtf8(source->title()));
        mItemModelSources->appendRow(sourceItem);

        auto feeds = source->getFeeds();
        for (const auto& feed : feeds)
        {
            QStandardItem* feedItem = new QStandardItem(QString::fromUtf8(feed->title()));
            sourceItem->appendRow(feedItem);
        }
    }
}

QString ZapFR::Client::MainWindow::dataDir() const
{
    auto data = QStandardPaths::locate(QStandardPaths::StandardLocation::GenericDataLocation, "", QStandardPaths::LocateDirectory);

    auto dir = QDir(QDir::cleanPath(data + QDir::separator() + "ZapFeedReader"));
    if (!dir.exists())
    {
        dir.mkpath("ZapFeedReader");
    }
    return dir.path();
}
