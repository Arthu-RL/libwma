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
