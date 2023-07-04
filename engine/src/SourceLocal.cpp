#include "SourceLocal.h"
#include "Database.h"

ZapFR::Engine::SourceLocal::SourceLocal(Database* db) : Source(db)
{
}

Poco::JSON::Array ZapFR::Engine::SourceLocal::getFeeds()
{
    return Poco::JSON::Array();
}

std::optional<Poco::JSON::Object> ZapFR::Engine::SourceLocal::getFeed(uint64_t feedID)
{
    return mDatabase->getFeed(feedID);
}

Poco::JSON::Array ZapFR::Engine::SourceLocal::getPosts(uint64_t feedID, uint64_t perPage, uint64_t page)
{
    return mDatabase->getPosts(feedID, perPage, page);
}
