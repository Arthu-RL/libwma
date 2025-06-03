#pragma once
#ifndef wma_MANAGERS_IWINDOW_MANAGER_HPP
#define wma_MANAGERS_IWINDOW_MANAGER_HPP

#include <functional>
#include <vector>
#include <memory>

#include "../core/Types.hpp"
#include "../core/WindowDetails.hpp"
#include "../core/WindowFlags.hpp"
#include "../input/KeyboardListener.hpp"
#include "../input/MouseListener.hpp"

namespace wma {

    /**
     * @brief Abstract base interface for window managers
     * 
     * Provides a common interface for different windowing backends
     * while maintaining type safety and performance.
     */
    class IWindowManager {
    public:
        virtual ~IWindowManager() = default;
        
        /**
         * @brief Create a window with the specified title
         * @param windowName The title of the window
         */
        virtual void createWindow(const char* windowName) = 0;
        
        /**
         * @brief Main event loop processing
         * @param actions Callback function to execute each frame
         */
        virtual void process(std::function<void()>&& actions) = 0;
        
        /**
         * @brief Get window flags for current state
         * @return Pointer to window flags structure
         */
        virtual WindowFlags* getWindowFlags() = 0;
        
        /**
         * @brief Get window details/configuration
         * @return Pointer to window details structure
         */
        virtual const WindowDetails* getWindowDetails() const = 0;
        
        /**
         * @brief Get required Vulkan extensions
         * @return Vector of extension names
         */
        virtual std::vector<const char*> getVulkanExtensions() const = 0;
        
        /**
         * @brief Get the keyboard listener instance
         * @return Reference to keyboard listener
         */
        virtual KeyboardListener& getKeyboardListener() = 0;

        /**
         * @brief Get the Mouse listener instance
         * @return Reference to mouse listener
         */
        virtual MouseListener& getMouseListener() = 0;
        
        /**
         * @brief Check if window should close
         * @return true if window should close
         */
        virtual bool shouldClose() const = 0;
        
        /**
         * @brief Get window backend type
         * @return The backend type (GLFW/SDL2)
         */
        virtual WindowBackend getBackendType() const = 0;
        
        /**
         * @brief Get graphics API being used
         * @return The graphics API type
         */
        virtual GraphicsAPI getGraphicsAPI() const = 0;
        
    protected:
        // Protected constructor to prevent direct instantiation
        IWindowManager() = default;
    };

    /**
     * @brief Factory function type for creating window managers
     */
    using WindowManagerFactory = std::function<std::unique_ptr<IWindowManager>(const WindowDetails&)>;

} // namespace wma

#endif // wma_MANAGERS_IWINDOW_MANAGER_HPP
