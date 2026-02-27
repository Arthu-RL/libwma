#ifdef WMA_ENABLE_SDL
#include "wma/input/keyboard/SDLKeyboardListener.hpp"
#include "wma/input/keyboard/Keys.h"
#include "wma/exceptions/WMAException.hpp"

#include <SDL2/SDL.h>

namespace wma {

SDLKeyboardListener::SDLKeyboardListener()
    : KeyboardListener()
    , sdlWindow_(nullptr)
{
}

void SDLKeyboardListener::initialize(SDL_Window* window)
{
    if (!window) {
        throw InputException("Invalid SDL window pointer");
    }

    sdlWindow_ = window;
    SDL_SetWindowData(window, "KeyboardListener", this);
}

void SDLKeyboardListener::handleKeyEvent(const SDL_KeyboardEvent& keyEvent)
{
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

} // namespace wma
#endif
