# Changelog

All notable changes to libwma are documented in this file.

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
