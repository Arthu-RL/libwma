#ifdef WMA_ENABLE_X11
#include "wma/input/keyboard/X11KeyboardListener.hpp"
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

} // namespace wma
#endif
