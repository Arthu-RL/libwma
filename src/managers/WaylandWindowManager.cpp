#include "wma/managers/WaylandWindowManager.hpp"
#include "wma/core/FrameTimer.hpp"
#include "wma/exceptions/WMAException.hpp"

#include <ink/InkAssert.h>
#include <cstring>

namespace wma {

// Registry Listener (Order: global, global_remove)
const wl_registry_listener WaylandWindowManager::registryListener_ = {
    handleRegistryGlobal,
    handleRegistryGlobalRemove
};

// Seat Listener (Order: capabilities, name)
const wl_seat_listener WaylandWindowManager::seatListener_ = {
    handleSeatCapabilities,
    handleSeatName
};

// XDG WM Base (Order: ping)
const xdg_wm_base_listener WaylandWindowManager::xdgWmBaseListener_ = {
    handleXdgWmBasePing
};

// XDG Surface (Order: configure)
const xdg_surface_listener WaylandWindowManager::xdgSurfaceListener_ = {
    handleXdgSurfaceConfigure
};

// XDG Toplevel (Order: configure, close)
// Note: If your xdg-shell protocol is very new (v4+), there might be a
// 'configure_bounds' member 3rd. C++ will auto-set it to nullptr if omitted here.
const xdg_toplevel_listener WaylandWindowManager::xdgToplevelListener_ = {
    handleXdgToplevelConfigure,
    handleXdgToplevelClose
};

WaylandWindowManager::WaylandWindowManager(const WindowDetails& windowDetails,
                                           GraphicsAPI graphicsAPI)
    : display_(nullptr)
    , registry_(nullptr)
    , compositor_(nullptr)
    , surface_(nullptr)
    , seat_(nullptr)
    , xdgWmBase_(nullptr)
    , xdgSurface_(nullptr)
    , xdgToplevel_(nullptr)
    , keyboard_(nullptr)
    , pointer_(nullptr)
    , windowDetails_(windowDetails)
    , windowFlags_{}
    , graphicsAPI_(graphicsAPI)
    , windowShouldClose_(false)
    , keyboardListener_(std::make_unique<WaylandKeyboardListener>())
    , mouseListener_(std::make_unique<WaylandMouseListener>())
{
}

WaylandWindowManager::~WaylandWindowManager()
{
    destroy();
}

WaylandWindowManager::WaylandWindowManager(WaylandWindowManager&& other) noexcept
    : display_(other.display_)
    , registry_(other.registry_)
    , compositor_(other.compositor_)
    , surface_(other.surface_)
    , seat_(other.seat_)
    , xdgWmBase_(other.xdgWmBase_)
    , xdgSurface_(other.xdgSurface_)
    , xdgToplevel_(other.xdgToplevel_)
    , keyboard_(other.keyboard_)
    , pointer_(other.pointer_)
    , windowDetails_(other.windowDetails_)
    , windowFlags_(other.windowFlags_)
    , graphicsAPI_(other.graphicsAPI_)
    , windowShouldClose_(other.windowShouldClose_)
    , keyboardListener_(std::move(other.keyboardListener_))
    , mouseListener_(std::move(other.mouseListener_))
{
    other.display_ = nullptr;
    other.registry_ = nullptr;
    other.compositor_ = nullptr;
    other.surface_ = nullptr;
    other.seat_ = nullptr;
    other.xdgWmBase_ = nullptr;
    other.xdgSurface_ = nullptr;
    other.xdgToplevel_ = nullptr;
    other.keyboard_ = nullptr;
    other.pointer_ = nullptr;
}

WaylandWindowManager& WaylandWindowManager::operator=(WaylandWindowManager&& other) noexcept
{
    if (this != &other) {
        destroy();

        display_ = other.display_;
        registry_ = other.registry_;
        compositor_ = other.compositor_;
        surface_ = other.surface_;
        seat_ = other.seat_;
        xdgWmBase_ = other.xdgWmBase_;
        xdgSurface_ = other.xdgSurface_;
        xdgToplevel_ = other.xdgToplevel_;
        keyboard_ = other.keyboard_;
        pointer_ = other.pointer_;
        windowDetails_ = other.windowDetails_;
        windowFlags_ = other.windowFlags_;
        graphicsAPI_ = other.graphicsAPI_;
        windowShouldClose_ = other.windowShouldClose_;
        keyboardListener_ = std::move(other.keyboardListener_);
        mouseListener_ = std::move(other.mouseListener_);

        other.display_ = nullptr;
        other.registry_ = nullptr;
        other.compositor_ = nullptr;
        other.surface_ = nullptr;
        other.seat_ = nullptr;
        other.xdgWmBase_ = nullptr;
        other.xdgSurface_ = nullptr;
        other.xdgToplevel_ = nullptr;
        other.keyboard_ = nullptr;
        other.pointer_ = nullptr;
    }
    return *this;
}

void WaylandWindowManager::createWindow(const char* windowName)
{
    display_ = wl_display_connect(nullptr);
    INK_ASSERT_MSG(display_ != nullptr, "Failed to connect to Wayland display");

    registry_ = wl_display_get_registry(display_);
    wl_registry_add_listener(registry_, &registryListener_, this);

    wl_display_roundtrip(display_);

    INK_ASSERT_MSG(compositor_ != nullptr, "Failed to bind Compositor");
    INK_ASSERT_MSG(xdgWmBase_ != nullptr, "Failed to bind XDG WM Base (Compositor might handle wl_shell but we need xdg-shell)");

    // Create Surface
    surface_ = wl_compositor_create_surface(compositor_);

    // Create XDG Surface
    xdgSurface_ = xdg_wm_base_get_xdg_surface(xdgWmBase_, surface_);
    xdg_surface_add_listener(xdgSurface_, &xdgSurfaceListener_, this);

    // Create XDG Toplevel (the actual window)
    xdgToplevel_ = xdg_surface_get_toplevel(xdgSurface_);
    xdg_toplevel_add_listener(xdgToplevel_, &xdgToplevelListener_, this);

    // Set Title and App ID
    xdg_toplevel_set_title(xdgToplevel_, windowName);
    xdg_toplevel_set_app_id(xdgToplevel_, "wma_app");

    // Commit surface to trigger the initial configure event
    wl_surface_commit(surface_);

    // Wait for the compositor to configure our window
    wl_display_roundtrip(display_);

    if (seat_) setupInputDevices();
}

void WaylandWindowManager::process(std::function<void()>&& actions)
{
    FrameTimer timer(windowFlags_);
    timer.setTargetFPS(windowDetails_.targetFPS);

    while (!windowShouldClose_) {
        timer.updateDeltaTime();

        processEvents();

        actions();

        timer.limitFrameRate();
    }
}

void WaylandWindowManager::processEvents()
{
    // Dispatch pending events
    wl_display_dispatch_pending(display_);

    // Read events from the display
    if (wl_display_prepare_read(display_) == 0) {
        wl_display_read_events(display_);
        wl_display_dispatch_pending(display_);
    }
}

void WaylandWindowManager::setupInputDevices()
{
    // Wait for seat capabilities
    wl_display_roundtrip(display_);

    // Initialize keyboard listener if keyboard is available
    if (keyboard_) {
        keyboardListener_->initialize(keyboard_);
    }

    // Initialize mouse listener if pointer is available
    if (pointer_) {
        mouseListener_->initialize(pointer_);
    }
}

void* WaylandWindowManager::getWindowInstance()
{
    return static_cast<void*>(surface_);
}

const std::vector<const char*> WaylandWindowManager::getVulkanExtensions() const
{
    return {"VK_KHR_surface", "VK_KHR_wayland_surface"};
}

WindowFlags* WaylandWindowManager::getWindowFlags() noexcept
{
    return &windowFlags_;
}

const WindowDetails* WaylandWindowManager::getWindowDetails() noexcept
{
    return &windowDetails_;
}

KeyboardListener& WaylandWindowManager::getKeyboardListener() noexcept
{
    return *keyboardListener_;
}

MouseListener& WaylandWindowManager::getMouseListener() noexcept
{
    return *mouseListener_;
}

const bool WaylandWindowManager::shouldClose() const
{
    return windowShouldClose_;
}

WindowBackend WaylandWindowManager::getBackendType() const
{
    return WindowBackend::WAYLAND;
}

GraphicsAPI WaylandWindowManager::getGraphicsAPI() const
{
    return graphicsAPI_;
}

WmaCode WaylandWindowManager::destroy()
{
    // Clean up input listeners first
    keyboardListener_.reset();
    mouseListener_.reset();

    // Destroy input objects
    if (keyboard_) {
        wl_keyboard_destroy(keyboard_);
        keyboard_ = nullptr;
    }

    if (pointer_) {
        wl_pointer_destroy(pointer_);
        pointer_ = nullptr;
    }

    if (seat_) {
        wl_seat_destroy(seat_);
        seat_ = nullptr;
    }

    // Destroy surface
    if (surface_) {
        wl_surface_destroy(surface_);
        surface_ = nullptr;
    }

    if (xdgToplevel_) {
        xdg_toplevel_destroy(xdgToplevel_);
        xdgToplevel_ = nullptr;
    }
    if (xdgSurface_) {
        xdg_surface_destroy(xdgSurface_);
        xdgSurface_ = nullptr;
    }
    if (xdgWmBase_) {
        xdg_wm_base_destroy(xdgWmBase_);
        xdgWmBase_ = nullptr;
    }

    // Destroy compositor
    if (compositor_) {
        wl_compositor_destroy(compositor_);
        compositor_ = nullptr;
    }

    // Destroy registry
    if (registry_) {
        wl_registry_destroy(registry_);
        registry_ = nullptr;
    }

    // Disconnect from display
    if (display_) {
        wl_display_disconnect(display_);
        display_ = nullptr;
    }

    return WmaCode::OK;
}

// Static callback implementations
void WaylandWindowManager::handleRegistryGlobal(void* data, wl_registry* registry,
                                                uint32_t name, const char* interface,
                                                uint32_t version)
{
    auto* manager = static_cast<WaylandWindowManager*>(data);

    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        manager->compositor_ = static_cast<wl_compositor*>(
            wl_registry_bind(registry, name, &wl_compositor_interface, 1)
            );
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        manager->xdgWmBase_ = static_cast<xdg_wm_base*>(
            wl_registry_bind(registry, name, &xdg_wm_base_interface, 1));

        xdg_wm_base_add_listener(manager->xdgWmBase_, &xdgWmBaseListener_, manager);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        manager->seat_ = static_cast<wl_seat*>(
            wl_registry_bind(registry, name, &wl_seat_interface, 1)
            );
        wl_seat_add_listener(manager->seat_, &seatListener_, manager);
    }
}

