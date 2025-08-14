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
    std::unique_ptr<IWindowManager> createWindowManager(
        WindowBackend backend,
        const WindowDetails& windowDetails,
        GraphicsAPI graphicsAPI = GraphicsAPI::Vulkan
    );

    /**
     * @brief Get default window backend based on platform and compilation
     * @return The recommended backend for current platform
     */
    WindowBackend getDefaultBackend();

    /**
     * @brief Check if a specific backend is available
     * @param backend The backend to check
     * @return true if backend is available
     */
    bool isBackendAvailable(WindowBackend backend);

    /**
     * @brief Get library information string
     * @return String containing version and build info
     */
    const char* getLibraryInfo();

} // namespace wma

#endif // WMA_H
