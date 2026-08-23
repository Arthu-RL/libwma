# Windowing

## Link it

```cmake
find_package(wma REQUIRED)
target_link_libraries(your_target PRIVATE wma::wma)
```

## Open a window

```cpp
#include <wma/wma.hpp>

wma::WindowDetails config{
    .width = 1280, .height = 720,
    .resizable = true,
    .targetFPS = 60,        // 0 = unlimited
    .fullscreen = false,
};

auto window = wma::createWindowManager(
    wma::getDefaultBackend(), config, wma::GraphicsAPI::CPU);

window->createWindow("My Game");
```

`getDefaultBackend()` picks what the build actually has, so this line needs no
`#ifdef` on desktop, Android or WASM.

## Frame loop

```cpp
window->process([&] {
    auto* flags = window->getWindowFlags();   // deltaTime, fps, resized, focused, minimized

    if (flags->resized) {
        const auto* d = window->getWindowDetails();
        resize(d->width, d->height);
        flags->resized = false;               // you clear it
    }
});
```

`process()` runs until `shouldClose()`. Call `window->destroy()` to break out.

> On WASM `process()` drives `requestAnimationFrame` and never returns — put
> shutdown work in the loop, not after it.

## Backends and graphics APIs

Two independent axes.

| `WindowBackend` | `GraphicsAPI` |
|---|---|
| `GLFW` `SDL3` `X11` `WAYLAND` | `OpenGL` `Vulkan` `CPU` `Metal` |

```cpp
if (wma::isBackendAvailable(wma::WindowBackend::WAYLAND)) { ... }
```

`createWindowManager` throws `WindowException` for a backend that was not
compiled in — check first, or take the default.

## Graphics interop

```cpp
window->getVulkanExtensions();      // required instance extensions
window->getGLProcAddress("glClear");
window->getMetalLayer();            // CAMetalLayer*
window->getWindowInstance();        // SDL_Window* / GLFWwindow* / X11 Window / wl_surface*
window->getNativeDisplayHandle();   // X11/Wayland only, else nullptr
```

## Software rendering

With `GraphicsAPI::CPU`:

```cpp
wma::SoftwareFramebuffer fb = window->lockFramebuffer();
if (fb.valid()) {
    for (i32 y = 0; y < fb.height; ++y) {
        auto* row = reinterpret_cast<u32*>(
            static_cast<u8*>(fb.pixels) + static_cast<size_t>(y) * fb.pitch);
        for (i32 x = 0; x < fb.width; ++x)
            row[x] = 0x00FF8000;      // XRGB8888
    }
    window->presentFramebuffer();
}
```

Pixels are 32-bit; channel order is backend-defined. Always go through `pitch`,
never `width * 4`.

## Errors

Everything derives from `wma::WMAException`. Catch most-derived first:
`WindowException`, `GraphicsException`, `InputException`, `AudioException`.
