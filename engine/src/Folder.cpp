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

#include "ZapFR/Folder.h"

const std::unordered_map<std::string, ZapFR::Engine::Folder::Statistic> ZapFR::Engine::Folder::JSONIdentifierFolderStatisticMap{
    {"feedCount", Statistic::FeedCount},   {"postCount", Statistic::PostCount},   {"flaggedPostCount", Statistic::FlaggedPostCount},
    {"oldestPost", Statistic::OldestPost}, {"newestPost", Statistic::NewestPost},
};

const std::unordered_map<ZapFR::Engine::Folder::Statistic, std::string> ZapFR::Engine::Folder::FolderStatisticJSONIdentifierMap{
    {Statistic::FeedCount, "feedCount"},   {Statistic::PostCount, "postCount"},   {Statistic::FlaggedPostCount, "flaggedPostCount"},
    {Statistic::OldestPost, "oldestPost"}, {Statistic::NewestPost, "newestPost"},
};

ZapFR::Engine::Folder::Folder(uint64_t id, uint64_t parentFolderID, Source* parentSource) : mID(id), mParentFolderID(parentFolderID), mParentSource(parentSource)
{
}

void ZapFR::Engine::Folder::appendSubfolder(std::unique_ptr<Folder> subfolder)
{
    mSubfolders.emplace_back(std::move(subfolder));
}

Poco::JSON::Object ZapFR::Engine::Folder::toJSON()
{
    Poco::JSON::Object o;
    o.set(Folder::JSONIdentifierFolderID, mID);
    o.set(Folder::JSONIdentifierFolderTitle, mTitle);
    o.set(Folder::JSONIdentifierFolderParent, mParentFolderID);
    o.set(Folder::JSONIdentifierFolderSortOrder, mSortOrder);

    if (mStatistics.size() > 0)
    {
        Poco::JSON::Object statsObj;
        for (const auto& [stat, value] : mStatistics)
        {
            statsObj.set(Folder::FolderStatisticJSONIdentifierMap.at(stat), value);
        }
        o.set(Folder::JSONIdentifierFolderStatistics, statsObj);
    }

    Poco::JSON::Array feedIDArr;
    for (const auto& feedID : mFeedIDs)
    {
        feedIDArr.add(feedID);
    }
    o.set(Folder::JSONIdentifierFolderFeedIDs, feedIDArr);

    return o;
}
