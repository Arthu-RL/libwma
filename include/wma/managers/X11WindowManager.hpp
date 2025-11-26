#ifndef WMA_MANAGERS_X11WINDOWMANAGER_HPP
#define WMA_MANAGERS_X11WINDOWMANAGER_HPP

#include "IWindowManager.hpp"

#include <X11/Xlib.h>
#include <memory>

namespace wma {

/**
 * @brief X11-based window manager implementation
 *
 * Provides window management using X11 backend with support
 * for Vulkan, OpenGL, and CPU rendering.
 */
class X11WindowManager : public IWindowManager
{
public:
    /**
     * @brief Construct X11 window manager
     * @param windowDetails Window configuration
     * @param graphicsAPI Graphics API to use
     */
    explicit X11WindowManager(const WindowDetails& windowDetails,
                              GraphicsAPI graphicsAPI = GraphicsAPI::Vulkan);

    /**
     * @brief Destructor - cleans up SDL resources
     */
    ~X11WindowManager() override;

    // Non-copyable
    X11WindowManager(const X11WindowManager&) = delete;
    X11WindowManager& operator=(const X11WindowManager&) = delete;

    // Movable
    X11WindowManager(X11WindowManager&&) noexcept;
    X11WindowManager& operator=(X11WindowManager&&) noexcept;

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
private:
    Display* display_;
    Window window_;
    Colormap colormap_;
    Atom wmDeleteWindow_;

    WindowDetails windowDetails_;
    WindowFlags windowFlags_;
    GraphicsAPI graphicsAPI_;
    std::unique_ptr<KeyboardListener> keyboardListener_;
    std::unique_ptr<MouseListener> mouseListener_;
    bool windowShouldClose_;
};

}

#endif // X11WINDOWMANAGER_H
