#include "wma/backends/wayland/WaylandKeyboardListener.hpp"
#include "wma/input/keyboard/Keys.h"
#include "wma/exceptions/WMAException.hpp"

#include <unistd.h>
#include <sys/mman.h>
#include <xkbcommon/xkbcommon.h>

namespace wma {

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

void WaylandKeyboardListener::handleKeymap(u32, i32 fd, u32)
{
    close(fd);
}

void WaylandKeyboardListener::handleEnter(u32, wl_surface*, wl_array*)
{
}

void WaylandKeyboardListener::handleLeave(u32, wl_surface*)
{
}

void WaylandKeyboardListener::handleKey(u32, u32, u32 key, u32 state)
{
    u32 xkbKeycode = key + 8;
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

void WaylandKeyboardListener::handleModifiers(u32, u32, u32, u32, u32)
{
}

void WaylandKeyboardListener::handleRepeatInfo(i32, i32)
{
}

void WaylandKeyboardListener::handleKeymapCallback(void* data, wl_keyboard*,
                                                   u32 format, i32 fd, u32 size)
{
    auto* listener = static_cast<WaylandKeyboardListener*>(data);
    if (listener) listener->handleKeymap(format, fd, size);
}

void WaylandKeyboardListener::handleEnterCallback(void* data, wl_keyboard*,
                                                  u32 serial, wl_surface* surface, wl_array* keys)
{
    auto* listener = static_cast<WaylandKeyboardListener*>(data);
    if (listener) listener->handleEnter(serial, surface, keys);
}

void WaylandKeyboardListener::handleLeaveCallback(void* data, wl_keyboard*,
                                                  u32 serial, wl_surface* surface)
{
    auto* listener = static_cast<WaylandKeyboardListener*>(data);
    if (listener) listener->handleLeave(serial, surface);
}

void WaylandKeyboardListener::handleKeyCallback(void* data, wl_keyboard*,
                                                u32 serial, u32 time,
                                                u32 key, u32 state)
{
    auto* listener = static_cast<WaylandKeyboardListener*>(data);
    if (listener) listener->handleKey(serial, time, key, state);
}

void WaylandKeyboardListener::handleModifiersCallback(void* data, wl_keyboard*,
                                                      u32 serial, u32 mods_depressed,
                                                      u32 mods_latched, u32 mods_locked,
                                                      u32 group)
{
    auto* listener = static_cast<WaylandKeyboardListener*>(data);
    if (listener) listener->handleModifiers(serial, mods_depressed, mods_latched, mods_locked, group);
}

void WaylandKeyboardListener::handleRepeatInfoCallback(void* data, wl_keyboard*,
                                                       i32 rate, i32 delay)
{
    auto* listener = static_cast<WaylandKeyboardListener*>(data);
    if (listener) listener->handleRepeatInfo(rate, delay);
}

} // namespace wma
