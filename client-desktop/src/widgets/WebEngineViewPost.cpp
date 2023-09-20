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
#include "widgets/MainWindow.h"

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

        if (mMainWindow->currentPreferenceDetectBrowsers())
        {

            static bool browsersDetected{false};
            static std::vector<DetectedBrowser> browsers;
            if (!browsersDetected)
            {
                auto firefox = detectBrowser("Firefox", "firefox", {"--version"}, {"{url}"});
                if (firefox.has_value())
                {
                    browsers.emplace_back(firefox.value());
                }

                auto chrome = detectBrowser("Chrome", "google-chrome-stable", {"--version"}, {"{url}"});
                if (chrome.has_value())
                {
                    browsers.emplace_back(chrome.value());
                }

                auto chromium = detectBrowser("Chromium", "chromium", {"--version"}, {"{url}"});
                if (chromium.has_value())
                {
                    browsers.emplace_back(chromium.value());
                }

                auto opera = detectBrowser("Opera", "opera", {"--version"}, {"{url}"});
                if (opera.has_value())
                {
                    browsers.emplace_back(opera.value());
                }

                auto brave = detectBrowser("Brave", "brave", {"--version"}, {"{url}"});
                if (brave.has_value())
                {
                    browsers.emplace_back(brave.value());
                }
                browsersDetected = true;
            }

            for (const auto& browser : browsers)
            {
                auto action = new QAction(tr("&Open in %1").arg(browser.title), mContextMenu.get());
                connect(action, &QAction::triggered,
                        [&]()
                        {
                            QStringList args;
                            for (auto arg : browser.args)
                            {
                                args << arg.replace("{url}", mClickedURL.toString());
                            }

                            qint64 pid{0};
                            QProcess::startDetached(browser.command, args, QString(), &pid);
                        });
                mContextMenu->addAction(action);
            }
        }
    }

    auto lcmrData = lastContextMenuRequest();
    mClickedURL = lcmrData->linkUrl();
    if (mClickedURL.isValid())
    {
        mContextMenu->popup(event->globalPos());
    }
}

std::optional<ZapFR::Client::WebEngineViewPost::DetectedBrowser>
ZapFR::Client::WebEngineViewPost::detectBrowser(const QString& title, const QString& command, const std::vector<QString>& versionArgs, const std::vector<QString>& runArgs)
{
    auto process = std::make_unique<QProcess>(this);
    QStringList args;
    for (const auto& a : versionArgs)
    {
        args << a;
    }
    process->start(command, args);
    if (process->waitForFinished())
    {
        DetectedBrowser b;
        b.title = title;
        b.command = command;
        b.args = runArgs;
        return b;
    }
    return {};
}