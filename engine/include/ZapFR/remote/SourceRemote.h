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

#ifndef ZAPFR_ENGINE_SOURCEREMOTE_H
#define ZAPFR_ENGINE_SOURCEREMOTE_H

#include "../Source.h"

namespace ZapFR
{
    namespace Engine
    {
        class SourceRemote : public Source
        {
          public:
            explicit SourceRemote(uint64_t id);
            ~SourceRemote() = default;

            // feed stuff
            std::vector<std::unique_ptr<Feed>> getFeeds() override;
            std::optional<std::unique_ptr<Feed>> getFeed(uint64_t feedID, uint32_t feedFetchInfo) override;
            uint64_t addFeed(const std::string& url, uint64_t folder) override;
            void moveFeed(uint64_t feedID, uint64_t newFolder, uint64_t newSortOrder) override;
            void removeFeed(uint64_t feedID) override;

            // folder stuff
            std::vector<std::unique_ptr<Folder>> getFolders(uint64_t parent) override;
            std::optional<std::unique_ptr<Folder>> getFolder(uint64_t folderID, uint32_t folderFetchInfo) override;
            void removeFolder(uint64_t folderID) override;
            void moveFolder(uint64_t folderID, uint64_t newParent, uint64_t newSortOrder) override;
            uint64_t addFolder(const std::string& title, uint64_t parentID) override;
            uint64_t createFolderHierarchy(uint64_t parentID, const std::vector<std::string>& folderHierarchy) override;

            // post stuff
            std::tuple<uint64_t, std::vector<std::unique_ptr<Post>>> getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, const std::string& searchFilter,
                                                                              FlagColor flagColor) override;
            void markAllAsRead() override;
            void markPostsAsRead(const std::vector<std::tuple<uint64_t, uint64_t>>& feedsAndPostIDs) override;

            // log stuff
            std::vector<std::unique_ptr<Log>> getLogs(uint64_t perPage, uint64_t page) override;
            uint64_t getTotalLogCount() override;

            // flag stuff
            std::unordered_set<FlagColor> getUsedFlagColors() override;

            // script folder stuff
            std::vector<std::unique_ptr<ScriptFolder>> getScriptFolders() override;
            std::optional<std::unique_ptr<ScriptFolder>> getScriptFolder(uint64_t id) override;
            void addScriptFolder(const std::string& title) override;
            void removeScriptFolder(uint64_t scriptFolderID) override;

            // script stuff
            std::vector<std::unique_ptr<Script>> getScripts() override;
            std::optional<std::unique_ptr<Script>> getScript(uint64_t scriptID) override;
            void removeScript(uint64_t scriptID) override;
            void addScript(Script::Type type, const std::string& filename, bool enabled, const std::unordered_set<Script::Event>& events,
                           const std::optional<std::unordered_set<uint64_t>>& feedIDs) override;

            // statistics
            void fetchStatistics() override;

            Poco::URI remoteURL() const;
            bool remoteURLIsValid() const noexcept { return mRemoteURLIsValid; }
            std::string remoteLogin() const noexcept { return mRemoteLogin; }
            std::string remotePassword() const noexcept { return mRemotePassword; }

          private:
            mutable Poco::URI mRemoteURL{};
            mutable std::string mRemoteLogin{""};
            mutable std::string mRemotePassword{""};
            mutable bool mRemoteURLIsValid{false};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_SOURCEREMOTE_H
