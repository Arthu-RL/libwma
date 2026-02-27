#ifndef WMA_MANAGERS_WAYLAND_WINDOW_MANAGER_HPP
#define WMA_MANAGERS_WAYLAND_WINDOW_MANAGER_HPP

#include <wayland-client.h>
#include <memory>
#include "wma/input/mouse/WaylandMouseListener.hpp"
#include "wma/input/keyboard/WaylandKeyboardListener.hpp"
#include "wma/managers/xdg-shell-client-protocol.h"
#include "IWindowManager.hpp"

namespace wma {

/**
 * @brief Wayland-based window manager implementation
 *
 * Provides window management using Wayland protocol with support
 * for Vulkan, OpenGL, and CPU rendering.
 */
class WaylandWindowManager : public IWindowManager {
public:
    /**
     * @brief Construct Wayland window manager
     * @param windowDetails Window configuration
     * @param graphicsAPI Graphics API to use
     */
    explicit WaylandWindowManager(const WindowDetails& windowDetails,
                                  GraphicsAPI graphicsAPI = GraphicsAPI::Vulkan);

    /**
     * @brief Destructor - cleans up Wayland resources
     */
    ~WaylandWindowManager() override;

    // Non-copyable
    WaylandWindowManager(const WaylandWindowManager&) = delete;
    WaylandWindowManager& operator=(const WaylandWindowManager&) = delete;

    // Movable
    WaylandWindowManager(WaylandWindowManager&&) noexcept;
    WaylandWindowManager& operator=(WaylandWindowManager&&) noexcept;

    // IWindowManager interface implementation
    void createWindow(const char* windowName) override;
    void process(std::function<void()>&& actions) override;
    void* getWindowInstance() override;
    WindowFlags* getWindowFlags() noexcept override;
    const WindowDetails* getWindowDetails() noexcept override;
    const std::vector<const char*> getVulkanExtensions() const override;
    KeyboardListener& getKeyboardListener() noexcept override;
    MouseListener& getMouseListener() noexcept override;
    const bool shouldClose() const override;
    WindowBackend getBackendType() const override;
    GraphicsAPI getGraphicsAPI() const override;
    WmaCode destroy() override;

    /**
     * @brief Get the Wayland display
     * @return Pointer to wl_display
     */
    wl_display* getDisplay() const { return display_; }

    /**
     * @brief Get the Wayland surface
     * @return Pointer to wl_surface
     */
    wl_surface* getSurface() const { return surface_; }

private:
    // Core Wayland objects
    wl_display* display_;
    wl_registry* registry_;
    wl_compositor* compositor_;
    wl_surface* surface_;
    wl_seat* seat_;

    // Shell interfaces (XDG shell)
    xdg_wm_base* xdgWmBase_;
    xdg_surface* xdgSurface_;
    xdg_toplevel* xdgToplevel_;

    // Input devices
    wl_keyboard* keyboard_;
    wl_pointer* pointer_;

    // Window state
    WindowDetails windowDetails_;
    WindowFlags windowFlags_;
    GraphicsAPI graphicsAPI_;
    bool windowShouldClose_;

    // Input listeners
    std::unique_ptr<WaylandKeyboardListener> keyboardListener_;
    std::unique_ptr<WaylandMouseListener> mouseListener_;

    // Registry listener for global objects
    static const wl_registry_listener registryListener_;
    static void handleRegistryGlobal(void* data, wl_registry* registry,
                                     uint32_t name, const char* interface, uint32_t version);
    static void handleRegistryGlobalRemove(void* data, wl_registry* registry, uint32_t name);

    // Seat listener for input devices
    static const wl_seat_listener seatListener_;
    static void handleSeatCapabilities(void* data, wl_seat* seat, uint32_t capabilities);
    static void handleSeatName(void* data, wl_seat* seat, const char* name);

    // XDG WM Base (Handles Ping/Pong)
    static const xdg_wm_base_listener xdgWmBaseListener_;
    static void handleXdgWmBasePing(void* data, xdg_wm_base* xdg_wm_base, uint32_t serial);

    // XDG Surface (Handles basic surface configuration)
    static const xdg_surface_listener xdgSurfaceListener_;
    static void handleXdgSurfaceConfigure(void* data, xdg_surface* xdg_surface, uint32_t serial);

    // XDG Toplevel (Handles window resizing and closing)
    static const xdg_toplevel_listener xdgToplevelListener_;
    static void handleXdgToplevelConfigure(void* data, xdg_toplevel* xdg_toplevel, int32_t width, int32_t height, wl_array* states);
    static void handleXdgToplevelClose(void* data, xdg_toplevel* xdg_toplevel);

    // Event processing
    void processEvents();
    void setupInputDevices();
};

} // namespace wma

#endif // WMA_MANAGERS_WAYLAND_WINDOW_MANAGER_HPP
