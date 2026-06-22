#ifdef WMA_ENABLE_SDL
#include "wma/backends/sdl/SDLKeyboardListener.hpp"
#include "wma/input/keyboard/Keys.h"
#include "wma/exceptions/WMAException.hpp"

#include <SDL3/SDL.h>

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
    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    SDL_SetPointerProperty(props, "KeyboardListener", this);
}

void SDLKeyboardListener::handleKeyEvent(const SDL_KeyboardEvent& keyEvent)
{
    const Key mappedKey = mapSDLKey(keyEvent.key);
    if (keyEvent.type == SDL_EVENT_KEY_DOWN) {
        dispatchKeyPress(mappedKey);
    } else if (keyEvent.type == SDL_EVENT_KEY_UP) {
        dispatchKeyRelease(mappedKey);
    }
}

} // namespace wma
#endif
