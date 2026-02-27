#ifndef WMA_INPUT_KEYBOARD_LISTENER_HPP
#define WMA_INPUT_KEYBOARD_LISTENER_HPP

#include <unordered_map>

#include "wma/input/KeyAction.hpp"
#include "wma/core/Types.hpp"

namespace wma {

/**
 * @brief Abstract base class for cross-platform keyboard input handling
 *
 * Provides a unified interface for handling keyboard input across
 * different windowing backends (GLFW/SDL/X11).
 */
class KeyboardListener {
public:
    KeyboardListener() = default;
    virtual ~KeyboardListener() = default;

    // Non-copyable
    KeyboardListener(const KeyboardListener&) = delete;
    KeyboardListener& operator=(const KeyboardListener&) = delete;

    // Movable
    KeyboardListener(KeyboardListener&&) = default;
    KeyboardListener& operator=(KeyboardListener&&) = default;

    /**
     * @brief Add a key action mapping
     * @param key The key code (unified key enum)
     * @param action The action to execute
     */
    void addKeyAction(i32 key, KeyAction action);

    /**
     * @brief Remove a key action mapping
     * @param key The key code to remove
     */
    void removeKeyAction(i32 key);

    /**
     * @brief Clear all key action mappings
     */
    void clearKeyActions();

    /**
     * @brief Check if a key has an action mapped
     * @param key The key code to check
     * @return true if the key has an action mapped
     */
    bool hasKeyAction(i32 key) const;

protected:
    std::unordered_map<i32, KeyAction> keyActions_;
};

} // namespace wma

#endif // WMA_INPUT_KEYBOARD_LISTENER_HPP
