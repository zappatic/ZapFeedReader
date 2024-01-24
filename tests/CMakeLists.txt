cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0076 NEW)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests)

set_target_properties(tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")

target_include_directories(tests PRIVATE include)
target_include_directories(tests PRIVATE ../engine/include)
target_include_directories(tests PRIVATE ../server/include)
target_include_directories(tests SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/3rdParty/fmtlib/include)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain
                                    zapfeedreader-engine
                                    Poco::Foundation
                                    Poco::XML
                                    Poco::JSON
                                    Poco::Util
                                    Poco::NetSSL
                                    Poco::Data
                                    Poco::DataSQLite)

if(ENABLE_COVERAGE)
    target_compile_options(tests PRIVATE -O0 --coverage -fno-elide-constructors -fno-default-inline)
    target_link_libraries(tests PRIVATE gcov)
endif()

add_subdirectory(src)

include(../server/src/CMakeLists.txt)
list(TRANSFORM ZAPFR_SERVER_SOURCES PREPEND "../server/src/")
target_sources(tests PRIVATE ${ZAPFR_SERVER_SOURCES})


add_custom_target(tests-symlink-input COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/tests/input "${CMAKE_BINARY_DIR}/$<CONFIG>/tests-input")
add_dependencies(tests tests-symlink-input)