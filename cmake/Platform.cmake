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
        add_compile_options(/EHsc /utf-8)
    endif()
endif()
