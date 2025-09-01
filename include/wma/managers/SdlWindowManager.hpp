#ifndef WMA_MANAGERS_SDL_WINDOW_MANAGER_HPP
#define WMA_MANAGERS_SDL_WINDOW_MANAGER_HPP

#include "IWindowManager.hpp"
#include <memory>

// Forward declarations
struct SDL_Window;
union SDL_Event;
struct SDL_KeyboardEvent;

namespace wma {

    /**
     * @brief SDL2-based window manager implementation
     * 
     * Provides window management using SDL2 backend with support
     * for Vulkan, OpenGL, and CPU rendering.
     */
    class SdlWindowManager : public IWindowManager {
    public:
        /**
         * @brief Construct SDL window manager
         * @param windowDetails Window configuration
         * @param graphicsAPI Graphics API to use
         */
        explicit SdlWindowManager(const WindowDetails& windowDetails, 
                                  GraphicsAPI graphicsAPI = GraphicsAPI::Vulkan);
        
        /**
         * @brief Destructor - cleans up SDL resources
         */
        ~SdlWindowManager() override;
        
        // Non-copyable
        SdlWindowManager(const SdlWindowManager&) = delete;
        SdlWindowManager& operator=(const SdlWindowManager&) = delete;
        
        // Movable
        SdlWindowManager(SdlWindowManager&&) noexcept;
        SdlWindowManager& operator=(SdlWindowManager&&) noexcept;
        
        // IWindowManager interface implementation
        void createWindow(const char* windowName) override;
        void process(std::function<void()>&& actions) override;
        void* getWindowInstance() override;
        u32 getSDLWindowFlags() const;
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
        SDL_Window* window_;
        WindowDetails windowDetails_;
        WindowFlags windowFlags_;
        GraphicsAPI graphicsAPI_;
        std::unique_ptr<KeyboardListener> keyboardListener_;
        std::unique_ptr<MouseListener> mouseListener_;
        bool windowShouldClose_;
        
        // Event handling
        void processEvents();
        void handleWindowEvent(const SDL_Event& event);
        
        // Helper methods
        void initializeSDL();
    };

} // namespace wma

#endif // WMA_MANAGERS_SDL_WINDOW_MANAGER_HPP
