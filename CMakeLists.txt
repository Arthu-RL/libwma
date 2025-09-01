cmake_minimum_required(VERSION 3.16)

# Project definition
project(wma
    VERSION 1.0.0
    DESCRIPTION "Cross-platform Window Management & Input Abstraction Library"
    LANGUAGES CXX
)

# C++17 requirement
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(WMA_BUILD_SHARED "Build shared library" OFF)
option(WMA_ENABLE_GLFW "Enable GLFW backend" OFF)
option(WMA_ENABLE_SDL "Enable SDL2 backend" ON)
option(WMA_BUILD_EXAMPLES "Build example applications" ON)
option(WMA_AUTO_INSTALL "Auto Install WMA lib" ON)
option(WMA_BUILD_TESTS "Build unit tests" OFF)
option(WMA_ENABLE_VULKAN "Enable Vulkan support" OFF)
option(WMA_ENABLE_OPENGL "Enable OpenGL support" OFF)

# Validate options
if(NOT WMA_ENABLE_GLFW AND NOT WMA_ENABLE_SDL)
    message(FATAL_ERROR "At least one backend (GLFW or SDL) must be enabled")
endif()

if(WMA_BUILD_SHARED)
    set(WMA_LIBRARY_TYPE SHARED)
    message(STATUS "🔧 Building SHARED library")
else()
    set(WMA_LIBRARY_TYPE STATIC)
    message(STATUS "🔧 Building STATIC library")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
endif()

