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

    execute_process(
        COMMAND "${WAYLAND_SCANNER}" client-header
            "${WAYLAND_PROTOCOLS_DATADIR}/stable/xdg-shell/xdg-shell.xml"
            "${WMA_WAYLAND_PROTO_DIR}/xdg-shell-client-protocol.h"
        RESULT_VARIABLE _wl_scanner_result
    )
    if(_wl_scanner_result)
        message(FATAL_ERROR
            "[wma] wayland-scanner failed to generate xdg-shell-client-protocol.h "
            "(exit code: ${_wl_scanner_result})"
        )
    endif()

    execute_process(
        COMMAND "${WAYLAND_SCANNER}" client-header
            "${WAYLAND_PROTOCOLS_DATADIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml"
            "${WMA_WAYLAND_PROTO_DIR}/xdg-decoration-unstable-v1-client-protocol.h"
        RESULT_VARIABLE _wl_scanner_result
    )
    if(_wl_scanner_result)
        message(FATAL_ERROR
            "[wma] wayland-scanner failed to generate xdg-decoration-unstable-v1-client-protocol.h "
            "(exit code: ${_wl_scanner_result})"
        )
    endif()
endif()
