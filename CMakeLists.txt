cmake_minimum_required(VERSION 4.2.3)

project(wma
    VERSION 1.0.0
    DESCRIPTION "Cross-platform Window Management & Input Abstraction Library"
    LANGUAGES C CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# =================================================================
# Options
# =================================================================
option(BUILD_SHARED_LIBS  "Build shared library"                  OFF)
option(WMA_ENABLE_GLFW    "Enable GLFW backend"                   OFF)
option(WMA_ENABLE_SDL     "Enable SDL2 backend"                   ON)
option(WMA_ENABLE_X11     "Enable X11 backend"                    ON)
option(WMA_ENABLE_WAYLAND "Enable Wayland backend"                ON)
option(WMA_BUILD_EXAMPLES "Build example applications"            ON)
option(WMA_AUTO_INSTALL   "Auto-install after build"              ON)
option(WMA_BUILD_TESTS    "Build unit tests"                      OFF)

if(NOT (WMA_ENABLE_GLFW OR WMA_ENABLE_SDL OR WMA_ENABLE_X11 OR WMA_ENABLE_WAYLAND))
    message(FATAL_ERROR "At least one backend must be enabled")
endif()

# =================================================================
# Target
# =================================================================
add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME ${PROJECT_NAME}
)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -Wall -Wextra -Wpedantic -Wno-unused-parameter>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# =================================================================
# Include paths
# =================================================================
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# =================================================================
# Core sources and public headers
# =================================================================
target_sources(${PROJECT_NAME}
    PRIVATE
        src/exceptions/WMAException.cpp
        src/input/keyboard/KeyboardListener.cpp
        src/input/mouse/MouseListener.cpp

    PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        FILES
            include/wma/wma.hpp
            include/wma/managers/IWindowManager.hpp
            include/wma/core/Types.hpp
            include/wma/core/WindowDetails.hpp
            include/wma/core/WindowFlags.hpp
            include/wma/core/FrameTimer.hpp
            include/wma/exceptions/WMAException.hpp
            include/wma/input/keyboard/KeyboardListener.hpp
            include/wma/input/keyboard/KeyAction.hpp
            include/wma/input/keyboard/Keys.h
            include/wma/input/mouse/MouseListener.hpp
            include/wma/input/mouse/MouseAction.hpp
)

# =================================================================
# Core dependency
# =================================================================
find_package(ink REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC ink)

# =================================================================
# GLFW backend
# =================================================================
if(WMA_ENABLE_GLFW)
    find_package(glfw3 REQUIRED)

    target_compile_definitions(${PROJECT_NAME} PUBLIC WMA_ENABLE_GLFW)
    target_link_libraries(${PROJECT_NAME} PUBLIC glfw3)

    target_sources(${PROJECT_NAME}
        PRIVATE
            src/backends/glfw/GlfwWindowManager.cpp
            src/backends/glfw/GLFWKeyboardListener.cpp
            src/backends/glfw/GLFWMouseListener.cpp

        PUBLIC
            FILE_SET public_headers
            TYPE HEADERS
            BASE_DIRS
                ${CMAKE_CURRENT_SOURCE_DIR}/include
            FILES
                include/wma/backends/glfw/GlfwWindowManager.hpp
                include/wma/backends/glfw/GLFWKeyboardListener.hpp
                include/wma/backends/glfw/GLFWMouseListener.hpp
    )
endif()

# =================================================================
# SDL2 backend
# =================================================================
if(WMA_ENABLE_SDL)
    find_package(SDL2 REQUIRED)

    target_compile_definitions(${PROJECT_NAME} PUBLIC WMA_ENABLE_SDL)
    target_link_libraries(${PROJECT_NAME} PUBLIC SDL2)

    target_sources(${PROJECT_NAME}
        PRIVATE
            src/backends/sdl/SdlWindowManager.cpp
            src/backends/sdl/SDLKeyboardListener.cpp
            src/backends/sdl/SDLMouseListener.cpp

        PUBLIC
            FILE_SET public_headers
            TYPE HEADERS
            BASE_DIRS
                ${CMAKE_CURRENT_SOURCE_DIR}/include
            FILES
                include/wma/backends/sdl/SdlWindowManager.hpp
                include/wma/backends/sdl/SDLKeyboardListener.hpp
                include/wma/backends/sdl/SDLMouseListener.hpp
    )
endif()

# =================================================================
# X11 backend
# =================================================================
if(WMA_ENABLE_X11)
    find_package(X11 REQUIRED)

    target_compile_definitions(${PROJECT_NAME} PUBLIC WMA_ENABLE_X11)
    target_link_libraries(${PROJECT_NAME} PUBLIC X11::X11)

    target_sources(${PROJECT_NAME}
        PRIVATE
            src/backends/x11/X11WindowManager.cpp
            src/backends/x11/X11KeyboardListener.cpp
            src/backends/x11/X11MouseListener.cpp

        PUBLIC
            FILE_SET public_headers
            TYPE HEADERS
            BASE_DIRS
                ${CMAKE_CURRENT_SOURCE_DIR}/include
            FILES
                include/wma/backends/x11/X11WindowManager.hpp
                include/wma/backends/x11/X11KeyboardListener.hpp
                include/wma/backends/x11/X11MouseListener.hpp
    )
