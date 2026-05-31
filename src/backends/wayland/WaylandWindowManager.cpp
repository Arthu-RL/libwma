#include "wma/backends/wayland/WaylandWindowManager.hpp"
#include "wma/core/FrameTimer.hpp"
#include "wma/exceptions/WMAException.hpp"

#include <ink/InkAssert.h>
#include <cstring>

namespace wma {

const wl_registry_listener WaylandWindowManager::registryListener_ = {
    handleRegistryGlobal,
    handleRegistryGlobalRemove
};

const wl_seat_listener WaylandWindowManager::seatListener_ = {
    handleSeatCapabilities,
    handleSeatName
};

const xdg_wm_base_listener WaylandWindowManager::xdgWmBaseListener_ = {
    handleXdgWmBasePing
};

const xdg_surface_listener WaylandWindowManager::xdgSurfaceListener_ = {
    handleXdgSurfaceConfigure
};

const xdg_toplevel_listener WaylandWindowManager::xdgToplevelListener_ = {
    handleXdgToplevelConfigure,
    handleXdgToplevelClose,
    handleXdgToplevelConfigureBounds
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
    INK_ASSERT_MSG(xdgWmBase_ != nullptr,
        "Failed to bind XDG WM Base (compositor must support xdg-shell)");

    surface_ = wl_compositor_create_surface(compositor_);

    xdgSurface_ = xdg_wm_base_get_xdg_surface(xdgWmBase_, surface_);
    xdg_surface_add_listener(xdgSurface_, &xdgSurfaceListener_, this);

    xdgToplevel_ = xdg_surface_get_toplevel(xdgSurface_);
    xdg_toplevel_add_listener(xdgToplevel_, &xdgToplevelListener_, this);

    xdg_toplevel_set_title(xdgToplevel_, windowName);
    xdg_toplevel_set_app_id(xdgToplevel_, "wma_app");

    wl_surface_commit(surface_);
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
    wl_display_dispatch_pending(display_);

    if (wl_display_prepare_read(display_) == 0) {
        wl_display_read_events(display_);
        wl_display_dispatch_pending(display_);
    }
}

void WaylandWindowManager::setupInputDevices()
{
    wl_display_roundtrip(display_);

    if (keyboard_) {
        keyboardListener_->initialize(keyboard_);
    }
    if (pointer_) {
        mouseListener_->initialize(pointer_);
    }
}

void* WaylandWindowManager::getWindowInstance() {
    return static_cast<void*>(surface_);
}

const std::vector<const char*> WaylandWindowManager::getVulkanExtensions() const {
    return {"VK_KHR_surface", "VK_KHR_wayland_surface"};
}

WindowFlags* WaylandWindowManager::getWindowFlags() noexcept { return &windowFlags_; }
const WindowDetails* WaylandWindowManager::getWindowDetails() noexcept { return &windowDetails_; }
KeyboardListener& WaylandWindowManager::getKeyboardListener() noexcept { return *keyboardListener_; }
MouseListener& WaylandWindowManager::getMouseListener() noexcept { return *mouseListener_; }
bool WaylandWindowManager::shouldClose() const { return windowShouldClose_; }
WindowBackend WaylandWindowManager::getBackendType() const { return WindowBackend::WAYLAND; }
GraphicsAPI WaylandWindowManager::getGraphicsAPI() const { return graphicsAPI_; }

WmaCode WaylandWindowManager::destroy()
{
    keyboardListener_.reset();
    mouseListener_.reset();

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

    if (xdgToplevel_) {
        xdg_toplevel_destroy(xdgToplevel_);
        xdgToplevel_ = nullptr;
    }
    if (xdgSurface_) {
        xdg_surface_destroy(xdgSurface_);
        xdgSurface_ = nullptr;
    }
    if (surface_) {
        wl_surface_destroy(surface_);
        surface_ = nullptr;
    }
    if (xdgWmBase_) {
        xdg_wm_base_destroy(xdgWmBase_);
        xdgWmBase_ = nullptr;
    }
    if (compositor_) {
        wl_compositor_destroy(compositor_);
        compositor_ = nullptr;
    }
    if (registry_) {
        wl_registry_destroy(registry_);
        registry_ = nullptr;
    }
    if (display_) {
        wl_display_disconnect(display_);
        display_ = nullptr;
    }

    return WmaCode::OK;
}

void WaylandWindowManager::handleRegistryGlobal(void* data, wl_registry* registry,
                                                u32 name, const char* interface,
                                                u32 version)
{
    auto* manager = static_cast<WaylandWindowManager*>(data);

    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        manager->compositor_ = static_cast<wl_compositor*>(
            wl_registry_bind(registry, name, &wl_compositor_interface, 1));
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        manager->xdgWmBase_ = static_cast<xdg_wm_base*>(
            wl_registry_bind(registry, name, &xdg_wm_base_interface, 1));
        xdg_wm_base_add_listener(manager->xdgWmBase_, &xdgWmBaseListener_, manager);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        manager->seat_ = static_cast<wl_seat*>(
            wl_registry_bind(registry, name, &wl_seat_interface, 1));
        wl_seat_add_listener(manager->seat_, &seatListener_, manager);
    }
}

void WaylandWindowManager::handleRegistryGlobalRemove(void*, wl_registry*, u32)
{
}

void WaylandWindowManager::handleSeatCapabilities(void* data, wl_seat* seat,
                                                  u32 capabilities)
{
    auto* manager = static_cast<WaylandWindowManager*>(data);

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

void WaylandWindowManager::handleSeatName(void*, wl_seat*, const char*)
{
}

void WaylandWindowManager::handleXdgWmBasePing(void*, xdg_wm_base* xdg_wm_base, u32 serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

void WaylandWindowManager::handleXdgSurfaceConfigure(void*, xdg_surface* xdg_surface, u32 serial)
{
    xdg_surface_ack_configure(xdg_surface, serial);
}

void WaylandWindowManager::handleXdgToplevelConfigure(void* data, xdg_toplevel*,
                                                      i32 width, i32 height, wl_array*)
{
    auto* manager = static_cast<WaylandWindowManager*>(data);
    if (width > 0 && height > 0) {
        manager->windowDetails_.width = static_cast<i32>(width);
        manager->windowDetails_.height = static_cast<i32>(height);
        manager->windowFlags_.resized = true;
    }
}

void WaylandWindowManager::handleXdgToplevelClose(void* data, xdg_toplevel*)
{
    auto* manager = static_cast<WaylandWindowManager*>(data);
    manager->windowShouldClose_ = true;
}

void WaylandWindowManager::handleXdgToplevelConfigureBounds(void*, xdg_toplevel*,
                                                            i32, i32)
{
}

} // namespace wma
