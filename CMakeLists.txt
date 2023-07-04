cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

string(TOLOWER "${CMAKE_BUILD_TYPE}" lcbt)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${lcbt}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${lcbt}")

add_compile_options(-Wall -Wextra -pedantic -Werror -Wconversion -Weffc++ -Wshadow -Wnon-virtual-dtor -Wcast-align -Wunused -Woverloaded-virtual -Wno-uninitialized)

project(ZapFeedReader)

add_subdirectory(server)

add_custom_target(build-poco COMMAND ${CMAKE_SOURCE_DIR}/3rdParty/build-poco.sh WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/3rdParty/poco/)
add_custom_target(build-fmtlib COMMAND ${CMAKE_SOURCE_DIR}/3rdParty/build-fmtlib.sh WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/3rdParty/fmtlib/)
