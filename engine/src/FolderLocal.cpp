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

#include "FolderLocal.h"

using namespace Poco::Data::Keywords;

ZapFR::Engine::FolderLocal::FolderLocal(uint64_t id, uint64_t parent) : Folder(id, parent)
{
}

bool ZapFR::Engine::FolderLocal::fetchData()
{
    if (!mDataFetched)
    {
        Poco::Data::Statement selectStmt(*(msDatabase->session()));
        selectStmt << "SELECT title"
                      ",sortOrder"
                      " FROM folders"
                      " WHERE id=?",
            use(mID), into(mTitle), into(mSortOrder), now;

        mDataFetched = true;
        auto rs = Poco::Data::RecordSet(selectStmt);
        return (rs.rowCount() == 1);
    }
    return true;
}

void ZapFR::Engine::FolderLocal::fetchSubfolders()
{
    if (!mSubfoldersFetched)
    {
        uint64_t subID;
        uint64_t subSortOrder;
        std::string subTitle;

        Poco::Data::Statement selectStmt(*(msDatabase->session()));
        selectStmt << "SELECT id"
                      ",title"
                      ",sortOrder"
                      " FROM folders"
                      " WHERE parent=?"
                      " ORDER BY sortOrder ASC",
            use(mID), into(subID), into(subTitle), into(subSortOrder), range(0, 1);

        while (!selectStmt.done())
        {
            if (selectStmt.execute() > 0)
            {
                auto f = std::make_unique<FolderLocal>(subID, mID);
                f->setTitle(subTitle);
                f->setSortOrder(subSortOrder);
                f->setDataFetched(true);
                mSubfolders.emplace_back(std::move(f));
            }
        }

        mSubfoldersFetched = true;
    }
}
