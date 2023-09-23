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

#include "ZapFR/dummy/PostDummy.h"

ZapFR::Engine::PostDummy::PostDummy(uint64_t id) : Post(id)
{
}

void ZapFR::Engine::PostDummy::markFlagged(FlagColor /*flagColor*/)
{
    // if (flagColor == FlagColor::Gray)
    // {
    //     return;
    // }

    // markUnflagged(flagColor);

    // auto fc = Flag::idForFlagColor(flagColor);
    // Poco::Data::Statement insertStmt(*(Database::getInstance()->session()));
    // insertStmt << "INSERT INTO flags (postID, flagID) VALUES (?, ?)", use(mID), use(fc), now;
}

void ZapFR::Engine::PostDummy::markUnflagged(FlagColor /*flagColor*/)
{
    // auto fc = Flag::idForFlagColor(flagColor);
    // Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
    // deleteStmt << "DELETE FROM flags WHERE postID=? AND flagID=?", use(mID), use(fc), now;
}

void ZapFR::Engine::PostDummy::markAsRead()
{
    //    updateIsRead(true, {"posts.feedID=?", "posts.id=?"}, {use(mFeedID, "feedID"), use(mID, "id")});
}

void ZapFR::Engine::PostDummy::markAsUnread()
{
    //    updateIsRead(false, {"posts.feedID=?", "posts.id=?"}, {use(mFeedID, "feedID"), use(mID, "id")});
}

void ZapFR::Engine::PostDummy::assignToScriptFolder(uint64_t /*scriptFolderID*/)
{
    // unassignFromScriptFolder(scriptFolderID);
    // Poco::Data::Statement insertStmt(*(Database::getInstance()->session()));
    // insertStmt << "INSERT INTO scriptfolder_posts (scriptFolderID, postID) VALUES (?, ?)", use(scriptFolderID), use(mID), now;
}

void ZapFR::Engine::PostDummy::unassignFromScriptFolder(uint64_t /*scriptFolderID*/)
{
    // Poco::Data::Statement deleteStmt(*(Database::getInstance()->session()));
    // deleteStmt << "DELETE FROM scriptfolder_posts WHERE postID=? AND scriptFolderID=?", use(mID), use(scriptFolderID), now;
}
