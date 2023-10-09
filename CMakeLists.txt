cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0076 NEW)

project(ZapFeedReader)

find_package(PkgConfig REQUIRED)
find_package(Poco REQUIRED Foundation Util XML Net NetSSL JSON Data DataSQLite) 
pkg_check_modules(lua REQUIRED IMPORTED_TARGET lua)

add_subdirectory(server)
add_subdirectory(engine)
add_subdirectory(client-desktop)
add_subdirectory(tests/engine)

if(ENABLE_COVERAGE)
  find_program(LCOV lcov REQUIRED)
  find_program(GENHTML genhtml REQUIRED)

  add_custom_target(coverage COMMAND ${LCOV} --directory . --capture --output-file coverage.info 
                             COMMAND ${LCOV} --remove coverage.info --output-file coverage_filtered.info 
                                                '/usr/include/*' 
                                                '/usr/local/include/*'
                                                '/*/ZapFeedReader/tests/*'
                                                '/*/ZapFeedReader/3rdParty/*'
                                                '/*/ZapFeedReader/build/*'
                             COMMAND ${GENHTML} --demangle-cpp -o coverage coverage_filtered.info
                             WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()