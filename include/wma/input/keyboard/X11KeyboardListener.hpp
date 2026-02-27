#ifdef WMA_ENABLE_X11
#ifndef WMA_INPUT_X11_KEYBOARD_LISTENER_HPP
#define WMA_INPUT_X11_KEYBOARD_LISTENER_HPP

#include "wma/input/keyboard/KeyboardListener.hpp"
#include <X11/Xlib.h>
#include <X11/keysym.h>

namespace wma {

/**
 * @brief X11-specific keyboard listener implementation
 */
class X11KeyboardListener : public KeyboardListener {
public:
    X11KeyboardListener();
    ~X11KeyboardListener() override = default;

    /**
     * @brief Initialize the listener with an X11 display
     * @param display The X11 display to attach to
     */
    void initialize(Display* display);

    /**
     * @brief Handle X11 keyboard events
     * @param x11Key The X11 KeySym
     * @param xKeyEvent The X11 key event
     */
    void handleKeyEvent(KeySym x11Key, const XKeyEvent& xKeyEvent);

private:
    Display* display_ = nullptr;
};

} // namespace wma

#endif // WMA_INPUT_X11_KEYBOARD_LISTENER_HPP
#endif
