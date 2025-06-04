#include "wma/wma.hpp"
#include "wma/exceptions/WMAException.hpp"

namespace wma {

    std::unique_ptr<IWindowManager> createWindowManager(
        WindowBackend backend,
        const WindowDetails& windowDetails,
        GraphicsAPI graphicsAPI
    ) {
        switch (backend) {
#ifdef WMA_ENABLE_GLFW
            case WindowBackend::GLFW:
                return std::make_unique<GlfwWindowManager>(windowDetails, graphicsAPI);
#endif

#ifdef WMA_ENABLE_SDL
            case WindowBackend::SDL2:
                return std::make_unique<SdlWindowManager>(windowDetails, graphicsAPI);
#endif

            default:
                throw WMAException("Requested window backend is not available or not compiled in");
        }
    }

    WindowBackend getDefaultBackend() {
        // Priority order: GLFW first (generally more stable for graphics development)
#ifdef WMA_ENABLE_GLFW
        return WindowBackend::GLFW;
#elif defined(WMA_ENABLE_SDL)
        return WindowBackend::SDL2;
#else
        static_assert(false, "No window backend is enabled");
#endif
    }

    bool isBackendAvailable(WindowBackend backend) {
        switch (backend) {
#ifdef WMA_ENABLE_GLFW
            case WindowBackend::GLFW:
                return true;
#endif

#ifdef WMA_ENABLE_SDL
            case WindowBackend::SDL2:
                return true;
#endif

            default:
                return false;
        }
    }

    const char* getLibraryInfo() {
        static const char* info = 
            "WMA Window Management & Input Abstraction Library v" WMA_VERSION_STRING_FULL "\n"
            "Backends: "
#ifdef WMA_ENABLE_GLFW
            "GLFW "
#endif
#ifdef WMA_ENABLE_SDL
            "SDL2 "
#endif
            "\nGraphics APIs: "
#ifdef WMA_ENABLE_VULKAN
            "Vulkan "
#endif
#ifdef WMA_ENABLE_OPENGL
            "OpenGL "
#endif
            "\nBuilt with C++17";
        
        return info;
    }

} // namespace wma
