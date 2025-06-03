#pragma once
#ifndef WMA_CORE_WINDOW_DETAILS_HPP
#define WMA_CORE_WINDOW_DETAILS_HPP

#include "Types.hpp"

namespace wma {

    /**
     * @brief Configuration structure for window creation
     */
    struct WindowDetails {
        i32 width = 800;
        i32 height = 600;
        bool resizable = true;
        i32 targetFPS = 60;
        bool vsync = false;
        bool fullscreen = false;
        
        // Default constructor
        WindowDetails() = default;
        
        // Convenience constructor
        WindowDetails(i32 w, i32 h, bool resize = true, i32 fps = 60, bool vsync_ = false, bool fullscreen_ = false)
            : width(w), height(h), resizable(resize), targetFPS(fps), vsync(vsync_), fullscreen(fullscreen_) {}
    };

} // namespace wma

#endif // WMA_CORE_WINDOW_DETAILS_HPP
