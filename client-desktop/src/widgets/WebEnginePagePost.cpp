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

#include <QDesktopServices>
#include <QUrlQuery>
#include <QWebEngineSettings>

#include "ZapFR/Agent.h"
#include "widgets/TableViewPosts.h"
#include "widgets/WebEnginePagePost.h"

ZapFR::Client::WebEnginePagePost::WebEnginePagePost(QObject* parent) : QWebEnginePage(parent)
{
    auto s = settings();
    s->setAttribute(QWebEngineSettings::JavascriptEnabled, true); // needed for embedded YouTube videos
    s->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    s->setAttribute(QWebEngineSettings::LocalStorageEnabled, false);
    s->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);
    s->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
    s->setAttribute(QWebEngineSettings::NavigateOnDropEnabled, false);
}

bool ZapFR::Client::WebEnginePagePost::acceptNavigationRequest(const QUrl& url, QWebEnginePage::NavigationType type, bool /*isMainFrame*/)
{
    switch (type)
    {
        case QWebEnginePage::NavigationTypeTyped:
        {
            return true;
        }
        case QWebEnginePage::NavigationTypeLinkClicked:
        {
            if (url.scheme() == "zapfr" && url.host() == "viewpostandmarkunread")
            {
                auto q = QUrlQuery(url);
                if (q.hasQueryItem("postID") && q.hasQueryItem("feedID") && q.hasQueryItem("sourceID") && q.hasQueryItem("link"))
                {
                    bool ok;
                    uint64_t postID = q.queryItemValue("postID").toUInt(&ok);
                    if (!ok)
                    {
                        return false;
                    }
                    uint64_t feedID = q.queryItemValue("feedID").toUInt(&ok);
                    if (!ok)
                    {
                        return false;
                    }
                    uint64_t sourceID = q.queryItemValue("sourceID").toUInt(&ok);
                    if (!ok)
                    {
                        return false;
                    }

                    auto link = QString(QByteArray::fromBase64(q.queryItemValue("link").toUtf8()));
                    if (!link.isEmpty())
                    {
                        QDesktopServices::openUrl(link);
                    }

                    ZapFR::Engine::Agent::getInstance()->queueMarkPostsRead(sourceID, {{feedID, postID}},
                                                                            [&](uint64_t affectedSourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs)
                                                                            {
                                                                                auto tableVieWPosts = qobject_cast<TableViewPosts*>(parent());
                                                                                QMetaObject::invokeMethod(
                                                                                    this, [=]() { tableVieWPosts->postsMarkedRead(affectedSourceID, feedAndPostIDs); });
                                                                            });
                }
            }
            else
            {
                QDesktopServices::openUrl(url);
            }
            return false;
        }
        case QWebEnginePage::NavigationTypeOther:
        {
            return true; // needed for embedded YouTube videos
        }
        default:
        {
            return false;
        }
    }
}
