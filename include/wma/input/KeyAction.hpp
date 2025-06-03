#ifndef WMA_INPUT_KEY_ACTION_HPP
#define WMA_INPUT_KEY_ACTION_HPP

#include <functional>

namespace wma {

    /**
     * @brief Encapsulates key press and release actions
     */
    class KeyAction {
    public:
        using ActionCallback = std::function<void()>;
        
        KeyAction(ActionCallback onPress = nullptr, ActionCallback onRelease = nullptr)
            : onPress_(std::move(onPress)), onRelease_(std::move(onRelease)) {}
        
        // Copy constructor
        KeyAction(const KeyAction& other) = default;
        
        // Move constructor
        KeyAction(KeyAction&& other) noexcept = default;
        
        // Copy assignment
        KeyAction& operator=(const KeyAction& other) = default;
        
        // Move assignment
        KeyAction& operator=(KeyAction&& other) noexcept = default;
        
        /**
         * @brief Execute the press action if available
         */
        void executePress() const {
            if (onPress_) {
                onPress_();
            }
        }
        
        /**
         * @brief Execute the release action if available
         */
        void executeRelease() const {
            if (onRelease_) {
                onRelease_();
            }
        }
        
        /**
         * @brief Check if press action is available
         */
        bool hasPressAction() const {
            return static_cast<bool>(onPress_);
        }
        
        /**
         * @brief Check if release action is available
         */
        bool hasReleaseAction() const {
            return static_cast<bool>(onRelease_);
        }
        
    private:
        ActionCallback onPress_;
        ActionCallback onRelease_;
    };

} // namespace wma

#endif // WMA_INPUT_KEY_ACTION_HPP