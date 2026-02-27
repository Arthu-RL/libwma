#ifndef WMA_INPUT_MOUSE_LISTENER_HPP
#define WMA_INPUT_MOUSE_LISTENER_HPP

#include "MouseAction.hpp"
#include <unordered_map>

namespace wma {

struct PendingEvent {
    enum WMAType {
        WMANone = 0,
        WMAMove,
        WMAScroll,
        WMAButtonPress,
        WMAButtonRelease
    } type = WMANone;

    WMAMousePosition position{};
    WMAMouseScroll scroll{};
    i32 button = -1;

    PendingEvent() = default;
    explicit PendingEvent(WMAType t) : type(t) {}
    PendingEvent(WMAType t, const WMAMousePosition& pos) : type(t), position(pos) {}
    PendingEvent(WMAType t, const WMAMouseScroll& s) : type(t), scroll(s) {}
    PendingEvent(WMAType t, i32 btn) : type(t), button(btn) {}
};

class MouseListener {
public:
    MouseListener();
    virtual ~MouseListener();

    // Action management
    void addButtonAction(i32 button, MouseAction action);
    void removeButtonAction(i32 button);
    void setMoveAction(MouseAction action);
    void setScrollAction(MouseAction action);
    void clearAllActions();
    bool hasButtonAction(i32 button) const;

    // State queries
    WMAMousePosition getCurrentPosition() const;
    void setCursorEnabled(bool enabled);
    bool isCursorEnabled() const;
    void setSensitivity(f64 sensitivity);
    f64 getSensitivity() const;

    // Event processing
    void processPendingEvents(const PendingEvent& event);

protected:
    // Platform-specific methods to be overridden
    virtual void updateCursorState() = 0;

    // Core state
    std::unordered_map<i32, MouseAction> buttonActions_;
    MouseAction moveAction_;
    MouseAction scrollAction_;

    WMAMousePosition currentPosition_;
    WMAMousePosition lastPosition_;

    bool cursorEnabled_ = true;
    f64 sensitivity_ = 1.0;
    bool firstMouse_ = true;
};

} // namespace wma

#endif // WMA_INPUT_MOUSE_LISTENER_HPP
