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

#define FMT_HEADER_ONLY
#include <fmt/core.h>

#include <Poco/URI.h>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QProcess>
#include <QWebEngineContextMenuRequest>

#include "./ui_MainWindow.h"
#include "widgets/MainWindow.h"
#include "widgets/WebEngineViewPost.h"

bool ZapFR::Client::WebEngineViewPost::msBrowsersDetected{false};
std::vector<ZapFR::Client::DetectedBrowser> ZapFR::Client::WebEngineViewPost::msDetectedBrowsers{};
std::mutex ZapFR::Client::WebEngineViewPost::msDetectBrowsersMutex{};

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

    if (mMainWindow->preferences()->detectBrowsers)
    {
        auto browsers = detectBrowsers();
        static bool browsersAdded{false};
        if (!browsersAdded)
        {
            for (const auto& browser : browsers)
            {
                auto action = new QAction(tr("&Open in %1").arg(browser.title), mContextMenu.get());
                connect(action, &QAction::triggered,
                        [=, this]()
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
            browsersAdded = true;
        }
    }

    auto lcmrData = lastContextMenuRequest();
    mClickedURL = lcmrData->linkUrl();
    if (mClickedURL.isValid())
    {
        mContextMenu->popup(event->globalPos());
    }
}

void ZapFR::Client::WebEngineViewPost::setPostHTML(const QString& html)
{
    setHtml(html);
    mMainWindow->getUI()->stackedWidgetPost->setCurrentIndex(PostPane::Post);
}

void ZapFR::Client::WebEngineViewPost::setBlankPostPage()
{
    QString htmlStr;
    QTextStream html(&htmlStr, QIODeviceBase::ReadWrite);
    html << "<!DOCTYPE html>\n<html><head><style type='text/css'>\n" << postStyles() << "</style></head><body></body></html>";
    setPostHTML(htmlStr);
}

QString ZapFR::Client::WebEngineViewPost::postStyles() const
{
    static QString cache{""};

    if (!mPostStylesCacheValid)
    {
        auto tvs = mMainWindow->treeViewSources();
        auto font = tvs->font();
        auto palette = QPalette(tvs->palette());

        QString overrideFilename;
        QString backgroundColor;
        QString textColor;
        QColor highlightColor = palette.color(QPalette::Active, QPalette::Highlight);
        QString bodyFontSize = QString::number(mMainWindow->preferences()->postFontSize);

        if (mMainWindow->getCurrentColorTheme() == Theme::Dark)
        {
            overrideFilename = "posttheme.dark.css";
            backgroundColor = "#2a2a2a";
            textColor = "#fff";
        }
        else
        {
            overrideFilename = "posttheme.light.css";
            backgroundColor = "#fff";
            textColor = "#000";
        }

        auto override = QFile(QDir::cleanPath(mMainWindow->configDir() + QDir::separator() + overrideFilename));
        if (override.exists())
        {
            override.open(QIODeviceBase::ReadOnly);
            auto styles = QString::fromUtf8(override.readAll());
            override.close();
            return styles;
        }

        cache = QString(R"(body { font-family: "%1", sans-serif; font-size: %5px; background-color: %2; color: %3; margin: 2px 25px; })"
                        "\n"
                        "a { color: %4; }\n"
                        ".zapfr_title { color: %3; font-size: 1.4em; font-weight: bold; text-decoration: none; display: block; margin: 25px 0 10px 0; user-select:none; }\n"
                        ".zapfr_infoheader { font-size: 0.75em; display: flex; gap: 10px; }\n"
                        ".zapfr_infoheader_separator { display: inline-block; margin-right: 10px; }\n"
                        ".zapfr_divider { margin-bottom: 30px; height: 1px; border: none; color: %3; background-color: %3; }\n")
                    .arg(font.family())
                    .arg(backgroundColor)
                    .arg(textColor)
                    .arg(highlightColor.name())
                    .arg(bodyFontSize);
        mPostStylesCacheValid = true;
    }

    return cache;
}

QString ZapFR::Client::WebEngineViewPost::postHTMLTemplate() const
{
    static std::optional<QString> cache{};
    if (!cache.has_value())
    {
        auto override = QFile(QDir::cleanPath(mMainWindow->configDir() + QDir::separator() + "post.html"));
        if (override.exists())
        {
            override.open(QIODeviceBase::ReadOnly);
            cache = QString::fromUtf8(override.readAll());
            override.close();
        }
        else
        {
            cache = R"(<!DOCTYPE html>
<html>
    <head>
        <base href="[BASE]">
        <style type="text/css">[STYLES]</style>
    </head>
    <body>
        [if POST.LINK]<a class="zapfr_title" href="[POST.LINK]">[POST.TITLE]</a>[else]<h1 class="zapfr_title">[POST.TITLE]</h1>[/if]
        <div class="zapfr_infoheader">
            <div>[I18N.PUBLISHED]: [POST.DATE_PUBLISHED]</div>
            [if POST.AUTHOR]<div><span class="zapfr_infoheader_separator">|</span>[I18N.AUTHOR]: [POST.AUTHOR]</div>[/if]
            [if POST.COMMENTS_URL]<div><span class="zapfr_infoheader_separator">|</span><a href="[POST.COMMENTS_URL]">[I18N.VIEWCOMMENTS]</a></div>[/if]
        </div>
        <hr class="zapfr_divider">
        [POST.CONTENT]
    </body>
</html>)";
        }
    }
    return cache.value();
}

