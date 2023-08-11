cmake_minimum_required(VERSION 3.5)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets WebEngineWidgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets WebEngineWidgets LinguistTools)

set(TS_FILES i18n/ZapFeedReader_en_US.ts)

set(PROJECT_SOURCES
        src/main.cpp
        src/MainWindow.cpp
        src/ItemDelegateSource.cpp
        src/ItemDelegatePost.cpp
        src/ItemDelegateLog.cpp
        src/DialogWithSourcesAndFolders.cpp
        src/DialogAddFeed.cpp
        src/DialogAddFolder.cpp
        src/DialogImportOPML.cpp
        src/DialogJumpToPage.cpp
        src/Utilities.cpp
        src/WebEnginePagePost.cpp
        src/WebEngineViewPost.cpp
        src/StandardItemModelSources.cpp
        src/TableViewPosts.cpp
        src/TableViewLogs.cpp
        src/TableViewScriptFolders.cpp
        src/TreeViewSources.cpp
        src/FeedIconCache.cpp
        src/PopupFlagChooser.cpp
        include/ClientGlobal.h
        include/MainWindow.h
        include/ItemDelegateSource.h
        include/ItemDelegatePost.h
        include/ItemDelegateLog.h
        include/DialogWithSourcesAndFolders.h
        include/DialogAddFeed.h
        include/DialogAddFolder.h
        include/DialogImportOPML.h
        include/DialogJumpToPage.h
        include/WebEnginePagePost.h
        include/WebEngineViewPost.h
        include/StandardItemModelSources.h
        include/TableViewPosts.h
        include/TableViewLogs.h
        include/TableViewScriptFolders.h
        include/TreeViewSources.h
        include/FeedIconCache.h
        include/PopupFlagChooser.h
        src/MainWindow.ui
        src/DialogAddFeed.ui
        src/DialogAddFolder.ui
        src/DialogImportOPML.ui
        src/DialogJumpToPage.ui
        src/PopupFlagChooser.ui
        resources/resources.qrc
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(zapfeedreader-client MANUAL_FINALIZATION ${PROJECT_SOURCES})
    qt6_add_translations(zapfeedreader-client TS_FILES ${TS_FILES} RESOURCE_PREFIX "i18n")
else()
    add_executable(zapfeedreader-client ${PROJECT_SOURCES} )
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

# poco
if (EXISTS ${CMAKE_SOURCE_DIR}/3rdParty/poco-static/)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/3rdParty/poco/cmake)
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/3rdParty/poco-static/lib/cmake/Poco)
    find_package(PostgreSQL REQUIRED)
    find_package(Poco REQUIRED Foundation Util XML Net NetSSL JSON Data DataSQLite) 
    target_link_libraries(zapfeedreader-client PRIVATE  Poco::Foundation
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
    target_link_libraries(zapfeedreader-client PRIVATE fmt::fmt)
endif()

target_include_directories(zapfeedreader-client PRIVATE include)
target_include_directories(zapfeedreader-client PRIVATE ../engine/include)

target_link_libraries(zapfeedreader-client PRIVATE  Qt${QT_VERSION_MAJOR}::Widgets 
                                                    Qt${QT_VERSION_MAJOR}::WebEngineWidgets)

target_sources(zapfeedreader-client PRIVATE
                                ../engine/src/Helpers.cpp
                                ../engine/src/Agent.cpp
                                ../engine/src/agents/AgentRefreshFeed.cpp
                                ../engine/src/agents/AgentRefreshFolder.cpp
                                ../engine/src/agents/AgentRefreshSource.cpp
                                ../engine/src/agents/AgentRemoveFolder.cpp
                                ../engine/src/agents/AgentRemoveFeed.cpp
                                ../engine/src/agents/AgentSubscribeFeed.cpp
                                ../engine/src/agents/AgentGetFeedPosts.cpp
                                ../engine/src/agents/AgentGetFolderPosts.cpp
                                ../engine/src/agents/AgentGetSourcePosts.cpp
                                ../engine/src/agents/AgentGetSourceTree.cpp
                                ../engine/src/agents/AgentAddFolder.cpp
                                ../engine/src/agents/AgentMoveFeed.cpp
                                ../engine/src/agents/AgentMoveFolder.cpp
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
                                ../engine/src/agents/AgentGetSourceFlaggedPosts.cpp
                                ../engine/src/agents/AgentGetFolderFlaggedPosts.cpp
                                ../engine/src/agents/AgentGetFeedFlaggedPosts.cpp
                                ../engine/src/agents/AgentGetScriptFolders.cpp
                                ../engine/src/agents/AgentGetScriptFolderPosts.cpp
                                ../engine/src/Database.cpp
                                ../engine/src/Log.cpp
                                ../engine/src/FeedFetcher.cpp
                                ../engine/src/FeedParser.cpp
                                ../engine/src/FeedParserRSS20.cpp
                                ../engine/src/FeedParserATOM10.cpp
                                ../engine/src/Source.cpp
                                ../engine/src/SourceLocal.cpp
                                ../engine/src/Folder.cpp
                                ../engine/src/FolderLocal.cpp
                                ../engine/src/FeedLocal.cpp
                                ../engine/src/PostLocal.cpp
                                ../engine/src/OPMLParser.cpp
                                ../engine/src/FavIconParser.cpp
                                ../engine/src/Flag.cpp
                                ../engine/src/ScriptFolderLocal.cpp
)

install(TARGETS zapfeedreader-client
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(zapfeedreader-client)
endif()
