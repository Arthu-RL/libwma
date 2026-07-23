#ifndef WMA_INPUT_KEYBOARD_LISTENER_HPP
#define WMA_INPUT_KEYBOARD_LISTENER_HPP

#include <vector>

#include "wma/input/InputBindingTable.hpp"
#include "wma/input/InputContextStack.hpp"
#include "wma/input/InputTypes.hpp"
#include "wma/input/keyboard/KeyAction.hpp"
#include "wma/input/keyboard/Keys.h"
#include "wma/core/Types.hpp"

namespace wma {

class KeyboardListener {
public:
    KeyboardListener();
    virtual ~KeyboardListener() = default;

    KeyboardListener(const KeyboardListener&)            = delete;
    KeyboardListener& operator=(const KeyboardListener&) = delete;
    KeyboardListener(KeyboardListener&&)                 = default;
    KeyboardListener& operator=(KeyboardListener&&)      = default;

    //! Context management.
    [[nodiscard]] InputContextId createContext();
    void setActiveContext(InputContextId context);
    void pushContext(InputContextId context);
    void popContext();
    [[nodiscard]] InputContextId getActiveContext()  const;
    [[nodiscard]] InputContextId getResolvedContext() const;

    //! Binding — default context (resolved at call time).
    void addKeyAction(Key key, KeyAction action);
    void addKeyAction(i32 key, KeyAction action);

    //! Binding — explicit context.
    void addKeyAction(Key key, KeyAction action, InputContextId context);
    void addKeyAction(i32 key, KeyAction action, InputContextId context);

    void removeKeyAction(Key key);
    void removeKeyAction(i32 key);
    void removeKeyAction(Key key, InputContextId context);
    void removeKeyAction(i32 key, InputContextId context);

    void clearKeyActions();
    void clearKeyActions(InputContextId context);

    [[nodiscard]] bool hasKeyAction(Key key) const;
    [[nodiscard]] bool hasKeyAction(i32 key) const;
    [[nodiscard]] bool hasKeyAction(Key key, InputContextId context) const;
    [[nodiscard]] bool hasKeyAction(i32 key, InputContextId context) const;

protected:
    void dispatchKeyPress(Key key);
    void dispatchKeyRelease(Key key);
    void ensureContextCapacity(InputContextId context);

    InputContextStack contexts_;
    std::vector<InputBindingTable<KeyAction, KEY_COUNT>> keyBindings_;
};

} // namespace wma

#endif // WMA_INPUT_KEYBOARD_LISTENER_HPP
