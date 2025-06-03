#include "wma/input/MouseListener.hpp"
#include "wma/exceptions/WMAException.hpp"
#include "wma/input/KeyboardListener.hpp"

#ifdef WMA_ENABLE_GLFW
    #include <GLFW/glfw3.h>
#endif

#ifdef WMA_ENABLE_SDL
    #include <SDL2/SDL.h>
#endif

namespace wma {

    void MouseListener::addButtonAction(i32 button, MouseAction action) {
        buttonActions_[button] = std::move(action);
    }

    void MouseListener::removeButtonAction(i32 button) {
        buttonActions_.erase(button);
    }

    void MouseListener::setMoveAction(MouseAction action) {
        moveAction_ = std::move(action);
    }

    void MouseListener::setScrollAction(MouseAction action) {
        scrollAction_ = std::move(action);
    }

    void MouseListener::clearAllActions() {
        buttonActions_.clear();
        moveAction_ = MouseAction{};
        scrollAction_ = MouseAction{};
    }

    bool MouseListener::hasButtonAction(i32 button) const {
        return buttonActions_.find(button) != buttonActions_.end();
    }

    MousePosition MouseListener::getCurrentPosition() const {
        return currentPosition_;
    }

    void MouseListener::setCursorEnabled(bool enabled) {
        if (cursorEnabled_ != enabled) {
            cursorEnabled_ = enabled;
            updateCursorStateGLFW();
            updateCursorStateSDL();
        }
    }

    bool MouseListener::isCursorEnabled() const {
        return cursorEnabled_;
    }

    void MouseListener::setSensitivity(f64 sensitivity) {
        sensitivity_ = sensitivity;
    }

    f64 MouseListener::getSensitivity() const {
        return sensitivity_;
    }

#ifdef WMA_ENABLE_GLFW
    void MouseListener::initializeGLFW(GLFWwindow* window) {
        if (!window) {
            throw InputException("Invalid GLFW window pointer");
        }
        
        glfwWindow_ = window;
        
        // Set callbacks
        glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);
        glfwSetCursorPosCallback(window, glfwCursorPosCallback);
        glfwSetScrollCallback(window, glfwScrollCallback);
        
        // Get initial cursor position
        f64 xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        currentPosition_ = MousePosition(xpos, ypos);
        lastPosition_ = currentPosition_;
        
        updateCursorStateGLFW();
    }

    void MouseListener::handleGLFWButtonEvent(i32 button, i32 action, i32 mods) {
        i32 unifiedButton = convertGLFWButton(button);
        auto it = buttonActions_.find(unifiedButton);
        
        if (it != buttonActions_.end()) {
            if (action == GLFW_PRESS) {
                it->second.executePress();
            } else if (action == GLFW_RELEASE) {
                it->second.executeRelease();
            }
        }
    }

    void MouseListener::handleGLFWPositionEvent(f64 xpos, f64 ypos) {
        if (firstMouse_) {
            lastPosition_ = MousePosition(xpos, ypos);
            firstMouse_ = false;
        }
        
        f64 deltaX = (xpos - lastPosition_.x) * sensitivity_;
        f64 deltaY = (lastPosition_.y - ypos) * sensitivity_; // Reversed since y-coordinates go from bottom to top
        
        currentPosition_ = MousePosition(xpos, ypos, deltaX, deltaY);
        
        if (moveAction_.hasMoveAction()) {
            moveAction_.executeMove(currentPosition_);
        }
        
        lastPosition_ = MousePosition(xpos, ypos);
    }

    void MouseListener::handleGLFWScrollEvent(f64 xoffset, f64 yoffset) {
        MouseScroll scroll(xoffset, yoffset);
        
        if (scrollAction_.hasScrollAction()) {
            scrollAction_.executeScroll(scroll);
        }
    }

    void MouseListener::glfwMouseButtonCallback(GLFWwindow* window, i32 button, i32 action, i32 mods) {
        auto* listener = getInstanceFromGLFW(window);
        if (listener) {
            listener->handleGLFWButtonEvent(button, action, mods);
        }
    }

    void MouseListener::glfwCursorPosCallback(GLFWwindow* window, f64 xpos, f64 ypos) {
        auto* listener = getInstanceFromGLFW(window);
        if (listener) {
            listener->handleGLFWPositionEvent(xpos, ypos);
        }
    }

    void MouseListener::glfwScrollCallback(GLFWwindow* window, f64 xoffset, f64 yoffset) {
        auto* listener = getInstanceFromGLFW(window);
        if (listener) {
            listener->handleGLFWScrollEvent(xoffset, yoffset);
        }
    }

    MouseListener* MouseListener::getInstanceFromGLFW(GLFWwindow* window) {
        // Get the user data structure that contains all instances
        struct GlfwUserData {
            void* windowManager;
            KeyboardListener* keyboardListener;
            MouseListener* mouseListener;
        };
        
        auto* userData = static_cast<GlfwUserData*>(glfwGetWindowUserPointer(window));
        return userData ? userData->mouseListener : nullptr;
    }

    void MouseListener::updateCursorStateGLFW() {
        if (glfwWindow_) {
            if (cursorEnabled_) {
                glfwSetInputMode(glfwWindow_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
                glfwSetInputMode(glfwWindow_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        }
    }

    i32 MouseListener::convertGLFWButton(i32 glfwButton) const {
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
    void MouseListener::initializeSDL(SDL_Window* window) {
        if (!window) {
            throw InputException("Invalid SDL window pointer");
        }
        
        sdlWindow_ = window;
        SDL_SetWindowData(window, "MouseListener", this);
        
        // Get initial cursor position
        i32 x, y;
        SDL_GetMouseState(&x, &y);
        currentPosition_ = MousePosition(static_cast<f64>(x), static_cast<f64>(y));
        lastPosition_ = currentPosition_;
        
        updateCursorStateSDL();
    }

    void MouseListener::handleSDLEvent(const SDL_Event& event) {
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
                    lastPosition_ = MousePosition(static_cast<f64>(event.motion.x), static_cast<f64>(event.motion.y));
                    firstMouse_ = false;
                }
                
                f64 xpos = static_cast<f64>(event.motion.x);
                f64 ypos = static_cast<f64>(event.motion.y);
                f64 deltaX = (xpos - lastPosition_.x) * sensitivity_;
                f64 deltaY = (lastPosition_.y - ypos) * sensitivity_;
                
                currentPosition_ = MousePosition(xpos, ypos, deltaX, deltaY);
                
                if (moveAction_.hasMoveAction()) {
                    moveAction_.executeMove(currentPosition_);
                }
                
                lastPosition_ = MousePosition(xpos, ypos);
                break;
            }
            
            case SDL_MOUSEWHEEL: {
                MouseScroll scroll(static_cast<f64>(event.wheel.x), static_cast<f64>(event.wheel.y));
                
                if (scrollAction_.hasScrollAction()) {
                    scrollAction_.executeScroll(scroll);
                }
                break;
            }
            
            default:
                break;
        }
    }

    void MouseListener::updateCursorStateSDL() {
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

    i32 MouseListener::convertSDLButton(i32 sdlButton) const {
        switch (sdlButton) {
            case SDL_BUTTON_LEFT:   return MouseButton::Left;
            case SDL_BUTTON_RIGHT:  return MouseButton::Right;
            case SDL_BUTTON_MIDDLE: return MouseButton::Middle;
            case SDL_BUTTON_X1:     return MouseButton::Button4;
            case SDL_BUTTON_X2:     return MouseButton::Button5;
            default:                return sdlButton;
        }
    }
#endif // WMA_ENABLE_SDL

} // namespace wma
