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

#include "Source.h"

namespace ZapFR
{
    namespace Engine
    {
        class Database;
        class Feed;

        class SourceLocal : public Source
        {
          public:
            explicit SourceLocal(uint64_t id);
            ~SourceLocal() = default;

            std::vector<std::unique_ptr<Feed>> getFeeds() override;
            std::optional<std::unique_ptr<Feed>> getFeed(uint64_t feedID) override;
            void addFeed(const std::string& url, uint64_t folder) override;
            void moveFeed(uint64_t feedID, uint64_t newFolder, uint64_t newSortOrder) override;
            void removeFeed(uint64_t feedID) override;
            void removeFolder(uint64_t folder) override;
            void addFolder(const std::string& title, uint64_t parentID) override;

            std::vector<std::unique_ptr<Folder>> getFolders(uint64_t parent) override;
            std::optional<std::unique_ptr<Folder>> getFolder(uint64_t folderID) override;
            void getSubfolderIDs(uint64_t parent, std::vector<uint64_t>& ids, bool includeParent) override;

          private:
            std::mutex mInsertFeedMutex{};

            void resortFeeds(uint64_t folder) const;
            void resortFolders(uint64_t folder) const;
            uint64_t getNextFeedSortOrder(uint64_t folder) const;
            uint64_t getNextFolderSortOrder(uint64_t folder) const;
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_SOURCELOCAL_H