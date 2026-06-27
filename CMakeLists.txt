cmake_minimum_required(VERSION 4.3.3)

project(wma
VERSION 1.0.0
LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

option(WMA_BUILD_TESTS      "Build test suite"                ON)
option(WMA_ENABLE_LTO       "Enable link-time optimization"   ON)
option(WMA_NATIVE_OPTIMIZE  "Enable native CPU optimization"  ON)

include(cmake/Platform.cmake)
include(cmake/Compiler.cmake)
include(cmake/Dependencies.cmake)

add_subdirectory(src)

if(WMA_BUILD_TESTS)
    add_subdirectory(tests)
endif()

include(cmake/Install.cmake)