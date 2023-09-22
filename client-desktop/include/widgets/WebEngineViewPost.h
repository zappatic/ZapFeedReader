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

#ifndef ZAPFR_CLIENT_WEBENGINEVIEWPOST_H
#define ZAPFR_CLIENT_WEBENGINEVIEWPOST_H

#include "ClientGlobal.h"

namespace ZapFR
{
    namespace Engine
    {
        class Post;
    }

    namespace Client
    {
        class MainWindow;

        struct DetectedBrowser
        {
            DetectedBrowser() = default;
            DetectedBrowser(const QString& bTitle, const QString& bCommand, const std::vector<QString>& bArgs) : title(bTitle), command(bCommand), args(bArgs) {}
            QString title{""};
            QString command{""};
            std::vector<QString> args{};
        };

        class WebEngineViewPost : public QWebEngineView
        {
            Q_OBJECT

          public:
            explicit WebEngineViewPost(QWidget* parent = nullptr);
            ~WebEngineViewPost() = default;
            WebEngineViewPost(const WebEngineViewPost& e) = delete;
            WebEngineViewPost& operator=(const WebEngineViewPost&) = delete;
            WebEngineViewPost(WebEngineViewPost&&) = delete;
            WebEngineViewPost& operator=(WebEngineViewPost&&) = delete;

            void setMainWindow(MainWindow* mainWindow) noexcept { mMainWindow = mainWindow; }
            void setPostHTML(const QString& html);
            void setBlankPostPage();
            QString postStyles() const;
            QString postHTMLTemplate() const;
            void invalidatePostStylesCache() { mPostStylesCacheValid = false; }
            QString getHTMLForPost(ZapFR::Engine::Post* post) const;
            static const std::vector<DetectedBrowser>& detectBrowsers();

          protected:
            void contextMenuEvent(QContextMenuEvent* event) override;

          private:
            QUrl mClickedURL{};
            std::unique_ptr<QMenu> mContextMenu{nullptr};
            MainWindow* mMainWindow{nullptr};

            mutable bool mPostStylesCacheValid{false};

            static std::optional<DetectedBrowser> detectBrowser(const QString& title, const QString& command, const std::vector<QString>& versionArgs,
                                                                const std::vector<QString>& runArgs);
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_WEBENGINEVIEWPOST_H