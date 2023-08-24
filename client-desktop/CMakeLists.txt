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
        src/Utilities.cpp
        src/FeedIconCache.cpp
        src/widgets/MainWindow.cpp
        src/widgets/MainWindow-Posts.cpp
        src/widgets/MainWindow-Sources.cpp
        src/widgets/MainWindow-Feeds.cpp
        src/widgets/MainWindow-Folders.cpp
        src/widgets/MainWindow-Logs.cpp
        src/widgets/MainWindow-Flags.cpp
        src/widgets/MainWindow-Scripts.cpp
        src/widgets/MainWindow-ScriptFolders.cpp
        src/widgets/WebEnginePagePost.cpp
        src/widgets/WebEngineViewPost.cpp
        src/widgets/TableViewPaletteCorrected.cpp
        src/widgets/TableViewPosts.cpp
        src/widgets/TableViewLogs.cpp
        src/widgets/TableViewScriptFolders.cpp
        src/widgets/TableViewScripts.cpp
        src/widgets/TreeViewPaletteCorrected.cpp
        src/widgets/TreeViewSources.cpp
        src/widgets/TreeViewEditScriptDialogSources.cpp
        src/widgets/PopupFlagChooser.cpp
        src/widgets/SearchWidget.cpp
        src/widgets/WidgetPostCaption.cpp
        src/delegates/ItemDelegateSource.cpp
        src/delegates/ItemDelegatePost.cpp
        src/delegates/ItemDelegateLog.cpp
        src/delegates/ItemDelegateScript.cpp
        src/delegates/ItemDelegateEditScriptDialogSource.cpp
        src/dialogs/DialogWithSourcesAndFolders.cpp
        src/dialogs/DialogAddFeed.cpp
        src/dialogs/DialogAddFolder.cpp
        src/dialogs/DialogImportOPML.cpp
        src/dialogs/DialogJumpToPage.cpp
        src/dialogs/DialogEditScript.cpp
        src/dialogs/DialogEditScriptFolder.cpp
        src/models/StandardItemModelSources.cpp
        include/ClientGlobal.h
        include/FeedIconCache.h
        include/Utilities.h
        include/widgets/MainWindow.h
        include/widgets/WebEnginePagePost.h
        include/widgets/WebEngineViewPost.h
        include/widgets/TableViewPaletteCorrected.h
        include/widgets/TableViewPosts.h
        include/widgets/TableViewLogs.h
        include/widgets/TableViewScriptFolders.h
        include/widgets/TableViewScripts.h
        include/widgets/TreeViewPaletteCorrected.h
        include/widgets/TreeViewSources.h
        include/widgets/TreeViewEditScriptDialogSources.h
        include/widgets/PopupFlagChooser.h
        include/widgets/SearchWidget.h
        include/widgets/WidgetPostCaption.h
        include/delegates/ItemDelegateSource.h
        include/delegates/ItemDelegatePost.h
        include/delegates/ItemDelegateLog.h
        include/delegates/ItemDelegateScript.h
        include/delegates/ItemDelegateEditScriptDialogSource.h
        include/dialogs/DialogWithSourcesAndFolders.h
        include/dialogs/DialogAddFeed.h
        include/dialogs/DialogAddFolder.h
        include/dialogs/DialogImportOPML.h
        include/dialogs/DialogJumpToPage.h
        include/dialogs/DialogEditScript.h
        include/dialogs/DialogEditScriptFolder.h
        include/models/StandardItemModelSources.h
        src/widgets/MainWindow.ui
        src/widgets/PopupFlagChooser.ui
        src/widgets/SearchWidget.ui
        src/dialogs/DialogAddFeed.ui
        src/dialogs/DialogAddFolder.ui
        src/dialogs/DialogImportOPML.ui
        src/dialogs/DialogJumpToPage.ui
        src/dialogs/DialogEditScript.ui
        src/dialogs/DialogEditScriptFolder.ui
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

target_link_libraries(zapfeedreader-client PRIVATE  zapfeedreader-engine
                                                    Qt${QT_VERSION_MAJOR}::Widgets 
                                                    Qt${QT_VERSION_MAJOR}::WebEngineWidgets)


install(TARGETS zapfeedreader-client
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(zapfeedreader-client)
endif()
