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

#include "widgets/TableViewPostEnclosures.h"

ZapFR::Client::TableViewPostEnclosures::TableViewPostEnclosures(QWidget* parent) : TableViewPaletteCorrected(parent)
{
    mItemModelPostEnclosures = std::make_unique<QStandardItemModel>(this);
    setModel(mItemModelPostEnclosures.get());

    mContextMenu = std::make_unique<QMenu>(nullptr);
    mCopyLinkAction = std::make_unique<QAction>(tr("Copy link"));
    mOpenInBrowserAction = std::make_unique<QAction>(tr("Open in browser"));
    mContextMenu->addAction(mOpenInBrowserAction.get());
    mContextMenu->addAction(mCopyLinkAction.get());

    connect(this, &QTableView::doubleClicked, this, &TableViewPostEnclosures::openEnclosureInExternalBrowser);
    connect(this, &QTableView::customContextMenuRequested, [&](const QPoint& p) { mContextMenu->popup(viewport()->mapToGlobal(p)); });
    connect(mCopyLinkAction.get(), &QAction::triggered, this, &TableViewPostEnclosures::copyLink);
    connect(mOpenInBrowserAction.get(), &QAction::triggered, this, &TableViewPostEnclosures::openEnclosureInExternalBrowser);
}

void ZapFR::Client::TableViewPostEnclosures::openEnclosureInExternalBrowser()
{
    auto index = currentIndex();
    if (index.isValid())
    {
        auto link = index.data(Role::Link).toString();
        if (!link.isEmpty() && link.startsWith("http"))
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
        auto mimeType = mimeDB.mimeTypeForName(QString::fromUtf8(e.mimeType));
        auto url = QString::fromUtf8(e.url);

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