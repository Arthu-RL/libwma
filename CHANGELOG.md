# Changelog

All notable changes to libwma are documented in this file.

## [0.1.0]

### Added
- Multiple window-management backends: GLFW, SDL3, X11, Wayland
- Multiple graphics API support: OpenGL, Vulkan, and software (CPU) rendering
- Cross-platform support: Linux, Windows, macOS, Android (SDL3) and WebAssembly (SDL3)
- Parallel software rendering (`parallelFill()`) spreading CPU pixel work across hardware threads, falling back to a single-threaded row dispatch on WebAssembly (so the WASM build needs neither `ink::threading` nor `-pthread`/SharedArrayBuffer/COOP-COEP)
- Keyboard, mouse and touch input listeners with zero-alloc, layered input contexts
- Wayland server-side decorations via `xdg-decoration-unstable-v1`
- CMake presets for desktop (Linux/Windows debug & release), Android and WASM builds
- `find_package(wma)` CMake package config with runtime/compile-time backend querying (`BuildConfig.hpp`), linking `ink::ink` and `ink::threading` explicitly
- Windows support: `cmake/Platform.cmake` sets `NOMINMAX`/`WIN32_LEAN_AND_MEAN` and, under MSVC, `/EHsc`/`/utf-8`/`/Zc:__cplusplus`; `cmake/Dependencies.cmake` forces `WMA_ENABLE_X11`/`WMA_ENABLE_WAYLAND` off there (only SDL3/GLFW exist on Windows); the `windows-debug`/`windows-release` CMake presets enable `WMA_ENABLE_SDL`/`WMA_ENABLE_GLFW`
- Release pipeline: tagging `vX.Y.Z` runs `.github/workflows/release.yml`, packaging `libwma.a`/`wma.lib` (+ public headers + CMake package config) for Linux, Android, WebAssembly and Windows, attaching one archive per platform to a draft GitHub Release
- Windows CI: a `windows-build` job (`.github/workflows/ci.yml`) builds wma on `windows-latest` via Ninja + `ilammy/msvc-dev-cmd`/`seanmiddleditch/gha-setup-ninja`, building its `ink` dependency from source since the `vulkan-dev` container image (used for Linux/Android/WASM) has no Windows equivalent
