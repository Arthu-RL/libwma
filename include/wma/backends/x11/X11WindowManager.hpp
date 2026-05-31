#ifndef WMA_BACKENDS_X11_WINDOW_MANAGER_HPP
#define WMA_BACKENDS_X11_WINDOW_MANAGER_HPP

#include <X11/Xlib.h>
#include <memory>

#include "X11MouseListener.hpp"
#include "X11KeyboardListener.hpp"
#include "wma/managers/IWindowManager.hpp"

namespace wma {

class X11WindowManager : public IWindowManager {
public:
    explicit X11WindowManager(const WindowDetails& windowDetails,
                              GraphicsAPI graphicsAPI = GraphicsAPI::Vulkan);
    ~X11WindowManager() override;

    X11WindowManager(const X11WindowManager&) = delete;
    X11WindowManager& operator=(const X11WindowManager&) = delete;
    X11WindowManager(X11WindowManager&&) noexcept;
    X11WindowManager& operator=(X11WindowManager&&) noexcept;

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

private:
    Display* display_;
    Window window_;
    Colormap colormap_;
    Atom wmDeleteWindow_;

    WindowDetails windowDetails_;
    WindowFlags windowFlags_;
    GraphicsAPI graphicsAPI_;
    std::unique_ptr<X11KeyboardListener> keyboardListener_;
    std::unique_ptr<X11MouseListener> mouseListener_;
    bool windowShouldClose_;

    void processEvents();
    void handleWindowEvent(const XEvent* event);
};

} // namespace wma

#endif // WMA_BACKENDS_X11_WINDOW_MANAGER_HPP