void WaylandWindowManager::handleRegistryGlobalRemove(void* data, wl_registry* registry,
                                                      uint32_t name)
{
    // Handle removal of global objects if needed
}

void WaylandWindowManager::handleSeatCapabilities(void* data, wl_seat* seat,
                                                  uint32_t capabilities)
{
    auto* manager = static_cast<WaylandWindowManager*>(data);

    // Check for keyboard capability
    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
        if (!manager->keyboard_) {
            manager->keyboard_ = wl_seat_get_keyboard(seat);
        }
    } else {
        if (manager->keyboard_) {
            wl_keyboard_destroy(manager->keyboard_);
            manager->keyboard_ = nullptr;
        }
    }

    // Check for pointer capability
    if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
        if (!manager->pointer_) {
            manager->pointer_ = wl_seat_get_pointer(seat);
        }
    } else {
        if (manager->pointer_) {
            wl_pointer_destroy(manager->pointer_);
            manager->pointer_ = nullptr;
        }
    }
}

void WaylandWindowManager::handleSeatName(void* data, wl_seat* seat, const char* name)
{
    // Seat name received, can be stored if needed
}

void WaylandWindowManager::handleXdgWmBasePing(void* data, xdg_wm_base* xdg_wm_base, uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

void WaylandWindowManager::handleXdgSurfaceConfigure(void* data, xdg_surface* xdg_surface, uint32_t serial)
{
    // auto* manager = static_cast<WaylandWindowManager*>(data);
    xdg_surface_ack_configure(xdg_surface, serial);

    // For Vulkan/OpenGL swapchain, you usually resize it here
    // or set a flag to resize it on the next frame.
}

void WaylandWindowManager::handleXdgToplevelConfigure(void* data, xdg_toplevel* xdg_toplevel,
                                                      int32_t width, int32_t height, wl_array* states)
{
    auto* manager = static_cast<WaylandWindowManager*>(data);

    // Width/Height will be 0 if the compositor wants us to decide the size ourselves
    if (width > 0 && height > 0) {
        manager->windowDetails_.width = static_cast<u32>(width);
        manager->windowDetails_.height = static_cast<u32>(height);
        manager->windowFlags_.resized = true;
    }
}

void WaylandWindowManager::handleXdgToplevelClose(void* data, xdg_toplevel* xdg_toplevel)
{
    auto* manager = static_cast<WaylandWindowManager*>(data);
    manager->windowShouldClose_ = true;
}

} // namespace wma
