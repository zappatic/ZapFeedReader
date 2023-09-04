##############################################################################
##### THIS FILE IS AUTO GENERATED                                        #####
##### DO NOT MODIFY THIS FILE, ANY CHANGES WILL BE UNDONE UPON RECOMPILE #####
##### USE CMakeLists.txt.tpl INSTEAD                                     #####
##############################################################################



target_sources(zapfeedreader-server PRIVATE
    main.cpp
    Daemon.cpp
    HTTPServer.cpp
    APIRequestHandlerFactory.cpp
    APIRequestHandler.cpp
    APIRequest404Handler.cpp
    APIRequestHandlerRegistration.cpp
    APIRequest.cpp
    API.cpp

    handlers/APIHandler_about.cpp
	handlers/APIHandler_index.cpp
	handlers/feeds/APIHandler_feed_add.cpp
	handlers/feeds/APIHandler_feed_get.cpp
	handlers/feeds/APIHandler_feed_markasread.cpp
	handlers/feeds/APIHandler_feed_move.cpp
	handlers/feeds/APIHandler_feed_remove.cpp
	handlers/feeds/APIHandler_feeds_list.cpp
	handlers/folders/APIHandler_folder_add.cpp
	handlers/folders/APIHandler_folder_get.cpp
	handlers/folders/APIHandler_folder_markasread.cpp
	handlers/folders/APIHandler_folder_move.cpp
	handlers/folders/APIHandler_folder_remove.cpp
	handlers/folders/APIHandler_folders_list.cpp
	handlers/posts/APIHandler_posts_list.cpp
	handlers/sources/APIHandler_source_markasread.cpp
	handlers/sources/APIHandler_source_setpostsreadstatus.cpp
	handlers/sources/APIHandler_source_statistics.cpp

)

