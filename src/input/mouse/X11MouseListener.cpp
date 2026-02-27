#include "wma/input/mouse/X11MouseListener.hpp"
#include "wma/exceptions/WMAException.hpp"
#include "wma/core/Types.hpp"

namespace wma {

X11MouseListener::X11MouseListener()
    : MouseListener()
    , display_(nullptr)
{
}

void X11MouseListener::initialize(Display* display, Window window)
{
    if (!display) {
        throw InputException("Invalid X11 Display pointer");
    }

    display_ = display;
    x11Window_ = window;

    invisibleCursor_ = createInvisibleCursor(display, window);

    // Get initial cursor position
    Window root, child;
    int rootX, rootY, x, y;
    unsigned int mask;

    XQueryPointer(
        display,
        DefaultRootWindow(display),
        &root,
        &child,
        &rootX, &rootY,
        &x, &y,
        &mask
    );

    currentPosition_ = WMAMousePosition(static_cast<f64>(x), static_cast<f64>(y));
    lastPosition_ = currentPosition_;
    firstMouse_ = true;
}

void X11MouseListener::handleEvent(const XEvent* event)
{
    if (!event) return;

    switch (event->type) {
    case ButtonPress: {
        const int btn = event->xbutton.button;

        // Handle scroll wheel
        if (btn == Button4 || btn == Button5) {
            f64 scrollX = 0.0;
            f64 scrollY = 0.0;

            switch (btn) {
            case Button4: scrollY = +1.0; break; // wheel up
            case Button5: scrollY = -1.0; break; // wheel down
            }

            WMAMouseScroll scroll(scrollX, scrollY);

            if (scrollAction_.hasScrollAction()) {
                scrollAction_.executeScroll(scroll);
            }
            break;
        }

        // Handle normal mouse buttons
        i32 unifiedButton = convertButton(btn);
        auto it = buttonActions_.find(unifiedButton);
        if (it != buttonActions_.end()) {
            it->second.executePress();
        }
        break;
    }

    case ButtonRelease: {
        const int btn = event->xbutton.button;

        // Scroll wheel doesn't have release events
        if (btn >= Button4) {
            break;
        }

        i32 unifiedButton = convertButton(btn);
        auto it = buttonActions_.find(unifiedButton);
        if (it != buttonActions_.end()) {
            it->second.executeRelease();
        }
        break;
    }

    case MotionNotify: {
        f64 xpos = static_cast<f64>(event->xmotion.x);
        f64 ypos = static_cast<f64>(event->xmotion.y);

        if (firstMouse_) {
            lastPosition_ = WMAMousePosition(xpos, ypos);
            firstMouse_ = false;
        }

        f64 deltaX = (xpos - lastPosition_.x) * sensitivity_;
        f64 deltaY = (lastPosition_.y - ypos) * sensitivity_;

        currentPosition_ = WMAMousePosition(xpos, ypos, deltaX, deltaY);

        if (moveAction_.hasMoveAction()) {
            moveAction_.executeMove(currentPosition_);
        }

        lastPosition_ = WMAMousePosition(xpos, ypos);
        break;
    }

    default:
        break;
    }
}

Cursor X11MouseListener::createInvisibleCursor(Display* display, Window window)
{
    Pixmap bmNo;
    XColor black;
    static char noData[] = { 0,0,0,0,0,0,0,0 };

    black.red = black.green = black.blue = 0;

    bmNo = XCreateBitmapFromData(display, window, noData, 8, 8);
    return XCreatePixmapCursor(display, bmNo, bmNo, &black, &black, 0, 0);
}

void X11MouseListener::updateCursorState()
{
    if (!display_ || !x11Window_)
        return;

    if (cursorEnabled_) {
        XUndefineCursor(display_, x11Window_);
    } else {
        XDefineCursor(display_, x11Window_, invisibleCursor_);
    }

    XFlush(display_);
}

i32 X11MouseListener::convertButton(i32 x11Button) const
{
    switch (x11Button) {
    case Button1:   return MouseButton::WMALeft;
    case Button2:   return MouseButton::WMAMiddle;
    case Button3:   return MouseButton::WMARight;
    default:        return x11Button;
    }
}

} // namespace wma
