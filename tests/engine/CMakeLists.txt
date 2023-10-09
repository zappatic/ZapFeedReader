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

add_executable(tests-engine)

set_target_properties(tests-engine PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")

target_include_directories(tests-engine PRIVATE include)
target_include_directories(tests-engine PRIVATE ../../engine/include)
target_include_directories(tests-engine SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/3rdParty/fmtlib/include)

target_link_libraries(tests-engine PRIVATE Catch2::Catch2WithMain
                                    zapfeedreader-engine
                                    Poco::Foundation)

if(ENABLE_COVERAGE)
    target_compile_options(tests-engine PRIVATE -O0 --coverage)
    target_link_libraries(tests-engine PRIVATE gcov)
endif()

add_subdirectory(src)

add_custom_target(tests-engine-symlink-input COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/tests/engine/input "${CMAKE_BINARY_DIR}/$<CONFIG>/tests-engine-input")
add_dependencies(tests-engine tests-engine-symlink-input)