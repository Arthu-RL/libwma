#ifndef WMA_INPUT_WAYLAND_KEYBOARD_LISTENER_HPP
#define WMA_INPUT_WAYLAND_KEYBOARD_LISTENER_HPP

#include "wma/input/keyboard/KeyboardListener.hpp"
#include <wayland-client.h>

namespace wma {

/**
 * @brief Wayland-specific keyboard listener implementation
 */
class WaylandKeyboardListener : public KeyboardListener {
public:
    WaylandKeyboardListener();
    ~WaylandKeyboardListener() override;

    /**
     * @brief Initialize the listener with a Wayland keyboard
     * @param keyboard The Wayland keyboard object
     */
    void initialize(wl_keyboard* keyboard);

    /**
     * @brief Get the Wayland keyboard object
     * @return Pointer to wl_keyboard
     */
    wl_keyboard* getKeyboard() const { return keyboard_; }

    // Wayland keyboard event handlers
    void handleKeymap(uint32_t format, int32_t fd, uint32_t size);
    void handleEnter(uint32_t serial, wl_surface* surface, wl_array* keys);
    void handleLeave(uint32_t serial, wl_surface* surface);
    void handleKey(uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    void handleModifiers(uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
                         uint32_t mods_locked, uint32_t group);
    void handleRepeatInfo(int32_t rate, int32_t delay);

private:
    wl_keyboard* keyboard_ = nullptr;

    // Wayland keyboard listener callbacks
    static const wl_keyboard_listener keyboardListener_;

    static void handleKeymapCallback(void* data, wl_keyboard* keyboard,
                                     uint32_t format, int32_t fd, uint32_t size);
    static void handleEnterCallback(void* data, wl_keyboard* keyboard,
                                    uint32_t serial, wl_surface* surface, wl_array* keys);
    static void handleLeaveCallback(void* data, wl_keyboard* keyboard,
                                    uint32_t serial, wl_surface* surface);
    static void handleKeyCallback(void* data, wl_keyboard* keyboard,
                                  uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    static void handleModifiersCallback(void* data, wl_keyboard* keyboard,
                                        uint32_t serial, uint32_t mods_depressed,
                                        uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
    static void handleRepeatInfoCallback(void* data, wl_keyboard* keyboard,
                                         int32_t rate, int32_t delay);
};

} // namespace wma

#endif // WMA_INPUT_WAYLAND_KEYBOARD_LISTENER_HPP
