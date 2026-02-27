#include "wma/input/keyboard/WaylandKeyboardListener.hpp"
#include "wma/input/Keys.h"
#include "wma/exceptions/WMAException.hpp"

#include <unistd.h>
#include <sys/mman.h>
#include <xkbcommon/xkbcommon.h>

namespace wma {

// Static keyboard listener implementation
const wl_keyboard_listener WaylandKeyboardListener::keyboardListener_ = {
    .keymap = handleKeymapCallback,
    .enter = handleEnterCallback,
    .leave = handleLeaveCallback,
    .key = handleKeyCallback,
    .modifiers = handleModifiersCallback,
    .repeat_info = handleRepeatInfoCallback
};

WaylandKeyboardListener::WaylandKeyboardListener()
    : KeyboardListener()
    , keyboard_(nullptr)
{
}

WaylandKeyboardListener::~WaylandKeyboardListener()
{
    if (keyboard_) {
        wl_keyboard_destroy(keyboard_);
        keyboard_ = nullptr;
    }
}

void WaylandKeyboardListener::initialize(wl_keyboard* keyboard)
{
    if (!keyboard) {
        throw InputException("Invalid Wayland keyboard pointer");
    }

    keyboard_ = keyboard;
    wl_keyboard_add_listener(keyboard_, &keyboardListener_, this);
}

void WaylandKeyboardListener::handleKeymap(uint32_t format, int32_t fd, uint32_t size)
{
    // This would typically set up XKB keymap for key translation
    // For now, we'll close the fd as we're handling basic key codes
    close(fd);
}

void WaylandKeyboardListener::handleEnter(uint32_t serial, wl_surface* surface, wl_array* keys)
{
    // Called when keyboard focus enters our surface
    // Can be used to track focus state
}

void WaylandKeyboardListener::handleLeave(uint32_t serial, wl_surface* surface)
{
    // Called when keyboard focus leaves our surface
    // Can be used to release all pressed keys
}

void WaylandKeyboardListener::handleKey(uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    // Wayland sends Linux evdev key codes (offset by 8)
    // Convert to our unified key enum
    uint32_t xkbKeycode = key + 8;
    Key mappedKey = mapWaylandKey(xkbKeycode);

    auto it = keyActions_.find(static_cast<i32>(mappedKey));
    if (it != keyActions_.end()) {
        if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
            it->second.executePress();
        } else if (state == WL_KEYBOARD_KEY_STATE_RELEASED) {
            it->second.executeRelease();
        }
    }
}

void WaylandKeyboardListener::handleModifiers(uint32_t serial, uint32_t mods_depressed,
                                              uint32_t mods_latched, uint32_t mods_locked,
                                              uint32_t group)
{
    // Handle modifier keys (Shift, Ctrl, Alt, etc.)
    // This would typically update XKB state for proper key interpretation
}

void WaylandKeyboardListener::handleRepeatInfo(int32_t rate, int32_t delay)
{
    // Handle key repeat configuration
    // rate: characters per second
    // delay: milliseconds before repeat starts
}

// Static callback implementations
void WaylandKeyboardListener::handleKeymapCallback(void* data, wl_keyboard* keyboard,
                                                   uint32_t format, int32_t fd, uint32_t size)
{
    auto* listener = static_cast<WaylandKeyboardListener*>(data);
    if (listener) {
        listener->handleKeymap(format, fd, size);
    }
}

void WaylandKeyboardListener::handleEnterCallback(void* data, wl_keyboard* keyboard,
                                                  uint32_t serial, wl_surface* surface,
                                                  wl_array* keys)
{
    auto* listener = static_cast<WaylandKeyboardListener*>(data);
    if (listener) {
        listener->handleEnter(serial, surface, keys);
    }
}

void WaylandKeyboardListener::handleLeaveCallback(void* data, wl_keyboard* keyboard,
                                                  uint32_t serial, wl_surface* surface)
{
    auto* listener = static_cast<WaylandKeyboardListener*>(data);
    if (listener) {
        listener->handleLeave(serial, surface);
    }
}

void WaylandKeyboardListener::handleKeyCallback(void* data, wl_keyboard* keyboard,
                                                uint32_t serial, uint32_t time,
                                                uint32_t key, uint32_t state)
{
    auto* listener = static_cast<WaylandKeyboardListener*>(data);
    if (listener) {
        listener->handleKey(serial, time, key, state);
    }
}

void WaylandKeyboardListener::handleModifiersCallback(void* data, wl_keyboard* keyboard,
                                                      uint32_t serial, uint32_t mods_depressed,
                                                      uint32_t mods_latched, uint32_t mods_locked,
                                                      uint32_t group)
{
    auto* listener = static_cast<WaylandKeyboardListener*>(data);
    if (listener) {
        listener->handleModifiers(serial, mods_depressed, mods_latched, mods_locked, group);
    }
}

void WaylandKeyboardListener::handleRepeatInfoCallback(void* data, wl_keyboard* keyboard,
                                                       int32_t rate, int32_t delay)
{
    auto* listener = static_cast<WaylandKeyboardListener*>(data);
    if (listener) {
        listener->handleRepeatInfo(rate, delay);
    }
}

} // namespace wma
