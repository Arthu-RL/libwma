cmake_minimum_required(VERSION 4.3.3)

project(wma
VERSION 0.1.0
LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

option(WMA_BUILD_TESTS      "Build test suite"                OFF)
option(WMA_BUILD_EXAMPLES   "Build example applications"      OFF)
option(WMA_ENABLE_LTO       "Enable link-time optimization"   ON)
# -march=native produces non-portable binaries; keep OFF by default for a library
# that may be installed and shipped. Enable it for machine-local builds.
option(WMA_NATIVE_OPTIMIZE  "Enable native CPU optimization"  OFF)

include(cmake/Platform.cmake)
include(cmake/Compiler.cmake)
include(cmake/Dependencies.cmake)

add_subdirectory(src)

if(WMA_BUILD_TESTS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    else()
        message(WARNING "[wma] WMA_BUILD_TESTS=ON but tests/ directory is absent — skipping.")
    endif()
endif()

if(WMA_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

include(cmake/Install.cmake)