endif()

# =================================================================
# Wayland backend
# =================================================================
if(WMA_ENABLE_WAYLAND)
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(WAYLAND REQUIRED IMPORTED_TARGET
        wayland-client
        wayland-cursor
        xkbcommon
    )

    pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)

    find_program(WAYLAND_SCANNER_EXE wayland-scanner REQUIRED)

    pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)

    set(XDG_SHELL_XML
        "${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml"
    )

    set(GENERATED_ROOT_DIR
        "${CMAKE_CURRENT_BINARY_DIR}/generated"
    )

    set(GENERATED_INCLUDE_DIR
        "${GENERATED_ROOT_DIR}/include"
    )

    set(GENERATED_PROTOCOL_DIR
        "${GENERATED_INCLUDE_DIR}/wma/backends/wayland/protocols"
    )

    set(XDG_SHELL_HEADER
        "${GENERATED_PROTOCOL_DIR}/xdg-shell-client-protocol.h"
    )

    set(XDG_SHELL_CODE
        "${GENERATED_ROOT_DIR}/xdg-shell-protocol.c"
    )

    add_custom_command(
        OUTPUT
            ${XDG_SHELL_HEADER}
        COMMAND
            ${CMAKE_COMMAND} -E make_directory ${GENERATED_PROTOCOL_DIR}
        COMMAND
            ${WAYLAND_SCANNER_EXE}
            client-header
            ${XDG_SHELL_XML}
            ${XDG_SHELL_HEADER}
        DEPENDS
            ${XDG_SHELL_XML}
        COMMENT
            "Generating Wayland xdg-shell client protocol header"
        VERBATIM
    )

    add_custom_command(
        OUTPUT
            ${XDG_SHELL_CODE}
        COMMAND
            ${CMAKE_COMMAND} -E make_directory ${GENERATED_ROOT_DIR}
        COMMAND
            ${WAYLAND_SCANNER_EXE}
            private-code
            ${XDG_SHELL_XML}
            ${XDG_SHELL_CODE}
        DEPENDS
            ${XDG_SHELL_XML}
            ${XDG_SHELL_HEADER}
        COMMENT
            "Generating Wayland xdg-shell private protocol source"
        VERBATIM
    )

    target_compile_definitions(${PROJECT_NAME} PUBLIC WMA_ENABLE_WAYLAND)

    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            PkgConfig::WAYLAND
    )

    target_include_directories(${PROJECT_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${GENERATED_INCLUDE_DIR}>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/wma/backends/wayland/protocols>
    )

    target_sources(${PROJECT_NAME}
        PRIVATE
            src/backends/wayland/WaylandWindowManager.cpp
            src/backends/wayland/WaylandKeyboardListener.cpp
            src/backends/wayland/WaylandMouseListener.cpp
            ${XDG_SHELL_CODE}

        PUBLIC
            FILE_SET public_headers
            TYPE HEADERS
            BASE_DIRS
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${GENERATED_INCLUDE_DIR}
            FILES
                include/wma/backends/wayland/WaylandWindowManager.hpp
                include/wma/backends/wayland/WaylandKeyboardListener.hpp
                include/wma/backends/wayland/WaylandMouseListener.hpp
                ${XDG_SHELL_HEADER}
    )
endif()

# =================================================================
# Examples
# =================================================================
if(WMA_BUILD_EXAMPLES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/basic_window/main.cpp")
        add_executable(basic_window examples/basic_window/main.cpp)

        target_link_libraries(basic_window
            PRIVATE
                ${PROJECT_NAME}::${PROJECT_NAME}
        )

        set_target_properties(basic_window PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/input_handling/main.cpp")
        add_executable(input_handling examples/input_handling/main.cpp)

        target_link_libraries(input_handling
            PRIVATE
                ${PROJECT_NAME}::${PROJECT_NAME}
        )

        set_target_properties(input_handling PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
    endif()
endif()

# =================================================================
# Tests
# =================================================================
if(WMA_BUILD_TESTS)
    enable_testing()

    find_package(GTest QUIET)

    if(GTest_FOUND)
        add_subdirectory(tests)
    endif()
endif()

# =================================================================
# Install target, headers, library
# =================================================================
install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets

    FILE_SET public_headers
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}

    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# =================================================================
# Install CMake target export
# =================================================================
install(
    EXPORT ${PROJECT_NAME}-targets
    FILE ${PROJECT_NAME}-targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# =================================================================
# Package config
# =================================================================
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# =================================================================
# Optional auto install
# =================================================================
if(WMA_AUTO_INSTALL)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Auto-installing ${PROJECT_NAME} to ${CMAKE_INSTALL_PREFIX}"
    )
endif()