add_executable(zapfeedreader-server)

# poco
if (EXISTS ${CMAKE_SOURCE_DIR}/3rdParty/poco-static/)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/3rdParty/poco/cmake)
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/3rdParty/poco-static/lib/cmake/Poco)
    find_package(PostgreSQL REQUIRED)
    find_package(Poco REQUIRED Foundation Util XML Net NetSSL JSON Data DataSQLite) 
    target_link_libraries(zapfeedreader-server  Poco::Foundation
                                                Poco::Util
                                                Poco::XML
                                                Poco::Net
                                                Poco::NetSSL
                                                Poco::JSON
                                                Poco::Data
                                                Poco::DataSQLite)

endif()

# fmtlib
if (EXISTS ${CMAKE_SOURCE_DIR}/3rdParty/fmtlib-install/)
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/3rdParty/fmtlib-install/lib/cmake/fmt)
    find_package(fmt)
    target_link_libraries(zapfeedreader-server fmt::fmt)
endif()

target_include_directories(zapfeedreader-server PRIVATE include)
target_include_directories(zapfeedreader-server PRIVATE ../engine/include)

add_subdirectory(src)

target_sources(zapfeedreader-server PRIVATE
    ../engine/src/Helpers.cpp
    ../engine/src/Agent.cpp
    ../engine/src/AgentRefreshFeed.cpp
    ../engine/src/AgentRemoveFolder.cpp
    ../engine/src/AgentSubscribeFeed.cpp
    ../engine/src/AgentGetPosts.cpp
    ../engine/src/AgentMarkPostRead.cpp
    ../engine/src/Database.cpp
    ../engine/src/FeedFetcher.cpp
    ../engine/src/FeedParser.cpp
    ../engine/src/FeedParserRSS20.cpp
    ../engine/src/FeedParserATOM10.cpp
    ../engine/src/Source.cpp
    ../engine/src/SourceLocal.cpp
    ../engine/src/Feed.cpp
    ../engine/src/FeedLocal.cpp
    ../engine/src/Post.cpp
    ../engine/src/OPMLParser.cpp
)