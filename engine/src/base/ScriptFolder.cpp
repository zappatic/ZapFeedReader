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

#include "ZapFR/base/ScriptFolder.h"
#include "ZapFR/base/Source.h"

ZapFR::Engine::ScriptFolder::ScriptFolder(uint64_t id, Source* parentSource) : mID(id), mParentSource(parentSource)
{
}

Poco::JSON::Object ZapFR::Engine::ScriptFolder::toJSON()
{
    Poco::JSON::Object o;
    o.set(JSON::ScriptFolder::ID, mID);
    o.set(JSON::ScriptFolder::Title, mTitle);
    o.set(JSON::ScriptFolder::ShowTotal, mShowTotal);
    o.set(JSON::ScriptFolder::ShowUnread, mShowUnread);
    o.set(JSON::ScriptFolder::TotalPostCount, mTotalPostCount);
    o.set(JSON::ScriptFolder::TotalUnreadCount, mTotalUnreadCount);
    return o;
}
