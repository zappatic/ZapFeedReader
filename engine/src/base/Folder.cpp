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

#include "ZapFR/base/Folder.h"

const std::unordered_map<std::string, ZapFR::Engine::Folder::Statistic> ZapFR::Engine::Folder::JSONIdentifierFolderStatisticMap{
    {JSON::Statistic::FeedCount, Statistic::FeedCount},
    {JSON::Statistic::PostCount, Statistic::PostCount},
    {JSON::Statistic::FlaggedPostCount, Statistic::FlaggedPostCount},
    {JSON::Statistic::OldestPost, Statistic::OldestPost},
    {JSON::Statistic::NewestPost, Statistic::NewestPost},
};

const std::unordered_map<ZapFR::Engine::Folder::Statistic, std::string> ZapFR::Engine::Folder::FolderStatisticJSONIdentifierMap{
    {Statistic::FeedCount, JSON::Statistic::FeedCount},
    {Statistic::PostCount, JSON::Statistic::PostCount},
    {Statistic::FlaggedPostCount, JSON::Statistic::FlaggedPostCount},
    {Statistic::OldestPost, JSON::Statistic::OldestPost},
    {Statistic::NewestPost, JSON::Statistic::NewestPost},
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
    o.set(JSON::Folder::ID, mID);
    o.set(JSON::Folder::Title, mTitle);
    o.set(JSON::Folder::Parent, mParentFolderID);
    o.set(JSON::Folder::SortOrder, mSortOrder);

    Poco::JSON::Array subfolders;
    for (const auto& subfolder : mSubfolders)
    {
        subfolders.add(subfolder->toJSON());
    }
    o.set(JSON::Folder::Subfolders, subfolders);

    if (mStatistics.size() > 0)
    {
        Poco::JSON::Object statsObj;
        for (const auto& [stat, value] : mStatistics)
        {
            statsObj.set(Folder::FolderStatisticJSONIdentifierMap.at(stat), value);
        }
        o.set(JSON::Folder::Statistics, statsObj);
    }

    Poco::JSON::Array feedIDArr;
    for (const auto& feedID : mFeedIDs)
    {
        feedIDArr.add(feedID);
    }
    o.set(JSON::Folder::FeedIDs, feedIDArr);

    return o;
}
