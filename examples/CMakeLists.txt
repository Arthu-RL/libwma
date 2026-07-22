# Example applications. Enabled via -DWMA_BUILD_EXAMPLES=ON.
#
# The example targets SDL3 (via wma::getDefaultBackend()) so the exact same
# source builds unmodified for desktop, Android and WASM:
#
#   Desktop    -> a normal executable.
#   Android    -> a SHARED library named "main" (SDL's SDLActivity loads a
#                 native library called "main" via JNI). This repo does not
#                 generate a Gradle project — wiring this .so into one (e.g.
#                 SDL's android-project template, `app/src/main/jniLibs`, or
#                 an `externalNativeBuild` CMake target) is left to whatever
#                 Gradle project consumes the library.
#   WASM       -> an executable that emits basic_window.html/.js/.wasm, using
#                 a minimal shell with the <canvas id="canvas"> SDL3 expects.

if(ANDROID)
    add_library(wma_basic_window SHARED basic_window/main.cpp)
    set_target_properties(wma_basic_window PROPERTIES OUTPUT_NAME "main")
else()
    add_executable(wma_basic_window basic_window/main.cpp)
endif()

target_link_libraries(wma_basic_window PRIVATE wma::wma)

if(EMSCRIPTEN)
    set_target_properties(wma_basic_window PROPERTIES SUFFIX ".html")
    target_link_options(wma_basic_window PRIVATE
        # This example links the real SDL3 port found via find_package(SDL3);
        # disable Emscripten's own bundled SDL port so the two never collide.
        "-sUSE_SDL=0"
        "-sALLOW_MEMORY_GROWTH=1"
        # process() never returns (emscripten_set_main_loop keeps running),
        # so there is no runtime-exit path to unwind for.
        "-sEXIT_RUNTIME=0"
        "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/basic_window/shell.wasm.html"
    )
endif()
