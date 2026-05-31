#ifndef WMA_BACKENDS_WAYLAND_WINDOW_MANAGER_HPP
#define WMA_BACKENDS_WAYLAND_WINDOW_MANAGER_HPP

#include <wayland-client.h>
#include <memory>
#include "WaylandMouseListener.hpp"
#include "WaylandKeyboardListener.hpp"
#include "wma/backends/wayland/protocols/xdg-shell-client-protocol.h"
#include "wma/managers/IWindowManager.hpp"

namespace wma {

class WaylandWindowManager : public IWindowManager {
public:
    explicit WaylandWindowManager(const WindowDetails& windowDetails,
                                  GraphicsAPI graphicsAPI = GraphicsAPI::Vulkan);
    ~WaylandWindowManager() override;

    WaylandWindowManager(const WaylandWindowManager&) = delete;
    WaylandWindowManager& operator=(const WaylandWindowManager&) = delete;
    WaylandWindowManager(WaylandWindowManager&&) noexcept;
    WaylandWindowManager& operator=(WaylandWindowManager&&) noexcept;

    void createWindow(const char* windowName) override;
    void process(std::function<void()>&& actions) override;
    void* getWindowInstance() override;
    WindowFlags* getWindowFlags() noexcept override;
    const WindowDetails* getWindowDetails() noexcept override;
    const std::vector<const char*> getVulkanExtensions() const override;
    KeyboardListener& getKeyboardListener() noexcept override;
    MouseListener& getMouseListener() noexcept override;
    bool shouldClose() const override;
    WindowBackend getBackendType() const override;
    GraphicsAPI getGraphicsAPI() const override;
    WmaCode destroy() override;

    wl_display* getDisplay() const { return display_; }
    wl_surface* getSurface() const { return surface_; }

private:
    wl_display* display_;
    wl_registry* registry_;
    wl_compositor* compositor_;
    wl_surface* surface_;
    wl_seat* seat_;

    xdg_wm_base* xdgWmBase_;
    xdg_surface* xdgSurface_;
    xdg_toplevel* xdgToplevel_;

    wl_keyboard* keyboard_;
    wl_pointer* pointer_;

    WindowDetails windowDetails_;
    WindowFlags windowFlags_;
    GraphicsAPI graphicsAPI_;
    bool windowShouldClose_;

    std::unique_ptr<WaylandKeyboardListener> keyboardListener_;
    std::unique_ptr<WaylandMouseListener> mouseListener_;

    static const wl_registry_listener registryListener_;
    static void handleRegistryGlobal(void* data, wl_registry* registry,
                                     u32 name, const char* interface, u32 version);
    static void handleRegistryGlobalRemove(void* data, wl_registry* registry, u32 name);

    static const wl_seat_listener seatListener_;
    static void handleSeatCapabilities(void* data, wl_seat* seat, u32 capabilities);
    static void handleSeatName(void* data, wl_seat* seat, const char* name);

    static const xdg_wm_base_listener xdgWmBaseListener_;
    static void handleXdgWmBasePing(void* data, xdg_wm_base* xdg_wm_base, u32 serial);

    static const xdg_surface_listener xdgSurfaceListener_;
    static void handleXdgSurfaceConfigure(void* data, xdg_surface* xdg_surface, u32 serial);

    static const xdg_toplevel_listener xdgToplevelListener_;
    static void handleXdgToplevelConfigure(void* data, xdg_toplevel* xdg_toplevel,
                                           i32 width, i32 height, wl_array* states);
    static void handleXdgToplevelClose(void* data, xdg_toplevel* xdg_toplevel);
    static void handleXdgToplevelConfigureBounds(void* data, xdg_toplevel* xdg_toplevel,
                                                 i32 width, i32 height);

    void processEvents();
    void setupInputDevices();
};

} // namespace wma

#endif // WMA_BACKENDS_WAYLAND_WINDOW_MANAGER_HPP
