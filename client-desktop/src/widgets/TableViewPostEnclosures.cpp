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

ZapFR::Client::TableViewPostEnclosures::TableViewPostEnclosures(QWidget* parent) : QTableView(parent)
{
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
        auto link = index.data(PostEnclosureLinkRole).toString();
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
        auto link = index.data(PostEnclosureLinkRole).toString();
        if (!link.isEmpty())
        {
            QGuiApplication::clipboard()->setText(link);
        }
    }
}
