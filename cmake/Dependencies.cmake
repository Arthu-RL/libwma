find_package(ink REQUIRED CONFIG)

# Backend options — desktop-only, automatically disabled on cross-compile targets.
option(WMA_ENABLE_SDL     "Enable SDL3 backend"    OFF)
option(WMA_ENABLE_GLFW    "Enable GLFW backend"    OFF)
option(WMA_ENABLE_X11     "Enable X11 backend"     OFF)
option(WMA_ENABLE_WAYLAND "Enable Wayland backend" OFF)

if(ANDROID OR EMSCRIPTEN)
    foreach(_backend SDL GLFW X11 WAYLAND)
        if(WMA_ENABLE_${_backend})
            message(WARNING
                "[wma] WMA_ENABLE_${_backend} is not supported on "
                "${CMAKE_SYSTEM_NAME} — forcing OFF"
            )
        endif()
        set(WMA_ENABLE_${_backend} OFF CACHE BOOL "" FORCE)
    endforeach()
endif()

if(WMA_ENABLE_SDL)
    find_package(SDL3 REQUIRED CONFIG COMPONENTS SDL3)
endif()

if(WMA_ENABLE_GLFW)
    find_package(glfw3 REQUIRED)
endif()

if(WMA_ENABLE_X11)
    find_package(X11 REQUIRED)
endif()

if(WMA_ENABLE_WAYLAND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND REQUIRED IMPORTED_TARGET
        wayland-client
        wayland-cursor
        xkbcommon
    )

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
endif()
