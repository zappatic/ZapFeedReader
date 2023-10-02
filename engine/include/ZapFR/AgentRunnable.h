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

#ifndef ZAPFR_ENGINE_AGENTRUNNABLE_H
#define ZAPFR_ENGINE_AGENTRUNNABLE_H

#include <functional>

#include <Poco/Runnable.h>

namespace ZapFR
{
    namespace Engine
    {
        class Source;

        class AgentRunnable : public Poco::Runnable
        {
          public:
            explicit AgentRunnable(uint64_t sourceID);
            virtual ~AgentRunnable() = default;

            enum class Type
            {
                FeedAdd,
                FeedClearLogs,
                FeedGet,
                FeedGetLogs,
                FeedGetPosts,
                FeedGetUnreadCount,
                FeedMarkRead,
                FeedMove,
                FeedRefresh,
                FeedRemove,
                FeedSetProperties,
                FolderAdd,
                FolderClearLogs,
                FolderGet,
                FolderGetLogs,
                FolderGetPosts,
                FolderMarkRead,
                FolderMove,
                FolderRefresh,
                FolderRemove,
                FolderSort,
                FolderUpdate,
                FoldersGet,
                MonitorFeedRefreshCompletion,
                MonitorSourceReloadCompletion,
                PostGet,
                PostsMarkFlagged,
                PostsMarkRead,
                PostsMarkUnflagged,
                PostsMarkUnread,
                ScriptFolderAdd,
                ScriptFolderAssignPosts,
                ScriptFolderGetPosts,
                ScriptFolderRemove,
                ScriptFolderRemovePosts,
                ScriptFoldersGet,
                ScriptFolderUpdate,
                ScriptAdd,
                ScriptRemove,
                ScriptsGet,
                ScriptUpdate,
                SourceClearLogs,
                SourceGet,
                SourceGetLogs,
                SourceGetPosts,
                SourceGetTree,
                SourceGetUnreadCounts,
                SourceGetUsedFlagColors,
                SourceImportOPML,
                SourceMarkRead,
                SourceRefresh,
            };

            virtual Type type() const noexcept = 0;
            virtual void payload(Source* source) = 0;
            virtual void onPayloadException([[maybe_unused]] Source* source){};

            void run() override;
            bool isDone() const noexcept { return mIsDone; }
            void setShouldAbort(bool b) { mShouldAbort = b; }

          protected:
            bool mIsDone{false};
            bool mShouldAbort{false};

            uint64_t mSourceID{0};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_AGENTRUNNABLE_H