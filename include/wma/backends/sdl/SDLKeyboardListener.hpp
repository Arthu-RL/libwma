#ifndef WMA_BACKENDS_SDL_KEYBOARD_LISTENER_HPP
#define WMA_BACKENDS_SDL_KEYBOARD_LISTENER_HPP

#include "wma/input/keyboard/KeyboardListener.hpp"

struct SDL_Window;
struct SDL_KeyboardEvent;

namespace wma {

class SDLKeyboardListener : public KeyboardListener {
public:
    SDLKeyboardListener();
    ~SDLKeyboardListener() override = default;

    void initialize(SDL_Window* window);
    void handleKeyEvent(const SDL_KeyboardEvent& keyEvent);

private:
    SDL_Window* sdlWindow_ = nullptr;
};

} // namespace wma

#endif // WMA_BACKENDS_SDL_KEYBOARD_LISTENER_HPP
