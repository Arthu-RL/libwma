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
endif()
