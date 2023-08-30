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

    %HANDLERS%

)

