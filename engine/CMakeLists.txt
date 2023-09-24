add_library(zapfeedreader-engine SHARED)

target_include_directories(zapfeedreader-engine PRIVATE include)
target_include_directories(zapfeedreader-engine SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/3rdParty/fmtlib/include)

target_link_libraries(zapfeedreader-engine PRIVATE  PkgConfig::lua
                                                    Poco::Foundation
                                                    Poco::Util
                                                    Poco::XML
                                                    Poco::Net
                                                    Poco::NetSSL
                                                    Poco::JSON
                                                    Poco::Data
                                                    Poco::DataSQLite)

add_subdirectory(src)

add_custom_target(generate-lua-api COMMAND ${CMAKE_SOURCE_DIR}/engine/src/lua/api-generator.py  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/engine/src/lua/)
add_dependencies(zapfeedreader-engine generate-lua-api)