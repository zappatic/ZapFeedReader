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

void ZapFR::Engine::PostDummy::markFlagged(FlagColor flagColor)
{
    if (flagColor == FlagColor::Gray)
    {
        return;
    }
    markUnflagged(flagColor);
    mFlagColors.insert(flagColor);
}

void ZapFR::Engine::PostDummy::markUnflagged(FlagColor flagColor)
{
    mFlagColors.erase(flagColor);
}

void ZapFR::Engine::PostDummy::markAsRead()
{
    setIsRead(true);
}

void ZapFR::Engine::PostDummy::markAsUnread()
{
    setIsRead(false);
}

void ZapFR::Engine::PostDummy::assignToScriptFolder(uint64_t scriptFolderID)
{
    if (mLogCallback.has_value())
    {
        mLogCallback.value()(fmt::format("Assigned to script folder with ID {}", scriptFolderID));
    }
}

void ZapFR::Engine::PostDummy::unassignFromScriptFolder(uint64_t scriptFolderID)
{
    if (mLogCallback.has_value())
    {
        mLogCallback.value()(fmt::format("Unassigned from script folder with ID {}", scriptFolderID));
    }
}
