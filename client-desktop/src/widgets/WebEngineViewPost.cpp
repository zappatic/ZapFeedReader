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
#include "FeedIconCache.h"
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

        // clang-format off
        cache = QString(R"(body { font-family: "%1", sans-serif; font-size: %5px; background-color: %2; color: %3; margin: 2px 25px; })"
                        "\n"
                        "a { color: %4; }\n"
                        ".zapfr_title { color: %3; font-size: 1.4em; font-weight: bold; text-decoration: none; display: block; margin: 25px 0 10px 0; user-select:none; }\n"
                        ".zapfr_infoheader { font-size: 0.75em; display: flex; gap: 10px; }\n"
                        ".zapfr_infoheader_separator { display: inline-block; margin-right: 10px; }\n"
                        ".zapfr_divider { margin-bottom: 30px; height: 1px; border: none; color: %3; background-color: %3; }\n"
                        ".zapfr_thumbnail_feedheader { color: %3; font-size: 1.4em; font-weight: bold; text-decoration: none; display: block; margin: 25px 0 10px 0;\n"
                        "   user-select:none; display: flex; flex-direction: row; gap: 15px; border-bottom: 1px solid %3; padding-bottom: 6px;\n"
                        "}\n"
                        ".zapfr_thumbnail_feedicon { max-width: 25px; max-height: 25px; }\n"
                        ".zapfr_thumbnail_grid { display: grid; grid-template-columns: repeat(6, 1fr); grid-column-gap: 10px; grid-row-gap: 20px; margin: 25px 0 65px 0; }\n"
                        ".zapfr_thumbnail_cell { display: flex; flex-direction: column; align-items:center; position:relative; }\n"
                        ".zapfr_thumbnail_cell_closebtn { display:none; position:absolute; right:0px; top:0px; width:25px; height:25px; }\n"
                        ".zapfr_thumbnail_cell:hover .zapfr_thumbnail_cell_closebtn { display:block; }\n"
                        "\n"
                        "@media screen and (min-width:0px) and (max-width:850px) {\n"
                        "   .zapfr_thumbnail_grid { grid-template-columns: repeat(2, 1fr); }\n"
                        "}\n"
                        "@media screen and (min-width:851px) and (max-width:1100px) {\n"
                        "   .zapfr_thumbnail_grid { grid-template-columns: repeat(3, 1fr); }\n"
                        "}\n"
                        "@media screen and (min-width:1101px) and (max-width:1350px) {\n"
                        "   .zapfr_thumbnail_grid { grid-template-columns: repeat(4, 1fr); }\n"
                        "}\n"
                        "@media screen and (min-width:1351px) and (max-width:1600px) {\n"
                        "   .zapfr_thumbnail_grid { grid-template-columns: repeat(5, 1fr); }\n"
                        "}\n")
                    .arg(font.family())
                    .arg(backgroundColor)
                    .arg(textColor)
                    .arg(highlightColor.name())
                    .arg(bodyFontSize);
        // clang-format on
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

QString ZapFR::Client::WebEngineViewPost::getHTMLForThumbnailData(uint64_t sourceID, const std::vector<ZapFR::Engine::ThumbnailData>& thumbnailData) const
{
    if (thumbnailData.empty())
    {
        return "";
    }

    QString html;
    auto ss = QTextStream(&html);
    ss << R"(<!DOCTYPE html>
<html>
    <head>
        <style type="text/css">)"
       << postStyles() << R"(</style>
    </head>
    <body>)";

    for (const auto& td : thumbnailData)
    {
        auto icon = FeedIconCache::base64icon(sourceID, td.feedID);

        ss << R"(<div>)"
           << R"(  <svg width="0" height="0">)"
           << R"(   <svg id="svgCloseBtn" width="25px" height="25px" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">)"
           << R"(       <path fill="#b5251b" fill-rule="evenodd" clip-rule="evenodd" d="M22 12C22 17.5228 17.5228 22 12 22C6.47715 22 2 17.5228 2 12C2 6.47715 6.47715 2 12 2C17.5228 2 22 6.47715 22 12ZM8.96963 8.96965C9.26252 8.67676 9.73739 8.67676 10.0303 8.96965L12 10.9393L13.9696 8.96967C14.2625 8.67678 14.7374 8.67678 15.0303 8.96967C15.3232 9.26256 15.3232 9.73744 15.0303 10.0303L13.0606 12L15.0303 13.9696C15.3232 14.2625 15.3232 14.7374 15.0303 15.0303C14.7374 15.3232 14.2625 15.3232 13.9696 15.0303L12 13.0607L10.0303 15.0303C9.73742 15.3232 9.26254 15.3232 8.96965 15.0303C8.67676 14.7374 8.67676 14.2625 8.96965 13.9697L10.9393 12L8.96963 10.0303C8.67673 9.73742 8.67673 9.26254 8.96963 8.96965Z" />)"
           << R"(   </svg>)"
           << R"(  </svg>)"
           << R"(</div>)"
           << R"(<h1 class="zapfr_thumbnail_feedheader">)"
           << R"(<img class="zapfr_thumbnail_feedicon" src=")" << icon << R"(" />)" << QString::fromUtf8(td.feedTitle) << "</h1>"
           << R"(<div class="zapfr_thumbnail_grid">)";
        for (const auto& tdp : td.posts)
        {
            auto qbaLink = QByteArray(tdp.link.c_str(), static_cast<ssize_t>(tdp.link.length()));
            auto b64Link = QString::fromUtf8(qbaLink.toBase64());
            auto openURL =
                QString("zapfr://viewPostAndMarkUnread?postID=%1&amp;feedID=%2&amp;sourceID=%3&amp;link=%4").arg(tdp.postID).arg(td.feedID).arg(sourceID).arg(b64Link);
            auto cellIdentifier = QString("p%1f%2s%3").arg(tdp.postID).arg(td.feedID).arg(sourceID);
            auto markAsReadArgs = QString("%1,%2,%3").arg(tdp.postID).arg(td.feedID).arg(sourceID);
            ss << R"(<div class="zapfr_thumbnail_cell" id=")" << cellIdentifier << R"(">)"
               << R"(   <div class="zapfr_thumbnail_cell_closebtn">)"
               << R"(       <a href="javascript:markAsRead()" << markAsReadArgs << R"();">)"
               << R"(           <svg width="25px" height="25px"><use xlink:href="#svgCloseBtn"/></svg>)"
               << R"(       </a>)"
               << R"(   </div>)"
               << R"(   <div>)"
               << R"(       <a href=")" << openURL << R"(">)"
               << R"(           <img src=")" << QString::fromUtf8(tdp.thumbnail) << R"(" alt="" width="250" />)"
               << R"(       </a>)"
               << R"(   </div>)"
               << R"(   <div style="text-align:center;">)"
               << R"(       <a href=")" << openURL << R"(">)" << QString::fromUtf8(tdp.title) << R"(       </a>)"
               << R"(     </div>)"
               << R"(</div>)";
        }
        ss << "</div>";
    }
    ss << R"(<script type="text/javascript">)"
       << "\n"
       << R"(   function markAsRead(postID, feedID, sourceID) {)"
       << "\n"
       << R"(       document.getElementById(`p${postID}f${feedID}s${sourceID}`).remove();)"
       << "\n"
       << R"(       document.location.href = `zapfr://markUnread?postID=${postID}&feedID=${feedID}&sourceID=${sourceID}`;)"
       << "\n"
       << R"(   })"
       << "\n"
       << R"(</script>)"
       << R"(</body></html>)";

    return html;
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
