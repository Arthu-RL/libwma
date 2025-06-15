#ifndef WMA_CORE_TYPES_HPP
#define WMA_CORE_TYPES_HPP

namespace wma {
    // Graphics API enumeration
    enum class GraphicsAPI {
        OpenGL,
        Vulkan,
        CPU  // Software rendering
    };

    // Window backend enumeration
    enum class WindowBackend {
        GLFW,
        SDL2
    };

} // namespace wma

#endif // WMA_CORE_TYPES_HPP
