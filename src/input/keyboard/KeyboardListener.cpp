#include "wma/input/KeyboardListener.hpp"

namespace wma {

void KeyboardListener::addKeyAction(i32 key, KeyAction action) {
    keyActions_[key] = std::move(action);
}

void KeyboardListener::removeKeyAction(i32 key) {
    keyActions_.erase(key);
}

void KeyboardListener::clearKeyActions() {
    keyActions_.clear();
}

bool KeyboardListener::hasKeyAction(i32 key) const {
    return keyActions_.find(key) != keyActions_.end();
}

} // namespace wma
