add_library(zapfeedreader-engine STATIC)

target_include_directories(zapfeedreader-engine PRIVATE include)

add_subdirectory(src)

# poco
if (EXISTS ${CMAKE_SOURCE_DIR}/3rdParty/poco-static/)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/3rdParty/poco/cmake)
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/3rdParty/poco-static/lib/cmake/Poco)
    find_package(PostgreSQL REQUIRED)
    find_package(Poco REQUIRED Foundation Util XML Net NetSSL JSON Data DataSQLite) 
    target_link_libraries(zapfeedreader-engine PRIVATE  Poco::Foundation
                                                        Poco::Util
                                                        Poco::XML
                                                        Poco::Net
                                                        Poco::NetSSL
                                                        Poco::JSON
                                                        Poco::Data
                                                        Poco::DataSQLite)

endif()

# fmtlib
if (EXISTS ${CMAKE_SOURCE_DIR}/3rdParty/fmtlib-install/)
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/3rdParty/fmtlib-install/lib/cmake/fmt)
    find_package(fmt)
    target_link_libraries(zapfeedreader-engine PRIVATE fmt::fmt)
endif()

# lua
if (EXISTS ${CMAKE_SOURCE_DIR}/3rdParty/lua-install/)
    set(ENV{PKG_CONFIG_PATH} "${CMAKE_SOURCE_DIR}/3rdParty/lua-install/lib/pkgconfig")
    pkg_check_modules(lua REQUIRED IMPORTED_TARGET lua)
   target_link_libraries(zapfeedreader-engine PRIVATE PkgConfig::lua)
endif()
