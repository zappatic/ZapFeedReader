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

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QHeaderView>
#include <QMenu>
#include <QMimeDatabase>
#include <QProcess>

#include "widgets/MainWindow.h"
#include "widgets/TableViewPostEnclosures.h"
#include "widgets/WebEngineViewPost.h"

ZapFR::Client::TableViewPostEnclosures::TableViewPostEnclosures(QWidget* parent) : TableViewPaletteCorrected(parent)
{
    mItemModelPostEnclosures = std::make_unique<QStandardItemModel>(this);
    setModel(mItemModelPostEnclosures.get());

    mContextMenu = std::make_unique<QMenu>(nullptr);
    mCopyLinkAction = std::make_unique<QAction>(tr("Copy URL"));
    mOpenInBrowserAction = std::make_unique<QAction>(tr("Open in external browser"));
    mContextMenu->addAction(mCopyLinkAction.get());
    mContextMenu->addAction(mOpenInBrowserAction.get());

    connect(this, &QTableView::doubleClicked, this, &TableViewPostEnclosures::openEnclosureInExternalBrowser);
    connect(this, &QTableView::customContextMenuRequested, this, &TableViewPostEnclosures::showContextMenu);
    connect(mCopyLinkAction.get(), &QAction::triggered, this, &TableViewPostEnclosures::copyLink);
    connect(mOpenInBrowserAction.get(), &QAction::triggered, this, &TableViewPostEnclosures::openEnclosureInExternalBrowser);
}

void ZapFR::Client::TableViewPostEnclosures::keyPressEvent(QKeyEvent* event)
{
    auto index = currentIndex();
    if (event->key() == Qt::Key_C && ((event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier))
    {
        QApplication::clipboard()->setText(index.data(Role::Link).toString());
        mMainWindow->setStatusBarMessage(tr("Enclosure URL copied to clipboard"));
        return;
    }
    QTableView::keyPressEvent(event);
}

void ZapFR::Client::TableViewPostEnclosures::openEnclosureInExternalBrowser()
{
    auto index = currentIndex();
    if (index.isValid())
    {
        auto link = index.data(Role::Link).toString();
        if (!link.isEmpty() && (link.startsWith("http") || link.startsWith("magnet:")))
        {
            QDesktopServices::openUrl(link);
        }
    }
}

void ZapFR::Client::TableViewPostEnclosures::copyLink()
{
    auto index = currentIndex();
    if (index.isValid())
    {
        auto link = index.data(Role::Link).toString();
        if (!link.isEmpty())
        {
            QGuiApplication::clipboard()->setText(link);
        }
    }
}

void ZapFR::Client::TableViewPostEnclosures::clear()
{
    mItemModelPostEnclosures->clear();
    mItemModelPostEnclosures->setHorizontalHeaderItem(Column::IconCol, new QStandardItem(""));
    mItemModelPostEnclosures->setHorizontalHeaderItem(Column::URLCol, new QStandardItem(tr("URL")));
    mItemModelPostEnclosures->setHorizontalHeaderItem(Column::MimetypeCol, new QStandardItem(tr("Type")));
    mItemModelPostEnclosures->setHorizontalHeaderItem(Column::FilesizeCol, new QStandardItem(tr("Size")));
    horizontalHeader()->setMinimumSectionSize(25);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    horizontalHeader()->setSectionResizeMode(Column::URLCol, QHeaderView::Stretch);
    horizontalHeader()->resizeSection(Column::MimetypeCol, 250);
    horizontalHeader()->resizeSection(Column::IconCol, 25);
}

void ZapFR::Client::TableViewPostEnclosures::loadEnclosures(const std::vector<ZapFR::Engine::Post::Enclosure>& enclosures)
{
    QMimeDatabase mimeDB;
    for (const auto& e : enclosures)
    {
        auto mimeType = mimeDB.mimeTypeForName(QString::fromStdString(e.mimeType));
        auto url = QString::fromStdString(e.url);

        auto icon = mimeType.iconName();
        if (icon.isEmpty())
        {
            icon = mimeType.genericIconName();
        }

        auto iconItem = new QStandardItem("");
        iconItem->setData(QIcon::fromTheme(icon), Qt::DecorationRole);
        iconItem->setData(url, Role::Link);

        auto urlItem = new QStandardItem(url);
        urlItem->setData(url, Qt::ToolTipRole);
        urlItem->setData(url, Role::Link);

        auto mimeTypeItem = new QStandardItem(mimeType.name());
        mimeTypeItem->setData(url, Role::Link);

        auto sizeCaption = tr("Unknown");
        if (e.size > 0)
        {
            sizeCaption = locale().formattedDataSize(static_cast<int64_t>(e.size));
        }
        auto sizeItem = new QStandardItem(sizeCaption);
        sizeItem->setData(url, Role::Link);

        QList<QStandardItem*> rowData;
        rowData << iconItem << urlItem << mimeTypeItem << sizeItem;

        mItemModelPostEnclosures->appendRow(rowData);
    }
}

void ZapFR::Client::TableViewPostEnclosures::showContextMenu(const QPoint& p)
{
    if (mMainWindow->preferences()->detectBrowsers)
    {
        auto browsers = WebEngineViewPost::detectBrowsers();
        static bool browsersAdded{false};
        if (!browsersAdded)
        {
            for (const auto& browser : browsers)
            {
                auto action = new QAction(tr("&Open in %1").arg(browser.title), mContextMenu.get());
                connect(action, &QAction::triggered,
                        [=, this]()
                        {
                            auto index = currentIndex();
                            if (index.isValid())
                            {
                                auto url = index.data(Role::Link).toString();
                                QStringList args;
                                for (auto arg : browser.args)
                                {
                                    args << arg.replace("{url}", url);
                                }
                                qint64 pid{0};
                                QProcess::startDetached(browser.command, args, QString(), &pid);
                            }
                        });
                mContextMenu->addAction(action);
            }
            browsersAdded = true;
        }
    }

    mContextMenu->popup(viewport()->mapToGlobal(p));
}
