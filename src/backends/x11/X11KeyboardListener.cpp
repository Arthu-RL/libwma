#ifdef WMA_ENABLE_X11
#include "wma/backends/x11/X11KeyboardListener.hpp"
#include "wma/input/keyboard/Keys.h"
#include "wma/exceptions/WMAException.hpp"

namespace wma {

X11KeyboardListener::X11KeyboardListener()
    : KeyboardListener()
    , display_(nullptr)
{
}

void X11KeyboardListener::initialize(Display* display)
{
    if (!display) {
        throw InputException("Invalid X11 Display pointer");
    }
    display_ = display;
}

void X11KeyboardListener::handleKeyEvent(KeySym x11Key, const XKeyEvent& xKeyEvent)
{
    const Key mappedKey = mapX11Key(x11Key);
    if (xKeyEvent.type == KeyPress) {
        dispatchKeyPress(mappedKey);
    } else if (xKeyEvent.type == KeyRelease) {
        dispatchKeyRelease(mappedKey);
    }
}

} // namespace wma
#endif
