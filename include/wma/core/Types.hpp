#ifndef WMA_CORE_TYPES_HPP
#define WMA_CORE_TYPES_HPP

#include <ink/ink_base.hpp>

namespace wma {

    enum GraphicsAPI : i32 {
        OpenGL,
        Vulkan,
        CPU
    };

    enum WindowBackend : i32 {
        GLFW,
        SDL3,
        X11,
        WAYLAND
    };

    enum WmaCode : i32 {
        OK,
        ERROR
    };

} // namespace wma

#endif // WMA_CORE_TYPES_HPP
