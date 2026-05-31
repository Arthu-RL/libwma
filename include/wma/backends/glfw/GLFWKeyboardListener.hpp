#ifndef WMA_BACKENDS_GLFW_KEYBOARD_LISTENER_HPP
#define WMA_BACKENDS_GLFW_KEYBOARD_LISTENER_HPP

#include "wma/input/keyboard/KeyboardListener.hpp"

struct GLFWwindow;

namespace wma {

class GLFWKeyboardListener : public KeyboardListener {
public:
    GLFWKeyboardListener();
    ~GLFWKeyboardListener() override;

    void initialize(GLFWwindow* window);
    void handleKeyEvent(i32 key, i32 action);
    static void glfwKeyCallback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods);

private:
    GLFWwindow* glfwWindow_ = nullptr;
    static GLFWKeyboardListener* getInstanceFromWindow(GLFWwindow* window);
};

} // namespace wma

#endif // WMA_BACKENDS_GLFW_KEYBOARD_LISTENER_HPP
