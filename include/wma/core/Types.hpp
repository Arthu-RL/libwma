#ifndef WMA_CORE_TYPES_HPP
#define WMA_CORE_TYPES_HPP

#include <ink/ink_base.hpp>

namespace wma {
    // Graphics API enumeration
    enum GraphicsAPI : i32 {
        OpenGL,
        Vulkan,
        CPU  // Software rendering
    };

    // Window backend enumeration
    enum WindowBackend : i32 {
        GLFW,
        SDL2
    };

    enum WmaCode : i32 {
        OK,
        ERROR
    };

} // namespace wma

#endif // WMA_CORE_TYPES_HPP
