#ifdef WMA_ENABLE_SDL
#ifndef WMA_INPUT_SDL_KEYBOARD_LISTENER_HPP
#define WMA_INPUT_SDL_KEYBOARD_LISTENER_HPP

#include "wma/input/keyboard/KeyboardListener.hpp"

struct SDL_Window;
struct SDL_KeyboardEvent;

namespace wma {

/**
 * @brief SDL2-specific keyboard listener implementation
 */
class SDLKeyboardListener : public KeyboardListener {
public:
    SDLKeyboardListener();
    ~SDLKeyboardListener() override = default;

    /**
     * @brief Initialize the listener with an SDL window
     * @param window The SDL window to attach to
     */
    void initialize(SDL_Window* window);

    /**
     * @brief Handle SDL keyboard events
     * @param keyEvent The SDL keyboard event
     */
    void handleKeyEvent(const SDL_KeyboardEvent& keyEvent);

private:
    SDL_Window* sdlWindow_ = nullptr;
};

} // namespace wma

#endif // WMA_INPUT_SDL_KEYBOARD_LISTENER_HPP
#endif
