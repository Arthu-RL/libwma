#ifdef WMA_ENABLE_SDL
#include "wma/input/mouse/SDLMouseListener.hpp"
#include "wma/exceptions/WMAException.hpp"
#include "wma/core/Types.hpp"

#include <SDL2/SDL.h>

namespace wma {

SDLMouseListener::SDLMouseListener()
    : MouseListener()
    , sdlWindow_(nullptr)
{
}

void SDLMouseListener::initialize(SDL_Window* window)
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
    firstMouse_ = true;

    updateCursorState();
}

void SDLMouseListener::handleEvent(const SDL_Event& event)
{
    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP: {
        i32 unifiedButton = convertButton(event.button.button);
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
            lastPosition_ = WMAMousePosition(
                static_cast<f64>(event.motion.x),
                static_cast<f64>(event.motion.y)
            );
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
        WMAMouseScroll scroll(
            static_cast<f64>(event.wheel.x),
            static_cast<f64>(event.wheel.y)
        );

        if (scrollAction_.hasScrollAction()) {
            scrollAction_.executeScroll(scroll);
        }
        break;
    }

    default:
        break;
    }
}

void SDLMouseListener::updateCursorState()
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

i32 SDLMouseListener::convertButton(i32 sdlButton) const
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

} // namespace wma

#endif
