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

#ifndef ZAPFR_CLIENT_CLIENTGLOBAL_H
#define ZAPFR_CLIENT_CLIENTGLOBAL_H

#include <QString>

namespace ZapFR
{
    namespace Client
    {
        namespace Setting
        {
            static const QString MainWindowState = "mainwindow.state";
            static const QString MainWindowGeometry = "mainwindow.geometry";
            static const QString SplitterSourcesAndContentPanesState = "splittersourcesandcontentpanes.state";
            static const QString SplitterSourcesAndScriptFoldersState = "splittersourcesandscriptfolders.state";
            static const QString SplitterPostsTableAdPostViewState = "splitterpoststableandpostview.state";
            static const QString SourceTreeViewExpansion = "sourcetreeview.expansion";
            static const QString UITheme = "ui.theme";
            static const QString UIFontSize = "ui.fontsize";
            static const QString UIHideLocalSource = "ui.hidelocalsource";
            static const QString UIMinimizeInsteadOfClose = "ui.minimizeinsteadofclose";
            static const QString UIShowUnreadPostsAtTop = "ui.showunreadpostsattop";
            static const QString PostFontSize = "post.fontsize";
            static const QString PostDetectBrowsers = "post.detectbrowsers";
            static const QString FeedsRefreshBehaviour = "feeds.refreshbehaviour";
            static const QString FeedsAutoRefreshEnabled = "feeds.autorefresh.enabled";
            static const QString FeedsAutoRefreshInterval = "feeds.autorefresh.interval";
            static const QString FeedsLogLevel = "feeds.loglevel";
        } // namespace Setting

        namespace MimeType
        {
            static const QString DraggableFeed = "application/x-zapfr-feed";
            static const QString DraggableFolder = "application/x-zapfr-folder";
            static const QString TestPost = "application/x-zapfr-testpost";
        } // namespace MimeType

        enum ContentPane
        {
            Posts = 0,
            Logs = 1,
            Scripts = 2,
            Properties = 3,
        };

        enum PostPane
        {
            Post = 0,
            PostCaption = 1,
        };

        enum PropertiesPane
        {
            SourceProps = 0,
            FolderProps = 1,
            FeedProps = 2,
        };

        enum class Theme
        {
            Light,
            Dark,
            UseSystem
        };

        enum class RefreshBehaviour
        {
            CurrentSelection,
            EntireSource,
        };

        struct Preferences
        {
            Theme theme{Theme::UseSystem};
            uint16_t uiFontSize{11};
            uint16_t postFontSize{16};
            RefreshBehaviour refreshBehaviour{RefreshBehaviour::CurrentSelection};
            bool detectBrowsers{false};
            bool hideLocalSource{false};
            bool minimizeInsteadOfClose{false};
            bool showUnreadPostsAtTop{false};
        };

        static const uint32_t StatusBarDefaultTimeout{2500};

    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_CLIENTGLOBAL_H