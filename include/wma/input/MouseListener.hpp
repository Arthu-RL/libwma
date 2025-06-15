#ifndef WMA_INPUT_MOUSE_LISTENER_HPP
#define WMA_INPUT_MOUSE_LISTENER_HPP

#include <unordered_map>
#include <memory>
#include "MouseAction.hpp"

// Forward declarations
struct GLFWwindow;
struct SDL_Window;
union SDL_Event;

namespace wma {

    /**
     * @brief Cross-platform mouse input listener
     * 
     * Provides a unified interface for handling mouse input across
     * different windowing backends (GLFW/SDL2).
     */
    class MouseListener {
    public:
        MouseListener() = default;
        ~MouseListener() = default;
        
        // Non-copyable
        MouseListener(const MouseListener&) = delete;
        MouseListener& operator=(const MouseListener&) = delete;
        
        // Movable
        MouseListener(MouseListener&&) = default;
        MouseListener& operator=(MouseListener&&) = default;
        
        /**
         * @brief Add a mouse button action mapping
         * @param button The mouse button code (platform-agnostic)
         * @param action The action to execute
         */
        void addButtonAction(i32 button, MouseAction action);
        
        /**
         * @brief Remove a mouse button action mapping
         * @param button The button code to remove
         */
        void removeButtonAction(i32 button);
        
        /**
         * @brief Set global mouse movement action
         * @param action The action to execute on mouse movement
         */
        void setMoveAction(MouseAction action);
        
        /**
         * @brief Set global mouse scroll action
         * @param action The action to execute on mouse scroll
         */
        void setScrollAction(MouseAction action);
        
        /**
         * @brief Clear all mouse action mappings
         */
        void clearAllActions();
        
        /**
         * @brief Check if a button has an action mapped
         * @param button The button code to check
         * @return true if the button has an action mapped
         */
        bool hasButtonAction(i32 button) const;
        
        /**
         * @brief Get current mouse position
         * @return MousePosition Current mouse position
         */
        MousePosition getCurrentPosition() const;
        
        /**
         * @brief Enable/disable mouse cursor
         * @param enabled true to show cursor, false to hide
         */
        void setCursorEnabled(bool enabled);
        
        /**
         * @brief Check if cursor is enabled
         * @return true if cursor is visible
         */
        bool isCursorEnabled() const;
        
        /**
         * @brief Set mouse sensitivity for movement calculations
         * @param sensitivity Sensitivity multiplier (default: 1.0)
         */
        void setSensitivity(f64 sensitivity);
        
        /**
         * @brief Get current mouse sensitivity
         * @return Current sensitivity value
         */
        f64 getSensitivity() const;
        
        // Platform-specific initialization
        void initializeGLFW(GLFWwindow* window);
        void initializeSDL(SDL_Window* window);
        
        // Platform-specific event handling
        void handleGLFWButtonEvent(i32 button, i32 action, i32 mods);
        void handleGLFWPositionEvent(f64 xpos, f64 ypos);
        void handleGLFWScrollEvent(f64 xoffset, f64 yoffset);
        void handleSDLEvent(const SDL_Event& event);
        
    private:
        std::unordered_map<i32, MouseAction> buttonActions_;
        MouseAction moveAction_;
        MouseAction scrollAction_;
        
        // Mouse state
        MousePosition currentPosition_;
        MousePosition lastPosition_;
        bool cursorEnabled_ = true;
        f64 sensitivity_ = 1.0;
        bool firstMouse_ = true;
        
        // Platform-specific window handles
        GLFWwindow* glfwWindow_ = nullptr;
        SDL_Window* sdlWindow_ = nullptr;
        
        // Static callback functions for GLFW
        static void glfwMouseButtonCallback(GLFWwindow* window, i32 button, i32 action, i32 mods);
        static void glfwCursorPosCallback(GLFWwindow* window, f64 xpos, f64 ypos);
        static void glfwScrollCallback(GLFWwindow* window, f64 xoffset, f64 yoffset);
        
        // Helper to get instance from window user pointer
        static MouseListener* getInstanceFromGLFW(GLFWwindow* window);
        
        // Platform-specific cursor management
        void updateCursorStateGLFW();
        void updateCursorStateSDL();
        
        // Convert platform-specific button codes to unified format
        i32 convertGLFWButton(i32 glfwButton) const;
        i32 convertSDLButton(i32 sdlButton) const;
    };

} // namespace wma

#endif // WMA_INPUT_MOUSE_LISTENER_HPP
