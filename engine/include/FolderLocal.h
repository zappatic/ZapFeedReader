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

#ifndef ZAPFR_ENGINE_FOLDERLOCAL_H
#define ZAPFR_ENGINE_FOLDERLOCAL_H

#include "Folder.h"
#include "Global.h"

namespace ZapFR
{
    namespace Engine
    {
        struct FolderRecord
        {
            uint64_t id;
            uint64_t left;
            uint64_t right;
            uint64_t parent;
            uint64_t nodeOrder;
            std::string title;
        };

        class FolderLocal : public Folder
        {
          public:
            explicit FolderLocal(uint64_t id, uint64_t parent);
            virtual ~FolderLocal() = default;

            void fetchSubfolders() override;
            bool fetchData() override;
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FOLDERLOCAL_H