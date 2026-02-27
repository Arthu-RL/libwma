#include "wma/input/mouse/MouseListener.hpp"

namespace wma {

MouseListener::MouseListener()
    : currentPosition_{}
    , lastPosition_{}
{
    moveAction_ = MouseAction(
        [](const WMAMousePosition& pos) {
            // handle move
        }
    );
    scrollAction_ = MouseAction(
        [](const WMAMouseScroll& scroll) {
            // handle scroll
        }
    );
}

MouseListener::~MouseListener() = default;

void MouseListener::addButtonAction(i32 button, MouseAction action)
{
    buttonActions_[button] = std::move(action);
}

void MouseListener::removeButtonAction(i32 button)
{
    buttonActions_.erase(button);
}

void MouseListener::setMoveAction(MouseAction action)
{
    moveAction_ = std::move(action);
}

void MouseListener::setScrollAction(MouseAction action)
{
    scrollAction_ = std::move(action);
}

void MouseListener::clearAllActions()
{
    buttonActions_.clear();
    moveAction_ = MouseAction(
        [](const WMAMousePosition& pos) {
            // handle move
        }
    );
    scrollAction_ = MouseAction(
        [](const WMAMouseScroll& scroll) {
            // handle scroll
        }
    );
}

bool MouseListener::hasButtonAction(i32 button) const
{
    return buttonActions_.find(button) != buttonActions_.end();
}

WMAMousePosition MouseListener::getCurrentPosition() const
{
    return currentPosition_;
}

void MouseListener::setCursorEnabled(bool enabled)
{
    if (cursorEnabled_ != enabled) {
        cursorEnabled_ = enabled;
        updateCursorState();
    }
}

bool MouseListener::isCursorEnabled() const
{
    return cursorEnabled_;
}

void MouseListener::setSensitivity(f64 sensitivity)
{
    sensitivity_ = sensitivity;
}

f64 MouseListener::getSensitivity() const
{
    return sensitivity_;
}

void MouseListener::processPendingEvents(const PendingEvent& event)
{
    switch (event.type) {
    case PendingEvent::WMAMove:
        if (moveAction_.hasMoveAction()) {
            moveAction_.executeMove(event.position);
        }
        break;

    case PendingEvent::WMAScroll:
        if (scrollAction_.hasScrollAction()) {
            scrollAction_.executeScroll(event.scroll);
        }
        break;

    case PendingEvent::WMAButtonPress: {
        auto it = buttonActions_.find(event.button);
        if (it != buttonActions_.end()) {
            it->second.executePress();
        }
        break;
    }

    case PendingEvent::WMAButtonRelease: {
        auto it = buttonActions_.find(event.button);
        if (it != buttonActions_.end()) {
            it->second.executeRelease();
        }
        break;
    }

    case PendingEvent::WMANone:
    default:
        break;
    }
}

} // namespace wma
