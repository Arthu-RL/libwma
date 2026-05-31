#ifndef WMA_BACKENDS_WAYLAND_MOUSE_LISTENER_HPP
#define WMA_BACKENDS_WAYLAND_MOUSE_LISTENER_HPP

#include "wma/input/mouse/MouseListener.hpp"
#include <wayland-client.h>

namespace wma {

class WaylandMouseListener : public MouseListener {
public:
    WaylandMouseListener();
    ~WaylandMouseListener() override;

    void initialize(wl_pointer* pointer);
    wl_pointer* getPointer() const { return pointer_; }

    void handleEnter(u32 serial, wl_surface* surface, wl_fixed_t x, wl_fixed_t y);
    void handleLeave(u32 serial, wl_surface* surface);
    void handleMotion(u32 time, wl_fixed_t x, wl_fixed_t y);
    void handleButton(u32 serial, u32 time, u32 button, u32 state);
    void handleAxis(u32 time, u32 axis, wl_fixed_t value);
    void handleFrame();
    void handleAxisSource(u32 axis_source);
    void handleAxisStop(u32 time, u32 axis);
    void handleAxisDiscrete(u32 axis, i32 discrete);

protected:
    void updateCursorState() override;

private:
    wl_pointer* pointer_ = nullptr;
    wl_surface* cursorSurface_ = nullptr;

    static const wl_pointer_listener pointerListener_;

    static void handleEnterCallback(void* data, wl_pointer* pointer,
                                    u32 serial, wl_surface* surface,
                                    wl_fixed_t x, wl_fixed_t y);
    static void handleLeaveCallback(void* data, wl_pointer* pointer,
                                    u32 serial, wl_surface* surface);
    static void handleMotionCallback(void* data, wl_pointer* pointer,
                                     u32 time, wl_fixed_t x, wl_fixed_t y);
    static void handleButtonCallback(void* data, wl_pointer* pointer,
                                     u32 serial, u32 time,
                                     u32 button, u32 state);
    static void handleAxisCallback(void* data, wl_pointer* pointer,
                                   u32 time, u32 axis, wl_fixed_t value);
    static void handleFrameCallback(void* data, wl_pointer* pointer);
    static void handleAxisSourceCallback(void* data, wl_pointer* pointer,
                                         u32 axis_source);
    static void handleAxisStopCallback(void* data, wl_pointer* pointer,
                                       u32 time, u32 axis);
    static void handleAxisDiscreteCallback(void* data, wl_pointer* pointer,
                                           u32 axis, i32 discrete);

    i32 convertButton(u32 waylandButton) const;
};

} // namespace wma

#endif // WMA_BACKENDS_WAYLAND_MOUSE_LISTENER_HPP