file(GLOB_RECURSE HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/core/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/input/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/input/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/exceptions/*.hpp
)

file(GLOB_RECURSE SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/exceptions/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/input/*.cpp
)

set(WMA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/wma.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/managers/IWindowManager.hpp
    ${HEADERS}
    ${SOURCES}
)

# Backend-specific sources
if(WMA_ENABLE_GLFW)
    list(APPEND WMA_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/managers/GlfwWindowManager.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/managers/GlfwWindowManager.cpp
    )
endif()

if(WMA_ENABLE_SDL)
    list(APPEND WMA_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/managers/SdlWindowManager.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/managers/SdlWindowManager.cpp
    )
endif()

# Create the main library
add_library(${PROJECT_NAME} ${WMA_LIBRARY_TYPE} ${WMA_SOURCES})

# VERIFY what library type was actually created
get_target_property(ACTUAL_LIB_TYPE ${PROJECT_NAME} TYPE)
message(STATUS "DEBUG: Actual library type created = ${ACTUAL_LIB_TYPE}")

# Include directories
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compile definitions
if(WMA_ENABLE_GLFW)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WMA_ENABLE_GLFW)
endif()

if(WMA_ENABLE_SDL)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WMA_ENABLE_SDL)
endif()

if(WMA_ENABLE_VULKAN)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WMA_ENABLE_VULKAN)
endif()

if(WMA_ENABLE_OPENGL)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WMA_ENABLE_OPENGL)
endif()

# Find and link dependencies
set(ALL_LIBRARY_PATH "$ENV{LIBRARY_PATH}/lib")

# External libs - INK dependency
find_package(ink HINTS ${ALL_LIBRARY_PATH})
target_link_libraries(${PROJECT_NAME} PUBLIC ink)

# GLFW
if(WMA_ENABLE_GLFW)
    find_package(glfw3 HINTS ${ALL_LIBRARY_PATH})
    target_link_libraries(${PROJECT_NAME} PUBLIC glfw3)
endif()

# SDL2
if(WMA_ENABLE_SDL)
    find_package(SDL2 HINTS ${ALL_LIBRARY_PATH})
    target_link_libraries(${PROJECT_NAME} PUBLIC SDL2)
endif()

# Find and link dependencies
set(ALL_LIBRARY_PATH "$ENV{LIBRARY_PATH}/lib")

# OpenGL/GLAD
if(WMA_ENABLE_OPENGL)
    find_package(glad HINTS ${ALL_LIBRARY_PATH})
    target_link_libraries(${PROJECT_NAME} PUBLIC glad)
endif()

# Platform-specific linking
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE user32 gdi32 winmm)
    if(WMA_ENABLE_OPENGL)
        target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)
    endif()
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
    if(WMA_ENABLE_OPENGL)
        target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL")
    endif()
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread dl)
    if(WMA_ENABLE_OPENGL)
        target_link_libraries(${PROJECT_NAME} PRIVATE GL)
    endif()
endif()

# Configuration-specific flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -g -O0)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        target_compile_options(${PROJECT_NAME} PRIVATE
            /W4
            /wd4100  # unreferenced formal parameter
            /wd4201  # nameless struct/union
            /wd4267  # conversion from size_t
        )
    endif()
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -O3 -DNDEBUG)
endif()

# Set library properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME ${PROJECT_NAME}
)

# Examples
if(WMA_BUILD_EXAMPLES)
    message(STATUS "Building examples...")

    # Basic window example
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/basic_window/main.cpp")
        add_executable(basic_window examples/basic_window/main.cpp)
        target_link_libraries(basic_window PRIVATE ${PROJECT_NAME})
        set_target_properties(basic_window PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
        message(STATUS "  - Basic window example: Enabled")
    endif()

    # Input handling example (keyboard + mouse)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/input_handling/main.cpp")
        add_executable(input_handling examples/input_handling/main.cpp)
        target_link_libraries(input_handling PRIVATE ${PROJECT_NAME})
        set_target_properties(input_handling PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
        message(STATUS "  - Input handling example: Enabled")
    endif()

    # Create examples target
    add_custom_target(wma_examples)
    if(TARGET basic_window)
        add_dependencies(wma_examples basic_window)
    endif()
    if(TARGET input_handling)
        add_dependencies(wma_examples input_handling)
    endif()
endif()

# Testing support
if(WMA_BUILD_TESTS)
    enable_testing()
    find_package(GTest QUIET)
    if(GTest_FOUND)
        add_subdirectory(tests)
        message(STATUS "Unit tests: Enabled (using Google Test)")
    else()
        message(STATUS "Unit tests: Disabled (Google Test not found)")
    endif()
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.h"
)

if(WMA_BUILD_EXAMPLES)
    if(TARGET wma_basic_window)
        install(TARGETS wma_basic_window DESTINATION ${CMAKE_INSTALL_BINDIR}/examples)
    endif()
    if(TARGET wma_input_handling)
        install(TARGETS wma_input_handling DESTINATION ${CMAKE_INSTALL_BINDIR}/examples)
    endif()
endif()

install(EXPORT ${PROJECT_NAME}-targets
    FILE ${PROJECT_NAME}-config.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    OPTIONAL
)

if(WMA_AUTO_INSTALL)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} --install .
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Auto-installing ${PROJECT_NAME} library..."
    )
endif()

# Build summary
message(STATUS "")
message(STATUS "=== ${PROJECT_NAME} (Window Management & Input Abstraction) Build Configuration ===")
message(STATUS "Version:         ${PROJECT_VERSION}")
message(STATUS "Library Type:    ${WMA_LIBRARY_TYPE}")
message(STATUS "C++ Standard:    ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type:      ${CMAKE_BUILD_TYPE}")
message(STATUS "")
message(STATUS "Backends:")
if(WMA_ENABLE_GLFW)
    message(STATUS "  ✓ GLFW")
endif()
if(WMA_ENABLE_SDL)
    message(STATUS "  ✓ SDL2")
endif()
message(STATUS "")
message(STATUS "Features:")
message(STATUS "  Examples:      ${WMA_BUILD_EXAMPLES}")
message(STATUS "  Tests:         ${WMA_BUILD_TESTS}")
message(STATUS "  Auto-install:  ${WMA_AUTO_INSTALL}")
message(STATUS "")
message(STATUS "Graphics APIs:")
if(WMA_ENABLE_VULKAN)
    message(STATUS "  ✓ Vulkan")
endif()
if(WMA_ENABLE_OPENGL)
    message(STATUS "  ✓ OpenGL")
endif()
if(WMA_ENABLE_SOFTWARE)
    message(STATUS "  ✓ Software/CPU")
endif()
message(STATUS "")
message(STATUS "Install prefix:  ${CMAKE_INSTALL_PREFIX}")
message(STATUS "=====================================")
message(STATUS "")
