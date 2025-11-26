#include "wma/input/KeyboardListener.hpp"
#include "wma/input/Keys.h"
#include "wma/exceptions/WMAException.hpp"

#ifdef WMA_ENABLE_GLFW
    #include <GLFW/glfw3.h>
#endif

#ifdef WMA_ENABLE_SDL
    #include <SDL2/SDL.h>
#endif

namespace wma {

    void KeyboardListener::addKeyAction(i32 key, KeyAction action) {
        keyActions_[key] = std::move(action);
    }

    void KeyboardListener::removeKeyAction(i32 key) {
        keyActions_.erase(key);
    }

    void KeyboardListener::clearKeyActions() {
        keyActions_.clear();
    }

    bool KeyboardListener::hasKeyAction(i32 key) const {
        return keyActions_.find(key) != keyActions_.end();
    }

#ifdef WMA_ENABLE_GLFW
    void KeyboardListener::initializeGLFW(GLFWwindow* window) {
        if (!window) {
            throw InputException("Invalid GLFW window pointer");
        }
        
        glfwWindow_ = window;
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, glfwKeyCallback);
    }

    void KeyboardListener::handleGLFWKeyEvent(i32 key, i32 action) {
        Key mappedKey = mapGLFWKey(key);
        auto it = keyActions_.find(static_cast<i32>(mappedKey));
        if (it != keyActions_.end()) {
            if (action == GLFW_PRESS) {
                it->second.executePress();
            } else if (action == GLFW_RELEASE) {
                it->second.executeRelease();
            }
        }
    }

    void KeyboardListener::glfwKeyCallback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods) {
        auto* listener = getInstanceFromGLFW(window);
        if (listener) {
            listener->handleGLFWKeyEvent(key, action);
        }
    }

    KeyboardListener* KeyboardListener::getInstanceFromGLFW(GLFWwindow* window) {
        return static_cast<KeyboardListener*>(glfwGetWindowUserPointer(window));
    }
#endif // WMA_ENABLE_GLFW

#ifdef WMA_ENABLE_SDL
    void KeyboardListener::initializeSDL(SDL_Window* window) {
        if (!window) {
            throw InputException("Invalid SDL window pointer");
        }
        
        sdlWindow_ = window;
        SDL_SetWindowData(window, "KeyboardListener", this);
    }

    void KeyboardListener::handleSDLKeyEvent(const SDL_KeyboardEvent& keyEvent) {
        Key mappedKey = mapSDLKey(keyEvent.keysym.sym);
        auto it = keyActions_.find(static_cast<i32>(mappedKey));
        if (it != keyActions_.end()) {
            if (keyEvent.type == SDL_KEYDOWN) {
                it->second.executePress();
            } else if (keyEvent.type == SDL_KEYUP) {
                it->second.executeRelease();
            }
        }
    }
#endif // WMA_ENABLE_SDL

#ifdef WMA_ENABLE_X11
void KeyboardListener::handleX11KeyEvent(KeySym x11Key, const XKeyEvent& xKeyEvent)
{
    Key mappedKey = mapX11Key(x11Key);
    auto it = keyActions_.find(static_cast<i32>(mappedKey));
    if (it != keyActions_.end()) {
        if (xKeyEvent.type == KeyPress) {
            it->second.executePress();
        } else if (xKeyEvent.type == KeyRelease) {
            it->second.executeRelease();
        }
    }
}
#endif

} // namespace wma
