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
#include "../input/touch/TouchListener.hpp"

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

        /**
         * @brief Cheap, non-blocking check: is the platform's native surface
         * currently attached to getWindowInstance()'s handle?
         *
         * Unlike waitUntilWindowReady() (which can block for seconds riding
         * out startup/resume churn), this is meant to be polled every frame
         * to decide whether to skip rendering entirely -- e.g. while an
         * Android app is backgrounded and has no surface at all, there is
         * nothing to wait for yet, so a renderer should check this first and
         * only call waitUntilWindowReady() once it comes back.
         *
         * Always true on platforms without this failure mode (default).
         */
        [[nodiscard]] virtual bool isSurfaceAvailable() const { return true; }

        /**
         * @brief Blocks until getWindowInstance()'s handle is safe to build a
         * graphics surface from, returning false if it never becomes so.
         *
         * On most platforms a created window is immediately usable and this is
         * a no-op. Android is the exception: the OS destroys and re-creates the
         * underlying native window around Activity lifecycle changes (and
         * during the orientation settle at startup), on a *different* thread
         * from the one running the app's main loop. A handle that looks valid
         * can therefore already be released -- and platform Vulkan drivers
         * dereference that freed window rather than failing cleanly, so
         * creating a surface from it is a hard crash, not a recoverable error.
         *
         * Call this before creating a Vulkan/GL surface from
         * getWindowInstance(). It narrows the race rather than eliminating it:
         * the window can still be torn down immediately afterwards, so a
         * renderer that must survive backgrounding still needs to handle
         * surface loss and re-create its surface/swapchain.
         *
         * @return true when the window is present and has held a stable size
         *         long enough to be trusted; false on timeout.
         */
        virtual bool waitUntilWindowReady() { return getWindowInstance() != nullptr; }

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

        //! Per-finger touch input. Distinct from getMouseListener(): SDL can
        //! synthesize mouse events from touch, but that collapses every finger
        //! into one cursor -- see TouchListener's doc comment.
        //!
        //! Not pure virtual: only the SDL backend delivers touch today, and the
        //! desktop-only X11/Wayland backends would otherwise be forced to
        //! implement it for hardware they never see. They inherit this default,
        //! whose listener simply never fires -- bindings on it are harmless
        //! no-ops rather than a compile error or a null dereference.
        virtual TouchListener& getTouchListener() noexcept { return inertTouchListener_; }

    protected:
        TouchListener inertTouchListener_;

    public:
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
