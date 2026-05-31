#ifndef WMA_INPUT_KEYBOARD_LISTENER_HPP
#define WMA_INPUT_KEYBOARD_LISTENER_HPP

#include <unordered_map>

#include "wma/input/keyboard/KeyAction.hpp"
#include "wma/core/Types.hpp"

namespace wma {

class KeyboardListener {
public:
    KeyboardListener() = default;
    virtual ~KeyboardListener() = default;

    KeyboardListener(const KeyboardListener&) = delete;
    KeyboardListener& operator=(const KeyboardListener&) = delete;
    KeyboardListener(KeyboardListener&&) = default;
    KeyboardListener& operator=(KeyboardListener&&) = default;

    void addKeyAction(i32 key, KeyAction action);
    void removeKeyAction(i32 key);
    void clearKeyActions();
    bool hasKeyAction(i32 key) const;

protected:
    std::unordered_map<i32, KeyAction> keyActions_;
};

} // namespace wma

#endif // WMA_INPUT_KEYBOARD_LISTENER_HPP
