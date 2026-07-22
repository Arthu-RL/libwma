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

    //! Scoped enums are referenced qualified everywhere (GraphicsAPI::Vulkan, …).
    //! `enum class` keeps the enumerators out of namespace wma; the WmaCode values
    //! are deliberately `Ok`/`Error` (not `OK`/`ERROR`) because <windows.h> defines
    //! an `ERROR` macro and <X11/X.h> defines a `Success` macro — those tokens would
    //! be text-substituted even inside an enum declaration and break the build.
    enum class GraphicsAPI : i32 {
        OpenGL,
        Vulkan,
        CPU
    };

    enum class WindowBackend : i32 {
        GLFW,
        SDL3,
        X11,
        WAYLAND
    };

    enum class WmaCode : i32 {
        Ok,
        Error
    };

    /**
     * @brief A CPU-writable framebuffer for software rendering.
     *
     * Returned by IWindowManager::lockFramebuffer() when the window was created
     * with GraphicsAPI::CPU. Pixels are 32-bit; the exact channel order is
     * backend-defined (SDL follows the surface format; X11/Wayland use BGRA/XRGB8888).
     * `pixels == nullptr` means software rendering is unavailable for that backend
     * or the window is not in CPU mode.
     */
    struct SoftwareFramebuffer {
        void* pixels = nullptr;  ///< Base address of the top-left pixel.
        i32   width  = 0;        ///< Width in pixels.
        i32   height = 0;        ///< Height in pixels.
        i32   pitch  = 0;        ///< Bytes per row (may exceed width * 4).

        [[nodiscard]] constexpr bool valid() const noexcept { return pixels != nullptr; }
    };

} // namespace wma

#endif // WMA_CORE_TYPES_HPP
