#ifndef WMA_CORE_WINDOW_FLAGS_HPP
#define WMA_CORE_WINDOW_FLAGS_HPP

#include <ink/ink_base.hpp>

namespace wma {

    /**
     * @brief Runtime flags and state information for the window
     */
    struct WindowFlags {
        u64 frameCounter;
        bool resized;
        bool minimized ;
        bool focused;
        f64 deltaTime;
        f64 fps;
        
        // Reset certain flags that need to be cleared each frame
        void resetFrameFlags() {
            resized = false;
        }

        WindowFlags() :
            frameCounter(0), resized(false), minimized(false), focused(true), deltaTime(0), fps(0.0) {}
    };

} // namespace wma

#endif // WMA_CORE_WINDOW_FLAGS_HPP
