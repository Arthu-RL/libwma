#ifndef WMA_INPUT_KEYBOARD_LISTENER_HPP
#define WMA_INPUT_KEYBOARD_LISTENER_HPP

#include <ink/ink_base.hpp>
#include <unordered_map>
#include <memory>

#include "KeyAction.hpp"

// Forward declarations
struct GLFWwindow;
struct SDL_Window;
union SDL_Event;
struct SDL_KeyboardEvent;

namespace wma {

    /**
     * @brief Cross-platform keyboard input listener
     * 
     * Provides a unified interface for handling keyboard input across
     * different windowing backends (GLFW/SDL2).
     */
    class KeyboardListener {
    public:
        KeyboardListener() = default;
        ~KeyboardListener() = default;
        
        // Non-copyable
        KeyboardListener(const KeyboardListener&) = delete;
        KeyboardListener& operator=(const KeyboardListener&) = delete;
        
        // Movable
        KeyboardListener(KeyboardListener&&) = default;
        KeyboardListener& operator=(KeyboardListener&&) = default;
        
        /**
         * @brief Add a key action mapping
         * @param key The key code (platform-specific)
         * @param action The action to execute
         */
        void addKeyAction(i32 key, KeyAction action);
        
        /**
         * @brief Remove a key action mapping
         * @param key The key code to remove
         */
        void removeKeyAction(i32 key);
        
        /**
         * @brief Clear all key action mappings
         */
        void clearKeyActions();
        
        /**
         * @brief Check if a key has an action mapped
         * @param key The key code to check
         * @return true if the key has an action mapped
         */
        bool hasKeyAction(i32 key) const;
        
        // Platform-specific initialization
        void initializeGLFW(GLFWwindow* window);
        void initializeSDL(SDL_Window* window);
        
        // Platform-specific event handling
        void handleGLFWKeyEvent(i32 key, i32 action);
        void handleSDLKeyEvent(const SDL_KeyboardEvent& keyEvent);
        
    private:
        std::unordered_map<i32, KeyAction> keyActions_;
        
        // Platform-specific window handles
        GLFWwindow* glfwWindow_ = nullptr;
        SDL_Window* sdlWindow_ = nullptr;
        
        // Static callback functions for GLFW
        static void glfwKeyCallback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods);
        
        // Helper to get instance from window user pointer
        static KeyboardListener* getInstanceFromGLFW(GLFWwindow* window);
    };

} // namespace wma

#endif // WMA_INPUT_KEYBOARD_LISTENER_HPP
