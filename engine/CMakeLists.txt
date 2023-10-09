add_library(zapfeedreader-engine SHARED)

set_target_properties(zapfeedreader-engine PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")

target_compile_options(zapfeedreader-engine PRIVATE -Wall -Wextra -pedantic -Werror -Wconversion -Weffc++ -Wshadow -Wnon-virtual-dtor -Wcast-align -Wunused -Woverloaded-virtual -Wno-uninitialized)

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

if(ENABLE_COVERAGE)
    target_compile_options(zapfeedreader-engine PRIVATE -O0 --coverage)
    target_link_libraries(zapfeedreader-engine PRIVATE gcov)
endif()

add_subdirectory(src)

add_custom_target(generate-lua-api COMMAND ${CMAKE_SOURCE_DIR}/engine/src/lua/api-generator.py  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/engine/src/lua/)
add_dependencies(zapfeedreader-engine generate-lua-api)