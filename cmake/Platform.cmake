if(EMSCRIPTEN)
    message(STATUS "[wma] WASM build")

    set(WMA_NATIVE_OPTIMIZE OFF CACHE BOOL "" FORCE)
    set(WMA_ENABLE_LTO      OFF CACHE BOOL "" FORCE)
    set(WMA_BUILD_TESTS     OFF CACHE BOOL "" FORCE)
endif()

if(ANDROID)
    message(STATUS "[wma] Android build — ABI=${ANDROID_ABI}")

    set(WMA_NATIVE_OPTIMIZE OFF CACHE BOOL "" FORCE)
    set(WMA_BUILD_TESTS     OFF CACHE BOOL "" FORCE)
endif()