#ifndef WMA_INPUT_WAYLAND_MOUSE_LISTENER_HPP
#define WMA_INPUT_WAYLAND_MOUSE_LISTENER_HPP

#include "wma/input/mouse/MouseListener.hpp"
#include <wayland-client.h>

namespace wma {

/**
 * @brief Wayland-specific mouse/pointer listener implementation
 */
class WaylandMouseListener : public MouseListener {
public:
    WaylandMouseListener();
    ~WaylandMouseListener() override;

    /**
     * @brief Initialize the listener with a Wayland pointer
     * @param pointer The Wayland pointer object
     */
    void initialize(wl_pointer* pointer);

    /**
     * @brief Get the Wayland pointer object
     * @return Pointer to wl_pointer
     */
    wl_pointer* getPointer() const { return pointer_; }

    // Wayland pointer event handlers
    void handleEnter(uint32_t serial, wl_surface* surface, wl_fixed_t x, wl_fixed_t y);
    void handleLeave(uint32_t serial, wl_surface* surface);
    void handleMotion(uint32_t time, wl_fixed_t x, wl_fixed_t y);
    void handleButton(uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
    void handleAxis(uint32_t time, uint32_t axis, wl_fixed_t value);
    void handleFrame();
    void handleAxisSource(uint32_t axis_source);
    void handleAxisStop(uint32_t time, uint32_t axis);
    void handleAxisDiscrete(uint32_t axis, int32_t discrete);

protected:
    void updateCursorState() override;

private:
    wl_pointer* pointer_ = nullptr;
    wl_surface* cursorSurface_ = nullptr;

    // Wayland pointer listener callbacks
    static const wl_pointer_listener pointerListener_;

    static void handleEnterCallback(void* data, wl_pointer* pointer,
                                    uint32_t serial, wl_surface* surface,
                                    wl_fixed_t x, wl_fixed_t y);
    static void handleLeaveCallback(void* data, wl_pointer* pointer,
                                    uint32_t serial, wl_surface* surface);
    static void handleMotionCallback(void* data, wl_pointer* pointer,
                                     uint32_t time, wl_fixed_t x, wl_fixed_t y);
    static void handleButtonCallback(void* data, wl_pointer* pointer,
                                     uint32_t serial, uint32_t time,
                                     uint32_t button, uint32_t state);
    static void handleAxisCallback(void* data, wl_pointer* pointer,
                                   uint32_t time, uint32_t axis, wl_fixed_t value);
    static void handleFrameCallback(void* data, wl_pointer* pointer);
    static void handleAxisSourceCallback(void* data, wl_pointer* pointer,
                                         uint32_t axis_source);
    static void handleAxisStopCallback(void* data, wl_pointer* pointer,
                                       uint32_t time, uint32_t axis);
    static void handleAxisDiscreteCallback(void* data, wl_pointer* pointer,
                                           uint32_t axis, int32_t discrete);

    // Helper to convert Wayland button codes
    i32 convertButton(uint32_t waylandButton) const;
};

} // namespace wma

#endif // WMA_INPUT_WAYLAND_MOUSE_LISTENER_HPP
