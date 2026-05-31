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
     * @brief Abstract base interface for window managers
     *
     * Provides a common interface for different windowing backends
     * while maintaining type safety and performance.
     */
    class IWindowManager {
    public:
        virtual ~IWindowManager() = default;

        virtual void createWindow(const char* windowName) = 0;
        virtual void process(std::function<void()>&& actions) = 0;
        virtual void* getWindowInstance() = 0;
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
