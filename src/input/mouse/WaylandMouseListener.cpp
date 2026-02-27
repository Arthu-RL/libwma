#include "wma/input/mouse/WaylandMouseListener.hpp"
#include "wma/exceptions/WMAException.hpp"
#include "wma/core/Types.hpp"

#include <linux/input-event-codes.h>

namespace wma {

// Static pointer listener implementation
const wl_pointer_listener WaylandMouseListener::pointerListener_ = {
    .enter = handleEnterCallback,
    .leave = handleLeaveCallback,
    .motion = handleMotionCallback,
    .button = handleButtonCallback,
    .axis = handleAxisCallback,
    .frame = handleFrameCallback,
    .axis_source = handleAxisSourceCallback,
    .axis_stop = handleAxisStopCallback,
    .axis_discrete = handleAxisDiscreteCallback
};

WaylandMouseListener::WaylandMouseListener()
    : MouseListener()
    , pointer_(nullptr)
    , cursorSurface_(nullptr)
{
}

WaylandMouseListener::~WaylandMouseListener()
{
    if (cursorSurface_) {
        wl_surface_destroy(cursorSurface_);
        cursorSurface_ = nullptr;
    }

    if (pointer_) {
        wl_pointer_destroy(pointer_);
        pointer_ = nullptr;
    }
}

void WaylandMouseListener::initialize(wl_pointer* pointer)
{
    if (!pointer) {
        throw InputException("Invalid Wayland pointer");
    }

    pointer_ = pointer;
    wl_pointer_add_listener(pointer_, &pointerListener_, this);
}

void WaylandMouseListener::handleEnter(uint32_t serial, wl_surface* surface,
                                       wl_fixed_t x, wl_fixed_t y)
{
    // Pointer entered our surface
    f64 xpos = wl_fixed_to_double(x);
    f64 ypos = wl_fixed_to_double(y);

    currentPosition_ = WMAMousePosition(xpos, ypos);
    lastPosition_ = currentPosition_;
    firstMouse_ = true;
}

void WaylandMouseListener::handleLeave(uint32_t serial, wl_surface* surface)
{
    // Pointer left our surface
    // Can be used to reset state or hide cursor
}

void WaylandMouseListener::handleMotion(uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
    f64 xpos = wl_fixed_to_double(x);
    f64 ypos = wl_fixed_to_double(y);

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
}

void WaylandMouseListener::handleButton(uint32_t serial, uint32_t time,
                                        uint32_t button, uint32_t state)
{
    i32 unifiedButton = convertButton(button);
    auto it = buttonActions_.find(unifiedButton);

    if (it != buttonActions_.end()) {
        if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
            it->second.executePress();
        } else if (state == WL_POINTER_BUTTON_STATE_RELEASED) {
            it->second.executeRelease();
        }
    }
}

void WaylandMouseListener::handleAxis(uint32_t time, uint32_t axis, wl_fixed_t value)
{
    // Axis events for scroll wheel
    f64 scrollValue = wl_fixed_to_double(value);

    if (scrollAction_.hasScrollAction()) {
        WMAMouseScroll scroll;

        if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
            // Normalize scroll value (Wayland uses pixels, we want units)
            scroll.yOffset = scrollValue > 0 ? -1.0 : 1.0;
        } else if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
            scroll.xOffset = scrollValue > 0 ? 1.0 : -1.0;
        }

        scrollAction_.executeScroll(scroll);
    }
}

void WaylandMouseListener::handleFrame()
{
    // Frame event groups pointer events together
    // Can be used for batching updates
}

void WaylandMouseListener::handleAxisSource(uint32_t axis_source)
{
    // Indicates the source of axis events (wheel, finger, continuous)
}

void WaylandMouseListener::handleAxisStop(uint32_t time, uint32_t axis)
{
    // Indicates that axis scrolling has stopped
}

