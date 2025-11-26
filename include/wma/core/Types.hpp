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
        SDL2,
        X11
    };

    enum WmaCode : i32 {
        OK,
        ERROR
    };

#ifdef WMA_ENABLE_GLFW
    class GlfwWindowManager;
    class KeyboardListener;
    class MouseListener;

    /**
     * @brief User data structure for GLFW callbacks
     */
    struct GlfwUserData {
        GlfwWindowManager* windowManager = nullptr;
        KeyboardListener* keyboardListener = nullptr;
        MouseListener* mouseListener = nullptr;
    };
#endif

} // namespace wma

#endif // WMA_CORE_TYPES_HPP
