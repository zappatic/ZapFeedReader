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

#ifndef ZAPFR_ENGINE_SOURCELOCAL_H
#define ZAPFR_ENGINE_SOURCELOCAL_H

#include "ZapFR/base/Source.h"

namespace ZapFR
{
    namespace Engine
    {
        class SourceLocal : public Source
        {
          public:
            explicit SourceLocal(uint64_t id);
            ~SourceLocal() = default;

            // source stuff
            void fetchStatistics() override;
            void fetchThumbnailData() override;
            std::unordered_set<uint64_t> importOPML(const std::string& opml, uint64_t parentFolderID) override;
            void clearLogs() override;

            // feed stuff
            std::vector<std::unique_ptr<Feed>> getFeeds(uint32_t fetchInfo) override;
            std::optional<std::unique_ptr<Feed>> getFeed(uint64_t feedID, uint32_t fetchInfo) override;
            std::optional<std::unique_ptr<Feed>> addFeed(const std::string& url, uint64_t folder) override;
            std::unordered_map<uint64_t, uint64_t> moveFeed(uint64_t feedID, uint64_t newFolder, uint64_t newSortOrder) override;
            void removeFeed(uint64_t feedID) override;

            // folder stuff
            std::vector<std::unique_ptr<Folder>> getFolders(uint64_t parent, uint32_t fetchInfo) override;
            std::optional<std::unique_ptr<Folder>> getFolder(uint64_t folderID, uint32_t fetchInfo) override;
            void removeFolder(uint64_t folderID) override;
            std::unordered_map<uint64_t, uint64_t> moveFolder(uint64_t folderID, uint64_t newParent, uint64_t newSortOrder) override;
            std::tuple<uint64_t, uint64_t> addFolder(const std::string& title, uint64_t parentID) override;
            uint64_t createFolderHierarchy(uint64_t parentID, const std::vector<std::string>& folderHierarchy);

            // post stuff
            std::tuple<uint64_t, std::vector<std::unique_ptr<Post>>> getPosts(uint64_t perPage, uint64_t page, bool showOnlyUnread, bool showUnreadPostsAtTop,
                                                                              const std::string& searchFilter, uint64_t categoryFilterID, FlagColor flagColor) override;
            void markAsRead(uint64_t maxPostID) override;
            void setPostsReadStatus(bool markAsRead, const std::vector<std::tuple<uint64_t, uint64_t>>& feedsAndPostIDs) override;
            void setPostsFlagStatus(bool markFlagged, const std::unordered_set<FlagColor>& flagColors,
                                    const std::vector<std::tuple<uint64_t, uint64_t>>& feedsAndPostIDs) override;
            void assignPostsToScriptFolder(uint64_t scriptFolderID, bool assign, const std::vector<std::tuple<uint64_t, uint64_t>>& feedsAndPostIDs) override;
            std::unordered_map<uint64_t, uint64_t> getUnreadCounts();
            std::unordered_map<uint64_t, std::string> getFeedErrors();
            Poco::JSON::Object getStatus() override;

            // log stuff
            std::tuple<uint64_t, std::vector<std::unique_ptr<Log>>> getLogs(uint64_t perPage, uint64_t page) override;

            // flag stuff
            std::unordered_set<FlagColor> getUsedFlagColors() override;

            // category stuff
            std::vector<std::unique_ptr<ZapFR::Engine::Category>> getCategories() override;

            // script folder stuff
            std::vector<std::unique_ptr<ScriptFolder>> getScriptFolders() override;
            std::optional<std::unique_ptr<ScriptFolder>> getScriptFolder(uint64_t id, uint32_t fetchInfo) override;
            void addScriptFolder(const std::string& title, bool showTotal, bool showUnread) override;
            void removeScriptFolder(uint64_t scriptFolderID) override;

            // script stuff
            std::vector<std::unique_ptr<Script>> getScripts() override;
            std::optional<std::unique_ptr<Script>> getScript(uint64_t scriptID, uint32_t fetchInfo) override;
            void removeScript(uint64_t scriptID) override;
            void addScript(Script::Type type, const std::string& title, bool enabled, const std::unordered_set<Script::Event>& events,
                           const std::optional<std::unordered_set<uint64_t>>& feedIDs, const std::string& script) override;

          protected:
            std::unordered_map<uint64_t, std::vector<uint64_t>> remapFeedPostTuplesToMap(const std::vector<std::tuple<uint64_t, uint64_t>>& feedsAndPostIDs) const;
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_SOURCELOCAL_H