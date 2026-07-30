if(DEFINED ENV{LOCAL_PREFIX})
    set(_WMA_SEARCH_PREFIX "$ENV{LOCAL_PREFIX}")
else()
    set(_WMA_SEARCH_PREFIX "/usr/local")
endif()

if(ANDROID)
    set(_WMA_PLATFORM "android")
elseif(EMSCRIPTEN)
    set(_WMA_PLATFORM "wasm")
else()
    # libink's linux presets install to linux/debug or linux/release
    # (single-config Ninja generator, so CMAKE_BUILD_TYPE is known here).
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(_WMA_PLATFORM "linux/debug")
    else()
        set(_WMA_PLATFORM "linux/release")
    endif()
endif()

set(ink_DIR "${_WMA_SEARCH_PREFIX}/${_WMA_PLATFORM}/lib/cmake/ink")

unset(_WMA_SEARCH_PREFIX)
unset(_WMA_PLATFORM)

find_package(ink REQUIRED CONFIG)

# Backend options. Defaults are OFF so a build explicitly opts into what it needs;
# Platform.cmake forces SDL3 ON for Android/WASM.
option(WMA_ENABLE_SDL     "Enable SDL3 backend"    OFF)
option(WMA_ENABLE_GLFW    "Enable GLFW backend"    OFF)
option(WMA_ENABLE_X11     "Enable X11 backend"     OFF)
option(WMA_ENABLE_WAYLAND "Enable Wayland backend" OFF)

# The native desktop backends do not exist on Android/WASM; only SDL3 survives.
if(ANDROID OR EMSCRIPTEN)
    foreach(_backend GLFW X11 WAYLAND)
        if(WMA_ENABLE_${_backend})
            message(WARNING
                "[wma] WMA_ENABLE_${_backend} is not supported on "
                "${CMAKE_SYSTEM_NAME} — forcing OFF (use SDL3)"
            )
        endif()
        set(WMA_ENABLE_${_backend} OFF CACHE BOOL "" FORCE)
    endforeach()
endif()

if(NOT WMA_ENABLE_SDL AND NOT WMA_ENABLE_GLFW AND NOT WMA_ENABLE_X11 AND NOT WMA_ENABLE_WAYLAND)
    message(WARNING
        "[wma] No backend enabled — the library will build but createWindowManager() "
        "will always throw. Enable at least one of WMA_ENABLE_SDL/GLFW/X11/WAYLAND."
    )
endif()

if(WMA_ENABLE_SDL)
    find_package(SDL3 REQUIRED CONFIG COMPONENTS SDL3)
endif()

if(WMA_ENABLE_GLFW)
    find_package(glfw3 REQUIRED)
endif()

set(WMA_ENABLE_X11_GL     FALSE)
set(WMA_ENABLE_WAYLAND_GL FALSE)

if(WMA_ENABLE_X11)
    find_package(X11 REQUIRED)
    # Optional: GLX for OpenGL-on-X11. Provides the imported target OpenGL::GLX.
    find_package(OpenGL COMPONENTS OpenGL GLX)
    if(TARGET OpenGL::GLX)
        set(WMA_ENABLE_X11_GL TRUE)
    else()
        message(STATUS "[wma] GLX not found — OpenGL on the X11 backend is disabled.")
    endif()
endif()

if(WMA_ENABLE_WAYLAND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND REQUIRED IMPORTED_TARGET
        wayland-client
        wayland-cursor
        xkbcommon
    )

    # Optional: EGL + wayland-egl for OpenGL-on-Wayland.
    pkg_check_modules(WAYLAND_EGL IMPORTED_TARGET egl wayland-egl)
    if(TARGET PkgConfig::WAYLAND_EGL)
        set(WMA_ENABLE_WAYLAND_GL TRUE)
    else()
        message(STATUS "[wma] EGL/wayland-egl not found — OpenGL on the Wayland backend is disabled.")
    endif()

    find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)
    pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)

    set(WMA_WAYLAND_PROTO_DIR
        "${CMAKE_BINARY_DIR}/include/wma/backends/wayland/protocols"
        CACHE INTERNAL ""
    )
    file(MAKE_DIRECTORY "${WMA_WAYLAND_PROTO_DIR}")

    set(WMA_WAYLAND_GENERATED_SOURCES "" CACHE INTERNAL "")

    # Runs a wayland-scanner subcommand, aborting the configure on failure.
    function(wma_run_wayland_scanner mode xml_relpath out_name)
        execute_process(
            COMMAND "${WAYLAND_SCANNER}" "${mode}"
                "${WAYLAND_PROTOCOLS_DATADIR}/${xml_relpath}"
                "${WMA_WAYLAND_PROTO_DIR}/${out_name}"
            RESULT_VARIABLE _wl_scanner_result
        )
        if(_wl_scanner_result)
            message(FATAL_ERROR
                "[wma] wayland-scanner failed to generate ${out_name} "
                "(exit code: ${_wl_scanner_result})"
            )
        endif()
    endfunction()

    # Generates both the client-header (declarations, e.g. `xdg_wm_base_interface`)
    # and the private-code (the .c that actually *defines* those wl_interface
    # symbols) for <xml_relpath>, relative to the wayland-protocols pkgdatadir.
    # Without the private-code half, the Wayland backend fails to link on its own
    # — it previously only worked when built alongside SDL3, whose static lib
    # happens to vendor the same symbols.
    function(wma_generate_wayland_protocol xml_relpath header_name source_name)
        wma_run_wayland_scanner(client-header "${xml_relpath}" "${header_name}")
        wma_run_wayland_scanner(private-code  "${xml_relpath}" "${source_name}")
        set(WMA_WAYLAND_GENERATED_SOURCES
            ${WMA_WAYLAND_GENERATED_SOURCES} "${WMA_WAYLAND_PROTO_DIR}/${source_name}"
            CACHE INTERNAL ""
        )
    endfunction()

    wma_generate_wayland_protocol(
        stable/xdg-shell/xdg-shell.xml
        xdg-shell-client-protocol.h
        xdg-shell-protocol.c
    )
    wma_generate_wayland_protocol(
        unstable/xdg-decoration/xdg-decoration-unstable-v1.xml
        xdg-decoration-unstable-v1-client-protocol.h
        xdg-decoration-unstable-v1-protocol.c
    )
endif()
