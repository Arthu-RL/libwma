#include "wma/input/MouseListener.hpp"
#include "wma/exceptions/WMAException.hpp"
#include "wma/input/KeyboardListener.hpp"
#include "wma/core/Types.hpp"

#include <ink/InkAssert.h>
#include <ink/Inkogger.h>
#include <algorithm>

#ifdef WMA_ENABLE_GLFW
#include <GLFW/glfw3.h>
#endif

#ifdef WMA_ENABLE_SDL
#include <SDL2/SDL.h>
#endif

namespace wma {

    MouseListener::MouseListener()
        : currentPosition_{}
        , lastPosition_{}
#ifdef WMA_ENABLE_GLFW
        , glfwWindow_(nullptr)
#endif
#ifdef WMA_ENABLE_SDL
        , sdlWindow_(nullptr)
#endif
    {
        // Empty
    }

    MouseListener::~MouseListener() {
#ifdef WMA_ENABLE_GLFW
        // Clear callbacks if still active
        if (glfwWindow_) {
            glfwSetMouseButtonCallback(glfwWindow_, nullptr);
            glfwSetCursorPosCallback(glfwWindow_, nullptr);
            glfwSetScrollCallback(glfwWindow_, nullptr);
        }
#endif
    }

    void MouseListener::addButtonAction(i32 button, MouseAction action)
    {
        buttonActions_[button] = std::move(action);
    }

    void MouseListener::removeButtonAction(i32 button)
    {
        buttonActions_.erase(button);
    }

    void MouseListener::setMoveAction(MouseAction action)
    {
        moveAction_ = std::move(action);
    }

    void MouseListener::setScrollAction(MouseAction action)
    {
        scrollAction_ = std::move(action);
    }

    void MouseListener::clearAllActions()
    {
        buttonActions_.clear();
        moveAction_ = MouseAction{};
        scrollAction_ = MouseAction{};
    }

    bool MouseListener::hasButtonAction(i32 button) const
    {
        return buttonActions_.find(button) != buttonActions_.end();
    }

    WMAMousePosition MouseListener::getCurrentPosition() const
    {
        return currentPosition_;
    }

    void MouseListener::setCursorEnabled(bool enabled)
    {
        if (cursorEnabled_ != enabled) {
            cursorEnabled_ = enabled;
    #ifdef WMA_ENABLE_GLFW
            updateCursorStateGLFW();
    #endif
    #ifdef WMA_ENABLE_SDL
            updateCursorStateSDL();
    #endif
        }
    }

    bool MouseListener::isCursorEnabled() const
    {
        return cursorEnabled_;
    }

    void MouseListener::setSensitivity(f64 sensitivity)
    {
        sensitivity_ = sensitivity;
    }

    f64 MouseListener::getSensitivity() const
    {
        return sensitivity_;
    }

