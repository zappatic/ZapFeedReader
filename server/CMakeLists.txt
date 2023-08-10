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
    ../engine/src/agents/AgentRefreshFeed.cpp
    ../engine/src/agents/AgentRefreshFolder.cpp
    ../engine/src/agents/AgentRefreshSource.cpp
    ../engine/src/agents/AgentRemoveFeed.cpp
    ../engine/src/agents/AgentRemoveFolder.cpp
    ../engine/src/agents/AgentSubscribeFeed.cpp
    ../engine/src/agents/AgentAddFolder.cpp
    ../engine/src/agents/AgentMoveFeed.cpp
    ../engine/src/agents/AgentMoveFolder.cpp
    ../engine/src/agents/AgentGetFeedPosts.cpp
    ../engine/src/agents/AgentGetFolderPosts.cpp
    ../engine/src/agents/AgentGetSourcePosts.cpp
    ../engine/src/agents/AgentGetSourceTree.cpp
    ../engine/src/agents/AgentMarkPostRead.cpp
    ../engine/src/agents/AgentMarkPostsUnread.cpp
    ../engine/src/agents/AgentMarkFeedRead.cpp
    ../engine/src/agents/AgentMarkFolderRead.cpp
    ../engine/src/agents/AgentMarkSourceRead.cpp
    ../engine/src/agents/AgentGetPost.cpp
    ../engine/src/agents/AgentGetSourceLogs.cpp
    ../engine/src/agents/AgentGetFolderLogs.cpp
    ../engine/src/agents/AgentGetFeedLogs.cpp
    ../engine/src/agents/AgentMarkPostFlagged.cpp
    ../engine/src/agents/AgentMarkPostUnflagged.cpp
    ../engine/src/agents/AgentGetFeedUnreadCount.cpp
    ../engine/src/agents/AgentGetUsedFlagColors.cpp
    ../engine/src/Database.cpp
    ../engine/src/Log.cpp
    ../engine/src/FeedFetcher.cpp
    ../engine/src/FeedParser.cpp
    ../engine/src/FeedParserRSS20.cpp
    ../engine/src/FeedParserATOM10.cpp
    ../engine/src/Source.cpp
    ../engine/src/SourceLocal.cpp
    ../engine/src/FeedLocal.cpp
    ../engine/src/PostLocal.cpp
    ../engine/src/Folder.cpp
    ../engine/src/FolderLocal.cpp
    ../engine/src/OPMLParser.cpp
    ../engine/src/FavIconParser.cpp
    ../engine/src/Flag.cpp
)