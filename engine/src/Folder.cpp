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

#include "Folder.h"

ZapFR::Engine::Database* ZapFR::Engine::Folder::msDatabase{nullptr};

ZapFR::Engine::Folder::Folder(uint64_t id, uint64_t parent) : mID(id), mParent(parent)
{
}

void ZapFR::Engine::Folder::registerDatabaseInstance(Database* db)
{
    msDatabase = db;
}

bool ZapFR::Engine::Folder::hasSubfolders()
{
    if (!mSubfoldersFetched)
    {
        fetchSubfolders();
    }
    return mSubfolders.size() > 0;
}

std::vector<ZapFR::Engine::Folder*> ZapFR::Engine::Folder::subfolders()
{
    if (!mSubfoldersFetched)
    {
        fetchSubfolders();
    }

    std::vector<Folder*> folders;
    for (const auto& f : mSubfolders)
    {
        folders.emplace_back(f.get());
    }
    return folders;
}