QString ZapFR::Client::WebEngineViewPost::getHTMLForPost(ZapFR::Engine::Post* post) const
{
    if (post == nullptr)
    {
        return "";
    }

    Poco::URI uri(post->feedLink());
    uri.setPath("");

    std::unordered_map<std::string, QString> replacers;
    replacers["BASE"] = QString::fromUtf8(uri.toString());
    replacers["STYLES"] = postStyles();
    replacers["POST.TITLE"] = QString::fromUtf8(post->title());
    replacers["POST.LINK"] = QString::fromUtf8(post->link());
    replacers["POST.AUTHOR"] = QString::fromUtf8(post->author());
    replacers["POST.CONTENT"] = QString::fromUtf8(post->content());
    replacers["POST.DATE_PUBLISHED"] = Utilities::prettyDate(QString::fromUtf8(post->datePublished()));
    replacers["POST.COMMENTS_URL"] = QString::fromUtf8(post->commentsURL());
    replacers["I18N.PUBLISHED"] = tr("Published");
    replacers["I18N.AUTHOR"] = tr("Author");
    replacers["I18N.VIEWCOMMENTS"] = tr("View comments");

    auto postHTML = postHTMLTemplate();
    for (const auto& [key, value] : replacers)
    {
        // handle {if}{else}{/if}
        auto pattern = QString::fromUtf8(fmt::format(R"(\[if {}\](.*?)\[else\](.*?)\[/if\])", key));
        postHTML.replace(QRegularExpression(pattern), value.isEmpty() ? R"(\2)" : R"(\1)");

        // handle [if][/if]
        pattern = QString::fromUtf8(fmt::format(R"(\[if {}\](.*?)\[/if\])", key));
        postHTML.replace(QRegularExpression(pattern), value.isEmpty() ? "" : R"(\1)");

        postHTML.replace(QString::fromUtf8(fmt::format("[{}]", key)), value);
    }
    return postHTML;
}

const std::vector<ZapFR::Client::DetectedBrowser>& ZapFR::Client::WebEngineViewPost::detectBrowsers()
{
    std::lock_guard<std::mutex> lock(msDetectBrowsersMutex);
    if (!msBrowsersDetected)
    {
        auto firefox = detectBrowser("Firefox", "firefox", {"--version"}, {"{url}"});
        if (firefox.has_value())
        {
            msDetectedBrowsers.emplace_back(firefox.value());
        }

        auto chrome = detectBrowser("Chrome", "google-chrome-stable", {"--version"}, {"{url}"});
        if (chrome.has_value())
        {
            msDetectedBrowsers.emplace_back(chrome.value());
        }

        auto chromium = detectBrowser("Chromium", "chromium", {"--version"}, {"{url}"});
        if (chromium.has_value())
        {
            msDetectedBrowsers.emplace_back(chromium.value());
        }

        auto opera = detectBrowser("Opera", "opera", {"--version"}, {"{url}"});
        if (opera.has_value())
        {
            msDetectedBrowsers.emplace_back(opera.value());
        }

        auto brave = detectBrowser("Brave", "brave", {"--version"}, {"{url}"});
        if (brave.has_value())
        {
            msDetectedBrowsers.emplace_back(brave.value());
        }
        msBrowsersDetected = true;
    }
    return msDetectedBrowsers;
}

std::optional<ZapFR::Client::DetectedBrowser> ZapFR::Client::WebEngineViewPost::detectBrowser(const QString& title, const QString& command,
                                                                                              const std::vector<QString>& versionArgs, const std::vector<QString>& runArgs)
{
    auto process = std::make_unique<QProcess>();
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
