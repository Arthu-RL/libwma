#ifndef WMA_H
#define WMA_H

/**
 * @file wma.hpp
 * @brief Main header for WMA cross-platform window management Abstraction library
 * 
 * WMA provides a unified interface for window management across different
 * backends (GLFW/SDL2) and graphics APIs (Vulkan/OpenGL).
 * 
 * @version 1.0.0
 * @author WMA Library Team
 */

#include <ink/ink_base.hpp>
#include <ink/InkAssert.h>

// Core components
#include "core/Types.hpp"
#include "core/WindowDetails.hpp"
#include "core/WindowFlags.hpp"

// Exception handling
#include "exceptions/WMAException.hpp"

// Input handling
#include "input/Keys.h"
#include "input/KeyAction.hpp"
#include "input/KeyboardListener.hpp"

// Window management
#include "managers/IWindowManager.hpp"

// Conditional backend includes
#ifdef WMA_ENABLE_GLFW
    #include "managers/GlfwWindowManager.hpp"
#endif

#ifdef WMA_ENABLE_SDL
    #include "managers/SdlWindowManager.hpp"
#endif

#ifdef WMA_ENABLE_X11
#include "managers/X11WindowManager.hpp"
#endif

/*====================
 * WMA VERSION INFO
 *====================*/
#define WMA_MAJOR_VERSION 1
#define WMA_MINOR_VERSION 0
#define WMA_PATCH_VERSION 0
#define WMA_VERSION ((WMA_MAJOR_VERSION * 10000) + (WMA_MINOR_VERSION * 100) + WMA_PATCH_VERSION)
#define WMA_VERSION_STRING_FULL INK_STR(WMA_MAJOR_VERSION) "." INK_STR(WMA_MINOR_VERSION) "." INK_STR(WMA_PATCH_VERSION)

namespace wma {

    /**
     * @brief Factory function to create a window manager
     * @param backend The windowing backend to use
     * @param windowDetails Window configuration
     * @param graphicsAPI Graphics API to use
     * @return Unique pointer to window manager instance
     */
    inline std::unique_ptr<IWindowManager> createWindowManager(
        WindowBackend backend,
        const WindowDetails& windowDetails,
        GraphicsAPI graphicsAPI
        ) {
        switch (backend) {
#ifdef WMA_ENABLE_GLFW
        case WindowBackend::GLFW:
            return std::make_unique<GlfwWindowManager>(windowDetails, graphicsAPI);
#endif

#ifdef WMA_ENABLE_SDL
        case WindowBackend::SDL2:
            return std::make_unique<SdlWindowManager>(windowDetails, graphicsAPI);
#endif

#ifdef WMA_ENABLE_X11
        case WindowBackend::X11:
            return std::make_unique<X11WindowManager>(windowDetails, graphicsAPI);
#endif
        default:
            throw WMAException("Requested window backend is not available or not compiled in");
        }
    }

    /**
     * @brief Get default window backend based on platform and compilation
     * @return The recommended backend for current platform
     */
    inline WindowBackend getDefaultBackend() {
        // Priority order: GLFW first (generally more stable for graphics development)
#ifdef WMA_ENABLE_GLFW
        return WindowBackend::GLFW;
#elif defined(WMA_ENABLE_SDL)
        return WindowBackend::SDL2;
#elif defined(WMA_ENABLE_X11)
        return WindowBackend::X11;
#else
        INK_ASSERT_MSG(false, "No window backend is enabled");
#endif
    }


    /**
     * @brief Check if a specific backend is available
     * @param backend The backend to check
     * @return true if backend is available
     */
    inline bool isBackendAvailable(WindowBackend backend) {
        switch (backend) {
#ifdef WMA_ENABLE_GLFW
        case WindowBackend::GLFW:
            return true;
#endif

#ifdef WMA_ENABLE_SDL
        case WindowBackend::SDL2:
            return true;
#endif

#ifdef WMA_ENABLE_X11
        case WindowBackend::X11:
            return true;
#endif
        default:
            return false;
        }
    }

    /**
     * @brief Get library information string
     * @return String containing version and build info
     */
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
#ifdef WMA_ENABLE_X11
            "X11 "
#endif

            "\nGraphics APIs: "
#ifdef WMA_ENABLE_VULKAN
            "Vulkan "
#endif
#ifdef WMA_ENABLE_OPENGL
            "OpenGL "
#endif
            "\nBuilt with C++17";

        return info;
    }

} // namespace wma

#endif // WMA_H
