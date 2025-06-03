#ifndef WMA_INPUT_MOUSE_ACTION_HPP
#define WMA_INPUT_MOUSE_ACTION_HPP

#include <functional>
#include "../core/Types.hpp"

namespace wma {

    /**
     * @brief Mouse position data
     */
    struct MousePosition {
        f64 x = 0.0;
        f64 y = 0.0;
        f64 deltaX = 0.0;
        f64 deltaY = 0.0;
        
        MousePosition() = default;
        MousePosition(f64 x, f64 y, f64 deltaX = 0.0, f64 deltaY = 0.0)
            : x(x), y(y), deltaX(deltaX), deltaY(deltaY) {}
    };

    /**
     * @brief Mouse scroll data
     */
    struct MouseScroll {
        f64 xOffset = 0.0;
        f64 yOffset = 0.0;
        
        MouseScroll() = default;
        MouseScroll(f64 xOffset, f64 yOffset)
            : xOffset(xOffset), yOffset(yOffset) {}
    };

    /**
     * @brief Encapsulates mouse button press, release, and movement actions
     */
    class MouseAction {
    public:
        using ButtonCallback = std::function<void()>;
        using PositionCallback = std::function<void(const MousePosition&)>;
        using ScrollCallback = std::function<void(const MouseScroll&)>;
        
        MouseAction(ButtonCallback onPress = nullptr, 
                   ButtonCallback onRelease = nullptr,
                   PositionCallback onMove = nullptr,
                   ScrollCallback onScroll = nullptr)
            : onPress_(std::move(onPress))
            , onRelease_(std::move(onRelease))
            , onMove_(std::move(onMove))
            , onScroll_(std::move(onScroll)) {}
        
        // Copy constructor
        MouseAction(const MouseAction& other) = default;
        
        // Move constructor
        MouseAction(MouseAction&& other) noexcept = default;
        
        // Copy assignment
        MouseAction& operator=(const MouseAction& other) = default;
        
        // Move assignment
        MouseAction& operator=(MouseAction&& other) noexcept = default;
        
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
         * @brief Execute the movement action if available
         */
        void executeMove(const MousePosition& position) const {
            if (onMove_) {
                onMove_(position);
            }
        }
        
        /**
         * @brief Execute the scroll action if available
         */
        void executeScroll(const MouseScroll& scroll) const {
            if (onScroll_) {
                onScroll_(scroll);
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
        
        /**
         * @brief Check if movement action is available
         */
        bool hasMoveAction() const {
            return static_cast<bool>(onMove_);
        }
        
        /**
         * @brief Check if scroll action is available
         */
        bool hasScrollAction() const {
            return static_cast<bool>(onScroll_);
        }
        
    private:
        ButtonCallback onPress_;
        ButtonCallback onRelease_;
        PositionCallback onMove_;
        ScrollCallback onScroll_;
    };

    /**
     * @brief Mouse button constants for cross-platform compatibility
     */
    namespace MouseButton {
        constexpr i32 Left = 0;
        constexpr i32 Right = 1;
        constexpr i32 Middle = 2;
        constexpr i32 Button4 = 3;
        constexpr i32 Button5 = 4;
        constexpr i32 Button6 = 5;
        constexpr i32 Button7 = 6;
        constexpr i32 Button8 = 7;
    }

} // namespace wma

#endif // WMA_INPUT_MOUSE_ACTION_HPP