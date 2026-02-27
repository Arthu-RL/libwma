#ifdef WMA_ENABLE_GLFW
#include "wma/input/keyboard/GLFWKeyboardListener.hpp"
#include "wma/input/keyboard/Keys.h"
#include "wma/exceptions/WMAException.hpp"
#include "wma/input/KeyboardListener.hpp"

#include <GLFW/glfw3.h>

namespace wma {

GLFWKeyboardListener::GLFWKeyboardListener()
    : KeyboardListener()
    , glfwWindow_(nullptr)
{
}

GLFWKeyboardListener::~GLFWKeyboardListener()
{
    if (glfwWindow_) {
        glfwSetKeyCallback(glfwWindow_, nullptr);
    }
}

void GLFWKeyboardListener::initialize(GLFWwindow* window)
{
    if (!window) {
        throw InputException("Invalid GLFW window pointer");
    }

    glfwWindow_ = window;

    // Set the instance pointer as user data
    auto* userData = static_cast<GlfwUserData*>(glfwGetWindowUserPointer(window));
    if (userData) {
        userData->keyboardListener = this;
    }

    // Set the callback
    glfwSetKeyCallback(window, glfwKeyCallback);
}

void GLFWKeyboardListener::handleKeyEvent(i32 key, i32 action)
{
    Key mappedKey = mapGLFWKey(key);
    auto it = keyActions_.find(static_cast<i32>(mappedKey));

    if (it != keyActions_.end()) {
        if (action == GLFW_PRESS) {
            it->second.executePress();
        } else if (action == GLFW_RELEASE) {
            it->second.executeRelease();
        }
        // Note: GLFW_REPEAT can be handled here if needed
    }
}

void GLFWKeyboardListener::glfwKeyCallback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods)
{
    auto* listener = getInstanceFromWindow(window);
    if (listener) {
        listener->handleKeyEvent(key, action);
    }
}

GLFWKeyboardListener* GLFWKeyboardListener::getInstanceFromWindow(GLFWwindow* window)
{
    if (!window) return nullptr;

    auto* userData = static_cast<GlfwUserData*>(glfwGetWindowUserPointer(window));
    if (!userData || !userData->keyboardListener) return nullptr;

    // Cast the base pointer to derived type
    return dynamic_cast<GLFWKeyboardListener*>(userData->keyboardListener);
}

} // namespace wma
#endif
