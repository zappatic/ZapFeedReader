add_executable(zapfeedreader-server)

target_include_directories(zapfeedreader-server PRIVATE include)
target_include_directories(zapfeedreader-server PRIVATE ../engine/include)
target_include_directories(zapfeedreader-server SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/3rdParty/fmtlib/include)

target_link_libraries(zapfeedreader-server PRIVATE  zapfeedreader-engine 
                                                    Poco::Foundation
                                                    Poco::Util
                                                    Poco::XML
                                                    Poco::Net
                                                    Poco::NetSSL
                                                    Poco::JSON
                                                    Poco::Data
                                                    Poco::DataSQLite)

add_subdirectory(src)

add_custom_target(generate-server-api COMMAND ${CMAKE_SOURCE_DIR}/server/helpers/api-generator.py -c "${CMAKE_SOURCE_DIR}/server/helpers/api.conf" -r ${CMAKE_SOURCE_DIR})
