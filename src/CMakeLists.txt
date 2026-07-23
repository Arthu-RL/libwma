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
foreach(_backend SDL GLFW X11 WAYLAND)
    if(NOT WMA_ENABLE_${_backend})
        string(TOLOWER "${_backend}" _lower)
        file(GLOB_RECURSE _disabled CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/backends/${_lower}/*.cpp"
        )
        if(_disabled)
            list(REMOVE_ITEM WMA_SOURCES ${_disabled})
        endif()
    endif()
endforeach()

set(WMA_GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
set(WMA_HAS_GLFW    ${WMA_ENABLE_GLFW})
set(WMA_HAS_SDL     ${WMA_ENABLE_SDL})
set(WMA_HAS_X11     ${WMA_ENABLE_X11})
set(WMA_HAS_WAYLAND ${WMA_ENABLE_WAYLAND})
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

target_link_libraries(wma PUBLIC ink::ink)

if(WMA_ENABLE_SDL)
    target_compile_definitions(wma PRIVATE WMA_ENABLE_SDL)
    target_link_libraries(wma PRIVATE SDL3::SDL3)
endif()

if(WMA_ENABLE_GLFW)
    target_compile_definitions(wma PRIVATE WMA_ENABLE_GLFW)
    target_link_libraries(wma PRIVATE glfw)
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
