cmake_minimum_required(VERSION 3.5)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Xml Network WebEngineWidgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Xml Network WebEngineWidgets LinguistTools)

set(TS_FILES i18n/ZapFeedReader_en_US.ts)

set(PROJECT_SOURCES
        src/main.cpp
        src/Utilities.cpp
        src/FeedIconCache.cpp
        src/SyntaxHighlighterLua.cpp
        src/widgets/FrameFlagFilters.cpp
        src/widgets/MainWindow.cpp
        src/widgets/WebEnginePagePost.cpp
        src/widgets/WebEngineViewPost.cpp
        src/widgets/TableViewPaletteCorrected.cpp
        src/widgets/TableViewPosts.cpp
        src/widgets/TableViewPostEnclosures.cpp
        src/widgets/TableViewLogs.cpp
        src/widgets/TableViewScriptFolders.cpp
        src/widgets/TableViewScripts.cpp
        src/widgets/TreeViewPaletteCorrected.cpp
        src/widgets/TreeViewSources.cpp
        src/widgets/TreeViewEditScriptDialogSources.cpp
        src/widgets/PopupFlagChooser.cpp
        src/widgets/LabelPaletteCorrected.cpp
        src/widgets/LineEditSearch.cpp
        src/widgets/WidgetPostCaption.cpp
        src/widgets/WidgetPropertiesPaneFeed.cpp
        src/widgets/WidgetPropertiesPaneFolder.cpp
        src/widgets/WidgetPropertiesPaneSource.cpp
        src/delegates/ItemDelegateDiscoveredFeed.cpp
        src/delegates/ItemDelegateSource.cpp
        src/delegates/ItemDelegatePost.cpp
        src/delegates/ItemDelegateLog.cpp
        src/delegates/ItemDelegateScript.cpp
        src/delegates/ItemDelegateEditScriptDialogSource.cpp
        src/delegates/ItemDelegateScriptFolder.cpp
        src/dialogs/DialogWithSourcesAndFolders.cpp
        src/dialogs/DialogAddFeed.cpp
        src/dialogs/DialogAddFolder.cpp
        src/dialogs/DialogEditFolder.cpp
        src/dialogs/DialogAddSource.cpp
        src/dialogs/DialogImportOPML.cpp
        src/dialogs/DialogJumpToPage.cpp
        src/dialogs/DialogEditScript.cpp
        src/dialogs/DialogEditScriptFolder.cpp
        src/dialogs/DialogTestScriptEditEnclosure.cpp
        src/dialogs/DialogPreferences.cpp
        src/models/SortFilterProxyModelSources.cpp
        include/ClientGlobal.h
        include/FeedIconCache.h
        include/SyntaxHighlighterLua.h
        include/Utilities.h
        include/widgets/ComboBoxWithPopupSignal.h
        include/widgets/FrameFlagFilters.h
        include/widgets/MainWindow.h
        include/widgets/WebEnginePagePost.h
        include/widgets/WebEngineViewPost.h
        include/widgets/TableViewPaletteCorrected.h
        include/widgets/TableViewPosts.h
        include/widgets/TableViewPostEnclosures.h
        include/widgets/TableViewLogs.h
        include/widgets/TableViewScriptFolders.h
        include/widgets/TableViewScripts.h
        include/widgets/TreeViewPaletteCorrected.h
        include/widgets/TreeViewSources.h
        include/widgets/TreeViewEditScriptDialogSources.h
        include/widgets/PopupFlagChooser.h
        include/widgets/LabelPaletteCorrected.h
        include/widgets/LineEditSearch.h
        include/widgets/WidgetPostCaption.h
        include/widgets/WidgetPropertiesPaneFeed.h
        include/widgets/WidgetPropertiesPaneFolder.h
        include/widgets/WidgetPropertiesPaneSource.h
        include/delegates/ItemDelegateDiscoveredFeed.h
        include/delegates/ItemDelegateSource.h
        include/delegates/ItemDelegatePost.h
        include/delegates/ItemDelegateLog.h
        include/delegates/ItemDelegateScript.h
        include/delegates/ItemDelegateEditScriptDialogSource.h
        include/delegates/ItemDelegateScriptFolder.h
        include/dialogs/DialogWithSourcesAndFolders.h
        include/dialogs/DialogAddFeed.h
        include/dialogs/DialogAddFolder.h
        include/dialogs/DialogEditFolder.h
        include/dialogs/DialogAddSource.h
        include/dialogs/DialogImportOPML.h
        include/dialogs/DialogJumpToPage.h
        include/dialogs/DialogEditScript.h
        include/dialogs/DialogEditScriptFolder.h
        include/dialogs/DialogTestScriptEditEnclosure.h
        include/dialogs/DialogPreferences.h
        include/models/SortFilterProxyModelSources.h
        src/widgets/MainWindow.ui
        src/widgets/PopupFlagChooser.ui
        src/widgets/WidgetPropertiesPaneFeed.ui
        src/widgets/WidgetPropertiesPaneFolder.ui
        src/widgets/WidgetPropertiesPaneSource.ui
        src/dialogs/DialogAddFeed.ui
        src/dialogs/DialogAddFolder.ui
        src/dialogs/DialogEditFolder.ui
        src/dialogs/DialogAddSource.ui
        src/dialogs/DialogImportOPML.ui
        src/dialogs/DialogJumpToPage.ui
        src/dialogs/DialogEditScript.ui
        src/dialogs/DialogEditScriptFolder.ui
        src/dialogs/DialogTestScriptEditEnclosure.ui
        src/dialogs/DialogPreferences.ui
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

target_include_directories(zapfeedreader-client PRIVATE include)
target_include_directories(zapfeedreader-client PRIVATE ../engine/include)
target_include_directories(zapfeedreader-client SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/3rdParty/fmtlib/include)

target_link_libraries(zapfeedreader-client PRIVATE  zapfeedreader-engine
                                                    PkgConfig::lua
                                                    Qt${QT_VERSION_MAJOR}::Widgets 
                                                    Qt${QT_VERSION_MAJOR}::Xml 
                                                    Qt${QT_VERSION_MAJOR}::Network
                                                    Qt${QT_VERSION_MAJOR}::WebEngineWidgets
                                                    Poco::Foundation
                                                    Poco::Util
                                                    Poco::XML
                                                    Poco::Net
                                                    Poco::NetSSL
                                                    Poco::JSON
                                                    Poco::Data
                                                    Poco::DataSQLite)


install(TARGETS zapfeedreader-client
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(zapfeedreader-client)
endif()

add_custom_target(deploy-linux-appimage COMMAND ${CMAKE_COMMAND} -E env QTDIR="${QT_DIR}" ${CMAKE_SOURCE_DIR}/deploy/client-desktop/AppImage/BuildAppImage.py WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
