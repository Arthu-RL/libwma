# Public headers ship both .hpp and .h (Keys.h is a .h) — glob both.
file(GLOB_RECURSE WMA_HEADERS
    CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/include/*.hpp
    ${PROJECT_SOURCE_DIR}/include/*.h
)

file(GLOB_RECURSE WMA_SOURCES
    CONFIGURE_DEPENDS
    *.cpp
)

# Remove source files that belong to disabled backends so their
# implementation-specific headers and macros are never required.
#
# Both trees are swept: backends/<name>/ holds windowing backends and
# audio/backends/<name>/ holds audio ones. They are separate directories rather
# than one because the two are independent axes -- SDL3 appears in both and is
# gated by the single WMA_ENABLE_SDL (one dependency, two subsystems), while
# ALSA is audio-only and GLFW/X11/Wayland are windowing-only. audio/backends/null
# is deliberately not in this list: it has no dependencies and is always built.
foreach(_backend SDL GLFW X11 WAYLAND ALSA)
    if(NOT WMA_ENABLE_${_backend})
        string(TOLOWER "${_backend}" _lower)
        file(GLOB_RECURSE _disabled CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/backends/${_lower}/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/audio/backends/${_lower}/*.cpp"
        )
        if(_disabled)
            list(REMOVE_ITEM WMA_SOURCES ${_disabled})
        endif()
    endif()
endforeach()

# Metal support: whether this build can honour GraphicsAPI::Metal at all. Apple
# platforms only, and only through a backend that has a route to a CAMetalLayer
# (SDL3 anywhere, GLFW on macOS). Exposed to consumers as WMA_HAS_METAL in
# BuildConfig.hpp, so a renderer can feature-detect instead of duplicating this
# reasoning against __APPLE__ and the backend list.
set(WMA_HAS_METAL FALSE)
if(APPLE AND (WMA_ENABLE_SDL OR WMA_ENABLE_GLFW))
    set(WMA_HAS_METAL TRUE)

    # The AppKit half of the GLFW route -- the library's only Objective-C++, and
    # the reason the root CMakeLists calls enable_language(OBJCXX) for APPLE.
    # Compiled unconditionally on Apple rather than only for WMA_ENABLE_GLFW: the
    # file is a handful of lines, and its non-macOS branch is already a stub, so
    # keeping it out of the GLFW conditional means one fewer way for an
    # SDL-only-plus-GLFW-later build to fail to link.
    list(APPEND WMA_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/platform/apple/AppleMetalLayer.mm")
endif()

set(WMA_GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
set(WMA_HAS_GLFW    ${WMA_ENABLE_GLFW})
set(WMA_HAS_SDL     ${WMA_ENABLE_SDL})
set(WMA_HAS_X11     ${WMA_ENABLE_X11})
set(WMA_HAS_WAYLAND ${WMA_ENABLE_WAYLAND})
set(WMA_HAS_ALSA    ${WMA_ENABLE_ALSA})
# WMA_HAS_METAL was resolved above; unlike the four backend flags it is not a
# straight copy of an option, since it depends on the target platform too.
configure_file(
    "${PROJECT_SOURCE_DIR}/include/wma/core/BuildConfig.hpp.in"
    "${WMA_GENERATED_DIR}/wma/core/BuildConfig.hpp"
    @ONLY
)

add_library(wma STATIC)
add_library(wma::wma ALIAS wma)

target_sources(wma
    PRIVATE
        ${WMA_SOURCES}

    PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS
            ${PROJECT_SOURCE_DIR}/include
        FILES
            ${WMA_HEADERS}

        FILE_SET generated_headers
        TYPE HEADERS
        BASE_DIRS
            ${WMA_GENERATED_DIR}
        FILES
            "${WMA_GENERATED_DIR}/wma/core/BuildConfig.hpp"
)

target_include_directories(wma
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${WMA_GENERATED_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(wma
    PUBLIC
        cxx_std_23
)

target_compile_options(wma PRIVATE
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
        $<$<CONFIG:Debug>:
            -Wall
            -Wextra
            -g
            -O0
        >

        $<$<CONFIG:Release>:-O3>

        $<$<AND:$<CONFIG:Release>,$<BOOL:${WMA_NATIVE_OPTIMIZE}>>:
            -march=native
            -mtune=native
        >
    >

    $<$<CXX_COMPILER_ID:MSVC>:
        /W4
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Release>:/O2>
    >
)

# MSVC has no -march=native/-mtune=native equivalent, so WMA_NATIVE_OPTIMIZE
# is a no-op there — flag it instead of silently ignoring the request.
if(WMA_NATIVE_OPTIMIZE AND MSVC)
    message(STATUS "[wma] WMA_NATIVE_OPTIMIZE has no effect on MSVC (no -march=native equivalent); ignoring.")
endif()

# ThreadPool/WorkerThread live in ink::ink again as of ink 0.3.0, so there is
# no separate threading target to link -- on Emscripten or anywhere else.
target_link_libraries(wma PUBLIC ink::ink)

if(WMA_ENABLE_SDL)
    target_compile_definitions(wma PRIVATE WMA_ENABLE_SDL)
    target_link_libraries(wma PRIVATE SDL3::SDL3)
endif()

if(WMA_HAS_METAL)
    # QuartzCore for CAMetalLayer, Foundation for the Objective-C runtime types
    # AppleMetalLayer.mm touches. Metal itself is deliberately NOT linked: wma
    # creates the layer but never a MTLDevice — the renderer sets that, and it is
    # the renderer that links Metal.
    #
    # PUBLIC because wma is a static library: these symbols have to resolve at the
    # final link of whatever executable consumes it, not here.
    target_link_libraries(wma PUBLIC
        "-framework QuartzCore"
        "-framework Foundation")

    # AppKit hosts the layer in an NSView on macOS. iOS reaches its layer through
    # SDL's UIView-backed metal view instead, so nothing there needs UIKit from
    # this library.
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        target_link_libraries(wma PUBLIC "-framework AppKit")
    endif()
endif()

if(WMA_ENABLE_GLFW)
    target_compile_definitions(wma PRIVATE WMA_ENABLE_GLFW)
    target_link_libraries(wma PRIVATE glfw)
endif()

if(WMA_ENABLE_ALSA)
    target_compile_definitions(wma PRIVATE WMA_ENABLE_ALSA)
    target_link_libraries(wma PRIVATE PkgConfig::ALSA)
endif()

if(WMA_ENABLE_X11)
    target_compile_definitions(wma PRIVATE WMA_ENABLE_X11)
    target_link_libraries(wma PRIVATE X11::X11)

    # OpenGL on X11 is done through GLX. Optional: if libGL/GLX is unavailable the
    # backend still builds and simply reports OpenGL as unsupported at runtime.
    if(WMA_ENABLE_X11_GL)
        target_link_libraries(wma PRIVATE OpenGL::GLX)
        target_compile_definitions(wma PRIVATE WMA_X11_HAS_GL)
    endif()
endif()

if(WMA_ENABLE_WAYLAND)
    target_compile_definitions(wma PRIVATE WMA_ENABLE_WAYLAND)
    target_link_libraries(wma PRIVATE PkgConfig::WAYLAND)

    # Generated protocol marshalling code (defines xdg_wm_base_interface & co.,
    # which the client headers only declare) — see cmake/Dependencies.cmake.
    target_sources(wma PRIVATE ${WMA_WAYLAND_GENERATED_SOURCES})

    # OpenGL on Wayland is done through EGL + wayland-egl (optional, as above).
    if(WMA_ENABLE_WAYLAND_GL)
        target_link_libraries(wma PRIVATE PkgConfig::WAYLAND_EGL)
        target_compile_definitions(wma PRIVATE WMA_WAYLAND_HAS_GL)
    endif()

    # Expose the build-time generated protocol headers to the compiler and
    # install them alongside the other public headers.
    target_include_directories(wma PUBLIC
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    )
    install(
        DIRECTORY "${WMA_WAYLAND_PROTO_DIR}/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/wma/backends/wayland/protocols"
        FILES_MATCHING PATTERN "*.h"
    )
endif()
