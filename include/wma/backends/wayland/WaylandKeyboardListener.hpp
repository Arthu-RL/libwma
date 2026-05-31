#ifndef WMA_BACKENDS_WAYLAND_KEYBOARD_LISTENER_HPP
#define WMA_BACKENDS_WAYLAND_KEYBOARD_LISTENER_HPP

#include "wma/input/keyboard/KeyboardListener.hpp"
#include <wayland-client.h>

namespace wma {

class WaylandKeyboardListener : public KeyboardListener {
public:
    WaylandKeyboardListener();
    ~WaylandKeyboardListener() override;

    void initialize(wl_keyboard* keyboard);
    wl_keyboard* getKeyboard() const { return keyboard_; }

    void handleKeymap(u32 format, i32 fd, u32 size);
    void handleEnter(u32 serial, wl_surface* surface, wl_array* keys);
    void handleLeave(u32 serial, wl_surface* surface);
    void handleKey(u32 serial, u32 time, u32 key, u32 state);
    void handleModifiers(u32 serial, u32 mods_depressed, u32 mods_latched,
                         u32 mods_locked, u32 group);
    void handleRepeatInfo(i32 rate, i32 delay);

private:
    wl_keyboard* keyboard_ = nullptr;

    static const wl_keyboard_listener keyboardListener_;

    static void handleKeymapCallback(void* data, wl_keyboard* keyboard,
                                     u32 format, i32 fd, u32 size);
    static void handleEnterCallback(void* data, wl_keyboard* keyboard,
                                    u32 serial, wl_surface* surface, wl_array* keys);
    static void handleLeaveCallback(void* data, wl_keyboard* keyboard,
                                    u32 serial, wl_surface* surface);
    static void handleKeyCallback(void* data, wl_keyboard* keyboard,
                                  u32 serial, u32 time, u32 key, u32 state);
    static void handleModifiersCallback(void* data, wl_keyboard* keyboard,
                                        u32 serial, u32 mods_depressed,
                                        u32 mods_latched, u32 mods_locked, u32 group);
    static void handleRepeatInfoCallback(void* data, wl_keyboard* keyboard,
                                         i32 rate, i32 delay);
};

} // namespace wma

#endif // WMA_BACKENDS_WAYLAND_KEYBOARD_LISTENER_HPP
