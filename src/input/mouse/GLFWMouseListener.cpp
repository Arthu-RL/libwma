#ifdef WMA_ENABLE_GLFW
#include "wma/input/mouse/GLFWMouseListener.hpp"
#include "wma/exceptions/WMAException.hpp"
#include "wma/input/KeyboardListener.hpp"
#include "wma/core/Types.hpp"

#include <GLFW/glfw3.h>

namespace wma {

GLFWMouseListener::GLFWMouseListener()
    : MouseListener()
    , glfwWindow_(nullptr)
{
}

GLFWMouseListener::~GLFWMouseListener()
{
    if (glfwWindow_) {
        glfwSetMouseButtonCallback(glfwWindow_, nullptr);
        glfwSetCursorPosCallback(glfwWindow_, nullptr);
        glfwSetScrollCallback(glfwWindow_, nullptr);
    }
}

void GLFWMouseListener::initialize(GLFWwindow* window)
{
    if (!window) {
        throw InputException("Invalid GLFW window pointer");
    }

    glfwWindow_ = window;

    // Get initial cursor position
    f64 xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    currentPosition_ = WMAMousePosition(xpos, ypos);
    lastPosition_ = currentPosition_;
    firstMouse_ = true;

    updateCursorState();

    // Set callbacks
    glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);
    glfwSetCursorPosCallback(window, glfwCursorPosCallback);
    glfwSetScrollCallback(window, glfwScrollCallback);
}

void GLFWMouseListener::handleButtonEvent(i32 button, i32 action, i32 mods)
{
    i32 unifiedButton = convertButton(button);

    if (buttonActions_.find(unifiedButton) != buttonActions_.end()) {
        PendingEvent event;
        event.button = unifiedButton;

        if (action == GLFW_PRESS) {
            event.type = PendingEvent::WMAButtonPress;
        } else if (action == GLFW_RELEASE) {
            event.type = PendingEvent::WMAButtonRelease;
        }

        if (event.type != PendingEvent::WMANone) {
            processPendingEvents(event);
        }
    }
}

void GLFWMouseListener::handlePositionEvent(f64 xpos, f64 ypos)
{
    if (firstMouse_) {
        lastPosition_ = WMAMousePosition(xpos, ypos);
        firstMouse_ = false;
    }

    f64 deltaX = (xpos - lastPosition_.x) * sensitivity_;
    f64 deltaY = (lastPosition_.y - ypos) * sensitivity_;

    currentPosition_ = WMAMousePosition(xpos, ypos, deltaX, deltaY);

    if (moveAction_.hasMoveAction()) {
        PendingEvent event;
        event.type = PendingEvent::WMAMove;
        event.position = currentPosition_;
        processPendingEvents(event);
    }

    lastPosition_ = WMAMousePosition(xpos, ypos);
}

void GLFWMouseListener::handleScrollEvent(f64 xoffset, f64 yoffset)
{
    if (scrollAction_.hasScrollAction()) {
        PendingEvent event;
        event.type = PendingEvent::WMAScroll;
        event.scroll = WMAMouseScroll(xoffset, yoffset);
        processPendingEvents(event);
    }
}

void GLFWMouseListener::glfwMouseButtonCallback(GLFWwindow* window, i32 button, i32 action, i32 mods)
{
    auto* listener = getInstanceFromWindow(window);
    if (listener) {
        listener->handleButtonEvent(button, action, mods);
    }
}

void GLFWMouseListener::glfwCursorPosCallback(GLFWwindow* window, f64 xpos, f64 ypos)
{
    auto* listener = getInstanceFromWindow(window);
    if (listener) {
        listener->handlePositionEvent(xpos, ypos);
    }
}

void GLFWMouseListener::glfwScrollCallback(GLFWwindow* window, f64 xoffset, f64 yoffset)
{
    auto* listener = getInstanceFromWindow(window);
    if (listener) {
        listener->handleScrollEvent(xoffset, yoffset);
    }
}

GLFWMouseListener* GLFWMouseListener::getInstanceFromWindow(GLFWwindow* window)
{
    if (!window) return nullptr;

    auto* userData = static_cast<GlfwUserData*>(glfwGetWindowUserPointer(window));
    if (!userData || !userData->mouseListener) return nullptr;

    // Cast the base pointer to derived type
    return dynamic_cast<GLFWMouseListener*>(userData->mouseListener);
}

void GLFWMouseListener::updateCursorState()
{
    if (glfwWindow_) {
        if (cursorEnabled_) {
            glfwSetInputMode(glfwWindow_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(glfwWindow_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

i32 GLFWMouseListener::convertButton(i32 glfwButton) const
{
    switch (glfwButton) {
    case GLFW_MOUSE_BUTTON_LEFT:   return MouseButton::WMALeft;
    case GLFW_MOUSE_BUTTON_RIGHT:  return MouseButton::WMARight;
    case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButton::WMAMiddle;
    case GLFW_MOUSE_BUTTON_4:      return MouseButton::WMAButton4;
    case GLFW_MOUSE_BUTTON_5:      return MouseButton::WMAButton5;
    case GLFW_MOUSE_BUTTON_6:      return MouseButton::WMAButton6;
    case GLFW_MOUSE_BUTTON_7:      return MouseButton::WMAButton7;
    case GLFW_MOUSE_BUTTON_8:      return MouseButton::WMAButton8;
    default:                       return glfwButton;
    }
}

} // namespace wma

#endif
