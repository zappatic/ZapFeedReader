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

    %HANDLERS%

)

