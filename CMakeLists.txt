cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0076 NEW)

string(TOLOWER "${CMAKE_BUILD_TYPE}" lcbt)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${lcbt}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${lcbt}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${lcbt}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${lcbt}")

add_compile_options(-Wall -Wextra -pedantic -Werror -Wconversion -Weffc++ -Wshadow -Wnon-virtual-dtor -Wcast-align -Wunused -Woverloaded-virtual -Wno-uninitialized)

project(ZapFeedReader)

find_package(PkgConfig REQUIRED)
find_package(Poco REQUIRED Foundation Util XML Net NetSSL JSON Data DataSQLite) 
pkg_check_modules(lua REQUIRED IMPORTED_TARGET lua)

add_subdirectory(server)
add_subdirectory(engine)
add_subdirectory(client-desktop)

