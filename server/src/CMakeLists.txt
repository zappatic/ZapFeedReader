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

    handlers/APIHandler_index.cpp

)

