#ifdef WMA_ENABLE_GLFW
#ifndef WMA_INPUT_GLFW_KEYBOARD_LISTENER_HPP
#define WMA_INPUT_GLFW_KEYBOARD_LISTENER_HPP

#include "wma/input/keyboard/KeyboardListener.hpp"

struct GLFWwindow;

namespace wma {

/**
 * @brief GLFW-specific keyboard listener implementation
 */
class GLFWKeyboardListener : public KeyboardListener {
public:
    GLFWKeyboardListener();
    ~GLFWKeyboardListener() override;

    /**
     * @brief Initialize the listener with a GLFW window
     * @param window The GLFW window to attach to
     */
    void initialize(GLFWwindow* window);

    /**
     * @brief Handle GLFW key events
     * @param key The GLFW key code
     * @param action The key action (press/release/repeat)
     */
    void handleKeyEvent(i32 key, i32 action);

    // Static GLFW callback
    static void glfwKeyCallback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods);

private:
    GLFWwindow* glfwWindow_ = nullptr;

    static GLFWKeyboardListener* getInstanceFromWindow(GLFWwindow* window);
};

} // namespace wma

#endif // WMA_INPUT_GLFW_KEYBOARD_LISTENER_HPP
#endif