    void MouseListener::processPendingEvents(const PendingEvent& event)
    {
        switch (event.type) {
        case PendingEvent::WMAMove:
            if (moveAction_.hasMoveAction()) {
                moveAction_.executeMove(event.position);
            }
            break;

        case PendingEvent::WMAScroll:
            if (scrollAction_.hasScrollAction()) {
                scrollAction_.executeScroll(event.scroll);
            }
            break;

        case PendingEvent::WMAButtonPress: {
            auto it = buttonActions_.find(event.button);
            if (it != buttonActions_.end()) {
                it->second.executePress();
            }
            break;
        }

        case PendingEvent::WMAButtonRelease: {
            auto it = buttonActions_.find(event.button);
            if (it != buttonActions_.end()) {
                it->second.executeRelease();
            }
            break;
        }

        case PendingEvent::WMANone:
        default:
            break;
        }
    }

#ifdef WMA_ENABLE_GLFW
    void MouseListener::initializeGLFW(GLFWwindow* window)
    {
        if (!window) {
            throw InputException("Invalid GLFW window pointer");
        }

        glfwWindow_ = window;

        // Get initial cursor position
        f64 xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        currentPosition_ = MousePosition(xpos, ypos);
        lastPosition_ = currentPosition_;
        firstMouse_ = true;

        updateCursorStateGLFW();

        // Set callbacks
        glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);
        glfwSetCursorPosCallback(window, glfwCursorPosCallback);
        glfwSetScrollCallback(window, glfwScrollCallback);
    }

    void MouseListener::handleGLFWButtonEvent(i32 button, i32 action, i32 mods)
    {
        i32 unifiedButton = convertGLFWButton(button);

        if (buttonActions_.find(unifiedButton) != buttonActions_.end()) {
            PendingEvent event;
            event.button = unifiedButton;

            if (action == GLFW_PRESS) {
                event.type = PendingEvent::ButtonPress;
            } else if (action == GLFW_RELEASE) {
                event.type = PendingEvent::ButtonRelease;
            }

            if (event.type != PendingEvent::None) {
                processPendingEvents(event);
            }
        }
    }

    void MouseListener::handleGLFWPositionEvent(f64 xpos, f64 ypos)
    {
        if (firstMouse_) {
            lastPosition_ = MousePosition(xpos, ypos);
            firstMouse_ = false;
        }

        f64 deltaX = (xpos - lastPosition_.x) * sensitivity_;
        f64 deltaY = (lastPosition_.y - ypos) * sensitivity_;

        currentPosition_ = MousePosition(xpos, ypos, deltaX, deltaY);

        // Defer execution instead of immediate callback
        if (moveAction_.hasMoveAction()) {
            PendingEvent event;
            event.type = PendingEvent::Move;
            event.position = currentPosition_;
            processPendingEvents(event);
        }

        lastPosition_ = MousePosition(xpos, ypos);
    }

    void MouseListener::handleGLFWScrollEvent(f64 xoffset, f64 yoffset)
    {
        if (scrollAction_.hasScrollAction()) {
            PendingEvent event;
            event.type = PendingEvent::Scroll;
            event.scroll = MouseScroll(xoffset, yoffset);
            processPendingEvents(event);
        }
    }

    void MouseListener::glfwMouseButtonCallback(GLFWwindow* window, i32 button, i32 action, i32 mods)
    {
        auto* listener = getInstanceFromGLFW(window);
        if (listener)
            listener->handleGLFWButtonEvent(button, action, mods);
    }

    void MouseListener::glfwCursorPosCallback(GLFWwindow* window, f64 xpos, f64 ypos)
    {
        auto* listener = getInstanceFromGLFW(window);

        if (listener)
            listener->handleGLFWPositionEvent(xpos, ypos);
    }

    void MouseListener::glfwScrollCallback(GLFWwindow* window, f64 xoffset, f64 yoffset)
    {
        auto* listener = getInstanceFromGLFW(window);
        if (listener)
            listener->handleGLFWScrollEvent(xoffset, yoffset);
    }

    MouseListener* MouseListener::getInstanceFromGLFW(GLFWwindow* window)
    {
        if (!window) return nullptr;

        auto* userData = static_cast<GlfwUserData*>(glfwGetWindowUserPointer(window));

        if (!userData) return nullptr;

        if (!userData->mouseListener) return nullptr;

        return userData->mouseListener;
    }

    void MouseListener::updateCursorStateGLFW()
    {
        if (glfwWindow_) {
            if (cursorEnabled_) {
                glfwSetInputMode(glfwWindow_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
                glfwSetInputMode(glfwWindow_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        }
    }

    i32 MouseListener::convertGLFWButton(i32 glfwButton) const
    {
        switch (glfwButton) {
        case GLFW_MOUSE_BUTTON_LEFT:   return MouseButton::Left;
        case GLFW_MOUSE_BUTTON_RIGHT:  return MouseButton::Right;
        case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButton::Middle;
        case GLFW_MOUSE_BUTTON_4:      return MouseButton::Button4;
        case GLFW_MOUSE_BUTTON_5:      return MouseButton::Button5;
        case GLFW_MOUSE_BUTTON_6:      return MouseButton::Button6;
        case GLFW_MOUSE_BUTTON_7:      return MouseButton::Button7;
        case GLFW_MOUSE_BUTTON_8:      return MouseButton::Button8;
        default:                       return glfwButton;
        }
    }
#endif // WMA_ENABLE_GLFW

#ifdef WMA_ENABLE_SDL
    void MouseListener::initializeSDL(SDL_Window* window)
    {
        if (!window) {
            throw InputException("Invalid SDL window pointer");
        }

        sdlWindow_ = window;
        SDL_SetWindowData(window, "MouseListener", this);

        // Get initial cursor position
        i32 x, y;
        SDL_GetMouseState(&x, &y);
        currentPosition_ = WMAMousePosition(static_cast<f64>(x), static_cast<f64>(y));
        lastPosition_ = currentPosition_;

        updateCursorStateSDL();
    }

    void MouseListener::handleSDLEvent(const SDL_Event& event)
    {
        // SDL events are already processed in the main loop context,
        // so we can execute immediately (no deferred execution needed)
        switch (event.type) {
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            i32 unifiedButton = convertSDLButton(event.button.button);
            auto it = buttonActions_.find(unifiedButton);

            if (it != buttonActions_.end()) {
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    it->second.executePress();
                } else {
                    it->second.executeRelease();
                }
            }
            break;
        }

        case SDL_MOUSEMOTION: {
            if (firstMouse_) {
                lastPosition_ = WMAMousePosition(static_cast<f64>(event.motion.x), static_cast<f64>(event.motion.y));
                firstMouse_ = false;
            }

            f64 xpos = static_cast<f64>(event.motion.x);
            f64 ypos = static_cast<f64>(event.motion.y);
            f64 deltaX = (xpos - lastPosition_.x) * sensitivity_;
            f64 deltaY = (lastPosition_.y - ypos) * sensitivity_;

            currentPosition_ = WMAMousePosition(xpos, ypos, deltaX, deltaY);

            if (moveAction_.hasMoveAction()) {
                moveAction_.executeMove(currentPosition_);
            }

            lastPosition_ = WMAMousePosition(xpos, ypos);
            break;
        }

        case SDL_MOUSEWHEEL: {
            WMAMouseScroll scroll(static_cast<f64>(event.wheel.x), static_cast<f64>(event.wheel.y));

            if (scrollAction_.hasScrollAction()) {
                scrollAction_.executeScroll(scroll);
            }
            break;
        }

        default:
            break;
        }
    }

    void MouseListener::updateCursorStateSDL()
    {
        if (sdlWindow_) {
            if (cursorEnabled_) {
                SDL_ShowCursor(SDL_ENABLE);
                SDL_SetRelativeMouseMode(SDL_FALSE);
            } else {
                SDL_ShowCursor(SDL_DISABLE);
                SDL_SetRelativeMouseMode(SDL_TRUE);
            }
        }
    }

    i32 MouseListener::convertSDLButton(i32 sdlButton) const
    {
        switch (sdlButton) {
        case SDL_BUTTON_LEFT:   return MouseButton::WMALeft;
        case SDL_BUTTON_RIGHT:  return MouseButton::WMARight;
        case SDL_BUTTON_MIDDLE: return MouseButton::WMAMiddle;
        case SDL_BUTTON_X1:     return MouseButton::WMAButton4;
        case SDL_BUTTON_X2:     return MouseButton::WMAButton5;
        default:                return sdlButton;
        }
    }
#endif // WMA_ENABLE_SDL

} // namespace wma
