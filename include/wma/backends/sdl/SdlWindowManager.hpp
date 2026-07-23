#ifndef WMA_BACKENDS_SDL_WINDOW_MANAGER_HPP
#define WMA_BACKENDS_SDL_WINDOW_MANAGER_HPP

#include <memory>

#include "SDLMouseListener.hpp"
#include "SDLKeyboardListener.hpp"
#include "wma/managers/IWindowManager.hpp"

struct SDL_Window;
union SDL_Event;
struct SDL_KeyboardEvent;

namespace wma {

    class SdlWindowManager : public IWindowManager {
    public:
        explicit SdlWindowManager(const WindowDetails& windowDetails,
                                  GraphicsAPI graphicsAPI = GraphicsAPI::Vulkan);
        ~SdlWindowManager() override;

        SdlWindowManager(const SdlWindowManager&) = delete;
        SdlWindowManager& operator=(const SdlWindowManager&) = delete;
        SdlWindowManager(SdlWindowManager&&) noexcept;
        SdlWindowManager& operator=(SdlWindowManager&&) noexcept;

        void createWindow(const char* windowName) override;
        void pollEvents() override;
        void swapBuffers() override;
        void* getWindowInstance() override;
        void* getNativeDisplayHandle() const noexcept override;
        void* getGLProcAddress(const char* name) const override;
        SoftwareFramebuffer lockFramebuffer() override;
        void presentFramebuffer() override;
        u64 getSDLWindowFlags() const;
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
        SDL_Window* window_;
        void* glContext_; //!< SDL_GLContext (opaque) — OpenGL mode only
        void* windowSurface_; //!< SDL_Surface* held between lock/present (CPU mode)
        WindowDetails windowDetails_;
        WindowFlags windowFlags_;
        GraphicsAPI graphicsAPI_;
        std::unique_ptr<SDLKeyboardListener> keyboardListener_;
        std::unique_ptr<SDLMouseListener> mouseListener_;
        bool windowShouldClose_;
        bool ownsSubsystem_; //!< this instance holds a reference to the SDL init

        void handleWindowEvent(const SDL_Event* event);
        void initializeSDL();
    };

} // namespace wma

#endif // WMA_BACKENDS_SDL_WINDOW_MANAGER_HPP
