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