void WaylandMouseListener::handleAxisDiscrete(uint32_t axis, int32_t discrete)
{
    // Discrete scroll steps (e.g., mouse wheel clicks)
    if (scrollAction_.hasScrollAction()) {
        WMAMouseScroll scroll;

        if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
            scroll.yOffset = static_cast<f64>(discrete);
        } else if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
            scroll.xOffset = static_cast<f64>(discrete);
        }

        scrollAction_.executeScroll(scroll);
    }
}

void WaylandMouseListener::updateCursorState()
{
    if (!pointer_) return;

    if (cursorEnabled_) {
        // Show cursor - would typically load cursor theme and set cursor
        // For now, just set to default arrow cursor
        // wl_pointer_set_cursor(pointer_, serial, cursorSurface_, hotspot_x, hotspot_y);
    } else {
        // Hide cursor by setting null cursor surface
        wl_pointer_set_cursor(pointer_, 0, nullptr, 0, 0);
    }
}

i32 WaylandMouseListener::convertButton(uint32_t waylandButton) const
{
    // Wayland uses Linux input event codes
    switch (waylandButton) {
    case BTN_LEFT:   return MouseButton::WMALeft;
    case BTN_RIGHT:  return MouseButton::WMARight;
    case BTN_MIDDLE: return MouseButton::WMAMiddle;
    case BTN_SIDE:   return MouseButton::WMAButton4;
    case BTN_EXTRA:  return MouseButton::WMAButton5;
    default:         return waylandButton;
    }
}

// Static callback implementations
void WaylandMouseListener::handleEnterCallback(void* data, wl_pointer* pointer,
                                               uint32_t serial, wl_surface* surface,
                                               wl_fixed_t x, wl_fixed_t y)
{
    auto* listener = static_cast<WaylandMouseListener*>(data);
    if (listener) {
        listener->handleEnter(serial, surface, x, y);
    }
}

void WaylandMouseListener::handleLeaveCallback(void* data, wl_pointer* pointer,
                                               uint32_t serial, wl_surface* surface)
{
    auto* listener = static_cast<WaylandMouseListener*>(data);
    if (listener) {
        listener->handleLeave(serial, surface);
    }
}

void WaylandMouseListener::handleMotionCallback(void* data, wl_pointer* pointer,
                                                uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
    auto* listener = static_cast<WaylandMouseListener*>(data);
    if (listener) {
        listener->handleMotion(time, x, y);
    }
}

void WaylandMouseListener::handleButtonCallback(void* data, wl_pointer* pointer,
                                                uint32_t serial, uint32_t time,
                                                uint32_t button, uint32_t state)
{
    auto* listener = static_cast<WaylandMouseListener*>(data);
    if (listener) {
        listener->handleButton(serial, time, button, state);
    }
}

void WaylandMouseListener::handleAxisCallback(void* data, wl_pointer* pointer,
                                              uint32_t time, uint32_t axis, wl_fixed_t value)
{
    auto* listener = static_cast<WaylandMouseListener*>(data);
    if (listener) {
        listener->handleAxis(time, axis, value);
    }
}

void WaylandMouseListener::handleFrameCallback(void* data, wl_pointer* pointer)
{
    auto* listener = static_cast<WaylandMouseListener*>(data);
    if (listener) {
        listener->handleFrame();
    }
}

void WaylandMouseListener::handleAxisSourceCallback(void* data, wl_pointer* pointer,
                                                    uint32_t axis_source)
{
    auto* listener = static_cast<WaylandMouseListener*>(data);
    if (listener) {
        listener->handleAxisSource(axis_source);
    }
}

void WaylandMouseListener::handleAxisStopCallback(void* data, wl_pointer* pointer,
                                                  uint32_t time, uint32_t axis)
{
    auto* listener = static_cast<WaylandMouseListener*>(data);
    if (listener) {
        listener->handleAxisStop(time, axis);
    }
}

void WaylandMouseListener::handleAxisDiscreteCallback(void* data, wl_pointer* pointer,
                                                      uint32_t axis, int32_t discrete)
{
    auto* listener = static_cast<WaylandMouseListener*>(data);
    if (listener) {
        listener->handleAxisDiscrete(axis, discrete);
    }
}

} // namespace wma
