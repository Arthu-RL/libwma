#ifndef WMA_MANAGERS_IWINDOW_MANAGER_HPP
#define WMA_MANAGERS_IWINDOW_MANAGER_HPP

#include <functional>
#include <vector>
#include <memory>

#include "../core/Types.hpp"
#include "../core/WindowDetails.hpp"
#include "../core/WindowFlags.hpp"
#include "../input/keyboard/KeyboardListener.hpp"
#include "../input/mouse/MouseListener.hpp"

namespace wma {

    /**
     * @brief Abstract base interface for window managers.
     *
     * Provides a common interface across windowing backends (GLFW, SDL3, X11,
     * Wayland) and graphics APIs (OpenGL, Vulkan, software/CPU).
     *
     * Two ways to drive the window:
     *   - process(): a managed blocking run-loop. On Emscripten/WASM it hands the
     *     loop to the browser (via emscripten_set_main_loop) and returns
     *     immediately, because a blocking while-loop cannot run there.
     *   - Manual stepping: call pollEvents(), render, then swapBuffers() yourself.
     *     Works identically on every platform.
     */
    class IWindowManager {
    public:
        virtual ~IWindowManager() = default;

        virtual void createWindow(const char* windowName) = 0;

        /**
         * @brief Run the managed main loop, invoking @p actions once per frame.
         *
         * Desktop: blocks until the window closes. WASM: registers the browser
         * animation-frame loop and returns immediately (the callable and window
         * must outlive the loop). Implemented in terms of pollEvents()/
         * swapBuffers()/shouldClose(), so backends do not reimplement it.
         */
        void process(std::function<void()>&& actions);

        //! Pump the OS event queue once (non-blocking). Safe to call every frame.
        virtual void pollEvents() = 0;

        //! Present the current frame: GL buffer swap, software blit, or no-op for
        //! Vulkan (which the application presents through its own swapchain).
        virtual void swapBuffers() {}

        //! Native window handle (SDL_Window*, GLFWwindow*, X11 Window, wl_surface*).
        virtual void* getWindowInstance() = 0;

        //! Native display/connection handle (X11 Display*, wl_display*) or nullptr
        //! when the backend has no separate display object (SDL, GLFW).
        virtual void* getNativeDisplayHandle() const noexcept { return nullptr; }

        //! Load an OpenGL function pointer. Returns nullptr when the window is not
        //! an OpenGL window or the backend cannot resolve it.
        virtual void* getGLProcAddress(const char* /*name*/) const { return nullptr; }

        //! Acquire a CPU-writable framebuffer (GraphicsAPI::CPU only). Returns an
        //! invalid framebuffer (pixels == nullptr) when unsupported.
        virtual SoftwareFramebuffer lockFramebuffer() { return {}; }

        //! Blit the framebuffer previously obtained from lockFramebuffer().
        virtual void presentFramebuffer() {}

        virtual WindowFlags* getWindowFlags() noexcept = 0;
        virtual const WindowDetails* getWindowDetails() noexcept = 0;
        virtual const std::vector<const char*> getVulkanExtensions() const = 0;
        virtual KeyboardListener& getKeyboardListener() noexcept = 0;
        virtual MouseListener& getMouseListener() noexcept = 0;
        virtual bool shouldClose() const = 0;
        virtual WindowBackend getBackendType() const = 0;
        virtual GraphicsAPI getGraphicsAPI() const = 0;
        virtual WmaCode destroy() = 0;

    protected:
        IWindowManager() = default;
    };

    using WindowManagerFactory = std::function<std::unique_ptr<IWindowManager>(const WindowDetails&)>;

} // namespace wma

#endif // WMA_MANAGERS_IWINDOW_MANAGER_HPP
