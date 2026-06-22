#include "wma/input/keyboard/KeyboardListener.hpp"

#include <algorithm>
#include <utility>

namespace wma {

KeyboardListener::KeyboardListener() {
    keyBindings_.emplace_back();
}

InputContextId KeyboardListener::createContext() {
    const InputContextId context = contexts_.createContext();
    ensureContextCapacity(context);
    return context;
}

void KeyboardListener::setActiveContext(InputContextId context) {
    contexts_.setActiveContext(context);
}

void KeyboardListener::pushContext(InputContextId context) {
    contexts_.pushContext(context);
}

void KeyboardListener::popContext() {
    contexts_.popContext();
}

InputContextId KeyboardListener::getActiveContext() const {
    return contexts_.activeContext();
}

InputContextId KeyboardListener::getResolvedContext() const {
    return contexts_.resolved();
}

void KeyboardListener::addKeyAction(Key key, KeyAction action) {
    addKeyAction(key, std::move(action), contexts_.resolved());
}

void KeyboardListener::addKeyAction(Key key, KeyAction action, InputContextId context) {
    const auto k = std::to_underlying(key);
    if (k < 0 || static_cast<usize>(k) >= KEY_COUNT) [[unlikely]] return;
    ensureContextCapacity(context);
    keyBindings_[context][static_cast<usize>(k)] = std::move(action);
}

void KeyboardListener::addKeyAction(i32 key, KeyAction action) {
    addKeyAction(static_cast<Key>(key), std::move(action));
}

void KeyboardListener::addKeyAction(i32 key, KeyAction action, InputContextId context) {
    addKeyAction(static_cast<Key>(key), std::move(action), context);
}

void KeyboardListener::removeKeyAction(Key key) {
    removeKeyAction(key, contexts_.resolved());
}

void KeyboardListener::removeKeyAction(Key key, InputContextId context) {
    const auto k = std::to_underlying(key);
    if (k < 0 || static_cast<usize>(k) >= KEY_COUNT) [[unlikely]] return;
    ensureContextCapacity(context);
    keyBindings_[context].clearSlot(static_cast<usize>(k));
}

void KeyboardListener::removeKeyAction(i32 key) {
    removeKeyAction(static_cast<Key>(key));
}

void KeyboardListener::removeKeyAction(i32 key, InputContextId context) {
    removeKeyAction(static_cast<Key>(key), context);
}

void KeyboardListener::clearKeyActions() {
    std::ranges::for_each(keyBindings_, [](auto& t) { t.clear(); });
}

void KeyboardListener::clearKeyActions(InputContextId context) {
    ensureContextCapacity(context);
    keyBindings_[context].clear();
}

bool KeyboardListener::hasKeyAction(Key key) const {
    return hasKeyAction(key, contexts_.resolved());
}

bool KeyboardListener::hasKeyAction(Key key, InputContextId context) const {
    const auto k = std::to_underlying(key);
    if (k < 0 || static_cast<usize>(k) >= KEY_COUNT) [[unlikely]] return false;
    if (context >= keyBindings_.size()) return false;
    return keyBindings_[context].has(static_cast<usize>(k));
}

bool KeyboardListener::hasKeyAction(i32 key) const {
    return hasKeyAction(static_cast<Key>(key));
}

bool KeyboardListener::hasKeyAction(i32 key, InputContextId context) const {
    return hasKeyAction(static_cast<Key>(key), context);
}

void KeyboardListener::dispatchKeyPress(Key key) {
    const auto k = std::to_underlying(key);
    if (k < 0 || static_cast<usize>(k) >= KEY_COUNT) [[unlikely]] return;
    [[assume(k >= 0)]];
    [[assume(static_cast<usize>(k) < KEY_COUNT)]];

    const InputContextId ctx = contexts_.resolved();
    if (ctx >= keyBindings_.size()) [[unlikely]] return;
    [[assume(ctx < keyBindings_.size())]];

    keyBindings_[ctx][static_cast<usize>(k)].executePress();
}

void KeyboardListener::dispatchKeyRelease(Key key) {
    const auto k = std::to_underlying(key);
    if (k < 0 || static_cast<usize>(k) >= KEY_COUNT) [[unlikely]] return;
    [[assume(k >= 0)]];
    [[assume(static_cast<usize>(k) < KEY_COUNT)]];

    const InputContextId ctx = contexts_.resolved();
    if (ctx >= keyBindings_.size()) [[unlikely]] return;
    [[assume(ctx < keyBindings_.size())]];

    keyBindings_[ctx][static_cast<usize>(k)].executeRelease();
}

void KeyboardListener::ensureContextCapacity(InputContextId context) {
    while (keyBindings_.size() <= context) {
        keyBindings_.emplace_back();
    }
}

} // namespace wma
