add_executable(zapfeedreader-server)

set_target_properties(zapfeedreader-server PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")

target_compile_options(zapfeedreader-server PRIVATE -Wall -Wextra -pedantic -Werror -Wconversion -Weffc++ -Wshadow -Wnon-virtual-dtor -Wcast-align -Wunused -Woverloaded-virtual -Wno-uninitialized)

target_include_directories(zapfeedreader-server PRIVATE include)
target_include_directories(zapfeedreader-server PRIVATE ../engine/include)
target_include_directories(zapfeedreader-server SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/3rdParty/fmtlib/include)

target_link_libraries(zapfeedreader-server PRIVATE  zapfeedreader-engine 
                                                    PkgConfig::lua
                                                    Poco::Foundation
                                                    Poco::Util
                                                    Poco::XML
                                                    Poco::Net
                                                    Poco::NetSSL
                                                    Poco::JSON
                                                    Poco::Data
                                                    Poco::DataSQLite)

# If the debug configuration pass the DEBUG define to the compiler 
target_compile_definitions(zapfeedreader-server PRIVATE $<$<CONFIG:Debug>:DEBUG>)

include(src/CMakeLists.txt)
list(TRANSFORM ZAPFR_SERVER_SOURCES PREPEND "src/")
target_sources(zapfeedreader-server PRIVATE ${ZAPFR_SERVER_SOURCES})
target_sources(zapfeedreader-server PRIVATE src/main.cpp)

add_custom_target(generate-server-api COMMAND ${CMAKE_SOURCE_DIR}/server/helpers/api-generator.py -c "${CMAKE_SOURCE_DIR}/server/helpers/api.conf" -r ${CMAKE_SOURCE_DIR})
