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

#include "widgets/WebEngineViewPost.h"

ZapFR::Client::WebEngineViewPost::WebEngineViewPost(QWidget* parent) : QWebEngineView(parent)
{
}

void ZapFR::Client::WebEngineViewPost::contextMenuEvent(QContextMenuEvent* event)
{
    if (mContextMenu == nullptr)
    {
        mContextMenu = std::make_unique<QMenu>();

        auto copyURLAction = new QAction(tr("&Copy link address"), mContextMenu.get());
        connect(copyURLAction, &QAction::triggered, [&]() { QGuiApplication::clipboard()->setText(mClickedURL.toString()); });
        mContextMenu->addAction(copyURLAction);

        auto openInBrowserAction = new QAction(tr("&Open in external browser"), mContextMenu.get());
        connect(openInBrowserAction, &QAction::triggered, [&]() { QDesktopServices::openUrl(mClickedURL); });
        mContextMenu->addAction(openInBrowserAction);
    }

    auto lcmrData = lastContextMenuRequest();
    mClickedURL = lcmrData->linkUrl();
    if (mClickedURL.isValid())
    {
        mContextMenu->popup(event->globalPos());
    }
}
