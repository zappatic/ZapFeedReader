##############################################################################
##### THIS FILE IS AUTO GENERATED                                        #####
##### DO NOT MODIFY THIS FILE, ANY CHANGES WILL BE UNDONE UPON RECOMPILE #####
##### USE CMakeLists.txt.tpl INSTEAD                                     #####
##############################################################################



list(APPEND ZAPFR_SERVER_SOURCES
    Daemon.cpp
	DummyFeed.cpp
    HTTPServer.cpp
    APIRequestHandlerFactory.cpp
    APIRequestHandler.cpp
    APIRequest404Handler.cpp
    APIRequestHandlerRegistration.cpp
    APIRequest.cpp
    API.cpp

    handlers/APIHandler_about.cpp
	handlers/APIHandler_index.cpp
	handlers/categories/APIHandler_categories_list.cpp
	handlers/dummy/APIHandler_dummyfeed_createpost.cpp
	handlers/dummy/APIHandler_dummyfeed_get_atom10.cpp
	handlers/dummy/APIHandler_dummyfeed_get_json11.cpp
	handlers/dummy/APIHandler_dummyfeed_get_rss20.cpp
	handlers/feeds/APIHandler_feed_add.cpp
	handlers/feeds/APIHandler_feed_deletelogs.cpp
	handlers/feeds/APIHandler_feed_get.cpp
	handlers/feeds/APIHandler_feed_markasread.cpp
	handlers/feeds/APIHandler_feed_move.cpp
	handlers/feeds/APIHandler_feed_refresh.cpp
	handlers/feeds/APIHandler_feed_remove.cpp
	handlers/feeds/APIHandler_feed_update.cpp
	handlers/feeds/APIHandler_feeds_list.cpp
	handlers/folders/APIHandler_folder_add.cpp
	handlers/folders/APIHandler_folder_deletelogs.cpp
	handlers/folders/APIHandler_folder_get.cpp
	handlers/folders/APIHandler_folder_markasread.cpp
	handlers/folders/APIHandler_folder_move.cpp
	handlers/folders/APIHandler_folder_remove.cpp
	handlers/folders/APIHandler_folder_sort.cpp
	handlers/folders/APIHandler_folder_update.cpp
	handlers/folders/APIHandler_folders_list.cpp
	handlers/logs/APIHandler_logs_list.cpp
	handlers/posts/APIHandler_post_get.cpp
	handlers/posts/APIHandler_posts_list.cpp
	handlers/scriptfolders/APIHandler_scriptfolder_add.cpp
	handlers/scriptfolders/APIHandler_scriptfolder_assignposts.cpp
	handlers/scriptfolders/APIHandler_scriptfolder_get.cpp
	handlers/scriptfolders/APIHandler_scriptfolder_markasread.cpp
	handlers/scriptfolders/APIHandler_scriptfolder_remove.cpp
	handlers/scriptfolders/APIHandler_scriptfolder_update.cpp
	handlers/scriptfolders/APIHandler_scriptfolders_list.cpp
	handlers/scripts/APIHandler_script_add.cpp
	handlers/scripts/APIHandler_script_get.cpp
	handlers/scripts/APIHandler_script_remove.cpp
	handlers/scripts/APIHandler_script_update.cpp
	handlers/scripts/APIHandler_scripts_list.cpp
	handlers/sources/APIHandler_source_deletelogs.cpp
	handlers/sources/APIHandler_source_getstatus.cpp
	handlers/sources/APIHandler_source_importopml.cpp
	handlers/sources/APIHandler_source_markasread.cpp
	handlers/sources/APIHandler_source_setpostsflagstatus.cpp
	handlers/sources/APIHandler_source_setpostsreadstatus.cpp
	handlers/sources/APIHandler_source_statistics.cpp
	handlers/sources/APIHandler_source_usedflagcolors.cpp

)

