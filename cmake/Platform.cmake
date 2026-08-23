if(EMSCRIPTEN)
    message(STATUS "[wma] WASM build — SDL3 is the only supported backend")

    set(WMA_NATIVE_OPTIMIZE OFF CACHE BOOL "" FORCE)
    set(WMA_ENABLE_LTO      OFF CACHE BOOL "" FORCE)
    set(WMA_BUILD_TESTS     OFF CACHE BOOL "" FORCE)

    # SDL3 is the sole web backend; desktop backends do not exist on the platform.
    set(WMA_ENABLE_SDL  ON  CACHE BOOL "" FORCE)
endif()

if(ANDROID)
    message(STATUS "[wma] Android build — ABI=${ANDROID_ABI}, SDL3 backend")

    set(WMA_NATIVE_OPTIMIZE OFF CACHE BOOL "" FORCE)
    set(WMA_BUILD_TESTS     OFF CACHE BOOL "" FORCE)

    # SDL3 is the sole Android backend.
    set(WMA_ENABLE_SDL  ON  CACHE BOOL "" FORCE)
endif()

if(APPLE)
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        message(STATUS "[wma] iOS build — SDL3 is the only supported backend")

        # GLFW has no iOS port.
        set(WMA_ENABLE_SDL  ON  CACHE BOOL "" FORCE)
        set(WMA_ENABLE_GLFW OFF CACHE BOOL "" FORCE)

        # -march=native is meaningless when cross-compiling for the device, and
        # neither the examples (which want a launchable binary) nor the tests
        # (which CTest cannot start on a device) are runnable from here.
        set(WMA_NATIVE_OPTIMIZE OFF CACHE BOOL "" FORCE)
        set(WMA_BUILD_EXAMPLES  OFF CACHE BOOL "" FORCE)
        set(WMA_BUILD_TESTS     OFF CACHE BOOL "" FORCE)
    else()
        message(STATUS "[wma] Target platform: macOS  "
                       "Arch=${CMAKE_OSX_ARCHITECTURES}  Compiler=${CMAKE_CXX_COMPILER_ID}")
    endif()

    # X11 and Wayland are Linux display protocols. XQuartz can supply an X server
    # on macOS, but a window opened through it is not a native Cocoa window and so
    # cannot host a CAMetalLayer — which is the reason this platform is supported
    # in the first place.
    foreach(_backend X11 WAYLAND)
        if(WMA_ENABLE_${_backend})
            message(WARNING
                "[wma] WMA_ENABLE_${_backend} is not supported on Apple platforms — "
                "forcing OFF (use SDL3/GLFW)"
            )
        endif()
        set(WMA_ENABLE_${_backend} OFF CACHE BOOL "" FORCE)
    endforeach()
endif()

if(WIN32 AND NOT EMSCRIPTEN)
    message(STATUS "[wma] Target platform: Windows  Compiler=${CMAKE_CXX_COMPILER_ID}")

    # <windows.h>'s min/max macros collide with std::min/std::max, and its ERROR
    # macro collides with WmaCode::Error (see core/Types.hpp) WIN32_LEAN_AND_MEAN
    # additionally keeps the winsock/GDI surface (unused here) out of the build.
    # Applies to every target in the tree, including consumers that pull in
    # <windows.h> themselves.
    add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)

    if(MSVC)
        # /EHsc: standard C++ exception unwinding (off by default under cl.exe;
        # the library and ink both throw). /utf-8: source and execution charset,
        # matching GCC/Clang defaults.
        # /Zc:__cplusplus: cl.exe reports __cplusplus as 199711L regardless of
        # the active /std: flag unless this is set, which trips ink_base.hpp's
        # `#if __cplusplus < 202100L` C++23 guard even when CMAKE_CXX_STANDARD 23
        # has correctly selected -std:c++latest.
        add_compile_options(/EHsc /utf-8 /Zc:__cplusplus)
    endif()
endif()
