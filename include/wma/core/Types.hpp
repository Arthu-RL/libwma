#ifndef WMA_CORE_TYPES_HPP
#define WMA_CORE_TYPES_HPP

#include <functional>
#include <ink/ink_base.hpp>

namespace wma {

#if defined(__cpp_lib_move_only_function)
    template <typename Sig>
    using move_only_function = std::move_only_function<Sig>;
#else
    template <typename Sig>
    using move_only_function = std::function<Sig>;
#endif

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
