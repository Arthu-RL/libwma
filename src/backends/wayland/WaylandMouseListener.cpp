#include "wma/backends/wayland/WaylandMouseListener.hpp"
#include "wma/exceptions/WMAException.hpp"
#include "wma/core/Types.hpp"

#include <linux/input-event-codes.h>

namespace wma {

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

void WaylandMouseListener::handleEnter(u32, wl_surface*,
                                       wl_fixed_t x, wl_fixed_t y)
{
    f64 xpos = wl_fixed_to_double(x);
    f64 ypos = wl_fixed_to_double(y);
    currentPosition_ = WMAMousePosition(xpos, ypos);
    lastPosition_ = currentPosition_;
    firstMouse_ = true;
}

void WaylandMouseListener::handleLeave(u32, wl_surface*)
{
}

void WaylandMouseListener::handleMotion(u32, wl_fixed_t x, wl_fixed_t y)
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
    dispatchMove(currentPosition_);
    lastPosition_ = WMAMousePosition(xpos, ypos);
}

void WaylandMouseListener::handleButton(u32, u32,
                                        u32 button, u32 state)
{
    const i32 unifiedButton = convertButton(button);
    if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
        dispatchButtonPress(unifiedButton);
    } else if (state == WL_POINTER_BUTTON_STATE_RELEASED) {
        dispatchButtonRelease(unifiedButton);
    }
}

void WaylandMouseListener::handleAxis(u32, u32 axis, wl_fixed_t value)
{
    const f64 scrollValue = wl_fixed_to_double(value);
    WMAMouseScroll scroll;

    if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
        scroll.yOffset = scrollValue > 0 ? -1.0 : 1.0;
    } else if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
        scroll.xOffset = scrollValue > 0 ? 1.0 : -1.0;
    }
    dispatchScroll(scroll);
}

void WaylandMouseListener::handleFrame() {}
void WaylandMouseListener::handleAxisSource(u32) {}
void WaylandMouseListener::handleAxisStop(u32, u32) {}

void WaylandMouseListener::handleAxisDiscrete(u32 axis, i32 discrete)
{
    WMAMouseScroll scroll;
    if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
        scroll.yOffset = static_cast<f64>(discrete);
    } else if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
        scroll.xOffset = static_cast<f64>(discrete);
    }
    dispatchScroll(scroll);
}

void WaylandMouseListener::updateCursorState()
{
    if (!pointer_) return;
    if (!cursorEnabled_) {
        wl_pointer_set_cursor(pointer_, 0, nullptr, 0, 0);
    }
}

i32 WaylandMouseListener::convertButton(u32 waylandButton) const
{
    switch (waylandButton) {
    case BTN_LEFT:   return MouseButton::WMALeft;
    case BTN_RIGHT:  return MouseButton::WMARight;
    case BTN_MIDDLE: return MouseButton::WMAMiddle;
    case BTN_SIDE:   return MouseButton::WMAButton4;
    case BTN_EXTRA:  return MouseButton::WMAButton5;
    default:         return static_cast<i32>(waylandButton);
    }
}

void WaylandMouseListener::handleEnterCallback(void* data, wl_pointer*,
                                               u32 serial, wl_surface* surface,
                                               wl_fixed_t x, wl_fixed_t y)
{
    auto* l = static_cast<WaylandMouseListener*>(data);
    if (l) l->handleEnter(serial, surface, x, y);
}

void WaylandMouseListener::handleLeaveCallback(void* data, wl_pointer*,
                                               u32 serial, wl_surface* surface)
{
    auto* l = static_cast<WaylandMouseListener*>(data);
    if (l) l->handleLeave(serial, surface);
}

void WaylandMouseListener::handleMotionCallback(void* data, wl_pointer*,
                                                u32 time, wl_fixed_t x, wl_fixed_t y)
{
    auto* l = static_cast<WaylandMouseListener*>(data);
    if (l) l->handleMotion(time, x, y);
}

void WaylandMouseListener::handleButtonCallback(void* data, wl_pointer*,
                                                u32 serial, u32 time,
                                                u32 button, u32 state)
{
    auto* l = static_cast<WaylandMouseListener*>(data);
    if (l) l->handleButton(serial, time, button, state);
}

void WaylandMouseListener::handleAxisCallback(void* data, wl_pointer*,
                                              u32 time, u32 axis, wl_fixed_t value)
{
    auto* l = static_cast<WaylandMouseListener*>(data);
    if (l) l->handleAxis(time, axis, value);
}

void WaylandMouseListener::handleFrameCallback(void* data, wl_pointer*)
{
    auto* l = static_cast<WaylandMouseListener*>(data);
    if (l) l->handleFrame();
}

void WaylandMouseListener::handleAxisSourceCallback(void* data, wl_pointer*, u32 axis_source)
{
    auto* l = static_cast<WaylandMouseListener*>(data);
    if (l) l->handleAxisSource(axis_source);
}

void WaylandMouseListener::handleAxisStopCallback(void* data, wl_pointer*,
                                                  u32 time, u32 axis)
{
    auto* l = static_cast<WaylandMouseListener*>(data);
    if (l) l->handleAxisStop(time, axis);
}

void WaylandMouseListener::handleAxisDiscreteCallback(void* data, wl_pointer*,
                                                      u32 axis, i32 discrete)
{
    auto* l = static_cast<WaylandMouseListener*>(data);
    if (l) l->handleAxisDiscrete(axis, discrete);
}

} // namespace wma
