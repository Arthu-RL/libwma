# Changelog

All notable changes to libwma are documented in this file.

## [0.2.0]

### Added
- **Release pipeline**: tagging `vX.Y.Z` now runs `.github/workflows/release.yml`, packaging `libwma.a`/`wma.lib` (+ public headers + CMake package config) for Linux, Android, WebAssembly and Windows and attaching one archive per platform to a draft GitHub Release
- **Windows CI**: a `windows-build` job (`.github/workflows/ci.yml`) builds wma on `windows-latest`/VS 2022, building its `ink` dependency from source since the `vulkan-dev` container image (used for Linux/Android/WASM) has no Windows equivalent

### Fixed
- `cmake/Platform.cmake` now sets `NOMINMAX`/`WIN32_LEAN_AND_MEAN` and, under MSVC, `/EHsc`/`/utf-8` for Windows targets — matching upstream ink's own Windows fixes — so `<windows.h>`'s `min`/`max`/`ERROR` macros no longer collide with `std::min`/`std::max`/`WmaCode::Error`
- `cmake/Dependencies.cmake` now forces `WMA_ENABLE_X11`/`WMA_ENABLE_WAYLAND` off on Windows (those protocols don't exist there; only SDL3/GLFW do), matching the existing Android/WASM guard instead of failing at `find_package()`
- The `windows-debug`/`windows-release` CMake presets previously enabled no backend at all (`createWindowManager()` would always throw); they now enable `WMA_ENABLE_SDL`/`WMA_ENABLE_GLFW`, mirroring the Linux presets' "enable everything this platform supports" approach

## [0.1.1]

### Fixed
- Linked `ink::threading` explicitly instead of relying on it coming in transitively through `ink::ink`, matching upstream ink's split of threading into its own exported target
- Software rendering (`parallelFill()`) now falls back to single-threaded row dispatch on WebAssembly instead of using `ink::ThreadPool`, so wma's WASM build no longer requires `ink::threading` and, in turn, no longer needs `-pthread`/SharedArrayBuffer/COOP-COEP

## [0.1.0]

### Added
- Multiple window-management backends: GLFW, SDL3, X11, Wayland
- Multiple graphics API support: OpenGL, Vulkan, and software (CPU) rendering
- Cross-platform support: Linux, Windows, macOS, Android (SDL3) and WebAssembly (SDL3)
- Parallel software rendering (`parallelFill()`) spreading CPU pixel work across hardware threads
- Keyboard, mouse and touch input listeners with zero-alloc, layered input contexts
- Wayland server-side decorations via `xdg-decoration-unstable-v1`
- CMake presets for desktop (Linux/Windows debug & release), Android and WASM builds
- `find_package(wma)` CMake package config with runtime/compile-time backend querying (`BuildConfig.hpp`)
