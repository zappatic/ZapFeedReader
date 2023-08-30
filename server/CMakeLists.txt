add_executable(zapfeedreader-server)

# poco
if (EXISTS ${CMAKE_SOURCE_DIR}/3rdParty/poco-static/)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/3rdParty/poco/cmake)
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/3rdParty/poco-static/lib/cmake/Poco)
    find_package(PostgreSQL REQUIRED)
    find_package(Poco REQUIRED Foundation Util XML Net NetSSL JSON Data DataSQLite) 
    target_link_libraries(zapfeedreader-server  PRIVATE Poco::Foundation
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
    target_link_libraries(zapfeedreader-server PRIVATE fmt::fmt)
endif()

target_include_directories(zapfeedreader-server PRIVATE include)
target_include_directories(zapfeedreader-server PRIVATE ../engine/include)

target_link_libraries(zapfeedreader-server PRIVATE zapfeedreader-engine)

add_subdirectory(src)

add_custom_target(generate-server-api COMMAND ${CMAKE_SOURCE_DIR}/server/helpers/api-generator.py -c "${CMAKE_SOURCE_DIR}/server/helpers/api.conf" -r ${CMAKE_SOURCE_DIR})
