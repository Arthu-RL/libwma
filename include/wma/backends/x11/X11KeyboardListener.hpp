#ifndef WMA_BACKENDS_X11_KEYBOARD_LISTENER_HPP
#define WMA_BACKENDS_X11_KEYBOARD_LISTENER_HPP

#include "wma/input/keyboard/KeyboardListener.hpp"
#include <X11/Xlib.h>
#include <X11/keysym.h>

namespace wma {

class X11KeyboardListener : public KeyboardListener {
public:
    X11KeyboardListener();
    ~X11KeyboardListener() override = default;

    void initialize(Display* display);
    void handleKeyEvent(KeySym x11Key, const XKeyEvent& xKeyEvent);

private:
    Display* display_ = nullptr;
};

} // namespace wma

#endif // WMA_BACKENDS_X11_KEYBOARD_LISTENER_HPP
