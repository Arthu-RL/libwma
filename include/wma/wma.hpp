#ifndef WMA_H
#define WMA_H

#include <ink/ink_base.hpp>
#include <ink/InkAssert.h>

#include "core/Types.hpp"
#include "core/WindowDetails.hpp"
#include "core/WindowFlags.hpp"
#include "exceptions/WMAException.hpp"
#include "input/keyboard/Keys.h"
#include "input/keyboard/KeyAction.hpp"
#include "input/keyboard/KeyboardListener.hpp"
#include "managers/IWindowManager.hpp"

#ifdef WMA_ENABLE_GLFW
    #include "backends/glfw/GlfwWindowManager.hpp"
#endif

#ifdef WMA_ENABLE_SDL
    #include "backends/sdl/SdlWindowManager.hpp"
#endif

#ifdef WMA_ENABLE_WAYLAND
    #include "backends/wayland/WaylandWindowManager.hpp"
#endif

#ifdef WMA_ENABLE_X11
    #include "backends/x11/X11WindowManager.hpp"
#endif

#define WMA_MAJOR_VERSION 1
#define WMA_MINOR_VERSION 0
#define WMA_PATCH_VERSION 0
#define WMA_VERSION ((WMA_MAJOR_VERSION * 10000) + (WMA_MINOR_VERSION * 100) + WMA_PATCH_VERSION)
#define WMA_VERSION_STRING_FULL INK_STR(WMA_MAJOR_VERSION) "." INK_STR(WMA_MINOR_VERSION) "." INK_STR(WMA_PATCH_VERSION)

namespace wma {

    inline std::unique_ptr<IWindowManager> createWindowManager(
        WindowBackend backend,
        const WindowDetails& windowDetails,
        GraphicsAPI graphicsAPI)
    {
        switch (backend) {
#ifdef WMA_ENABLE_GLFW
        case WindowBackend::GLFW:
            return std::make_unique<GlfwWindowManager>(windowDetails, graphicsAPI);
#endif
#ifdef WMA_ENABLE_SDL
        case WindowBackend::SDL2:
            return std::make_unique<SdlWindowManager>(windowDetails, graphicsAPI);
#endif
#ifdef WMA_ENABLE_WAYLAND
        case WindowBackend::WAYLAND:
            return std::make_unique<WaylandWindowManager>(windowDetails, graphicsAPI);
#endif
#ifdef WMA_ENABLE_X11
        case WindowBackend::X11:
            return std::make_unique<X11WindowManager>(windowDetails, graphicsAPI);
#endif
        default:
            throw WMAException("Requested window backend is not available or not compiled in");
        }
    }

    inline WindowBackend getDefaultBackend() {
#ifdef WMA_ENABLE_WAYLAND
        return WindowBackend::WAYLAND;
#elif defined(WMA_ENABLE_SDL)
        return WindowBackend::SDL2;
#elif defined(WMA_ENABLE_GLFW)
        return WindowBackend::GLFW;
#elif defined(WMA_ENABLE_X11)
        return WindowBackend::X11;
#else
        INK_ASSERT_MSG(false, "No window backend is enabled");
#endif
    }

    inline bool isBackendAvailable(WindowBackend backend) {
        switch (backend) {
#ifdef WMA_ENABLE_GLFW
        case WindowBackend::GLFW: return true;
#endif
#ifdef WMA_ENABLE_SDL
        case WindowBackend::SDL2: return true;
#endif
#ifdef WMA_ENABLE_WAYLAND
        case WindowBackend::WAYLAND: return true;
#endif
#ifdef WMA_ENABLE_X11
        case WindowBackend::X11: return true;
#endif
        default: return false;
        }
    }

    inline const char* getLibraryInfo() {
        static const char* info =
            "WMA Window Management & Input Abstraction Library v" WMA_VERSION_STRING_FULL "\n"
            "Backends: "
#ifdef WMA_ENABLE_GLFW
            "GLFW "
#endif
#ifdef WMA_ENABLE_SDL
            "SDL2 "
#endif
#ifdef WMA_ENABLE_WAYLAND
            "Wayland "
#endif
#ifdef WMA_ENABLE_X11
            "X11 "
#endif
            "\nGraphics APIs: Vulkan OpenGL"
            "\nBuilt with C++17";
        return info;
    }

} // namespace wma

#endif // WMA_H
