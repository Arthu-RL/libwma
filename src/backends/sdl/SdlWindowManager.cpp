#include "wma/backends/sdl/SdlWindowManager.hpp"
#include "wma/exceptions/WMAException.hpp"
#include "wma/core/FrameTimer.hpp"

#include <ink/Inkogger.h>

#ifdef WMA_ENABLE_SDL

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace wma {

    SdlWindowManager::SdlWindowManager(const WindowDetails& windowDetails, GraphicsAPI graphicsAPI)
        : window_(nullptr)
        , windowDetails_(windowDetails)
        , windowFlags_{}
        , graphicsAPI_(graphicsAPI)
        , keyboardListener_(std::make_unique<SDLKeyboardListener>())
        , mouseListener_(std::make_unique<SDLMouseListener>())
        , windowShouldClose_(false)
    {
        initializeSDL();
    }

    SdlWindowManager::~SdlWindowManager() {
        destroy();
    }

    SdlWindowManager::SdlWindowManager(SdlWindowManager&& other) noexcept
        : window_(other.window_)
        , windowDetails_(std::move(other.windowDetails_))
        , windowFlags_(std::move(other.windowFlags_))
        , graphicsAPI_(other.graphicsAPI_)
        , keyboardListener_(std::move(other.keyboardListener_))
        , mouseListener_(std::move(other.mouseListener_))
        , windowShouldClose_(other.windowShouldClose_)
    {
        other.window_ = nullptr;
    }

    SdlWindowManager& SdlWindowManager::operator=(SdlWindowManager&& other) noexcept {
        if (this != &other) {
            destroy();
            window_ = other.window_;
            windowDetails_ = std::move(other.windowDetails_);
            windowFlags_ = std::move(other.windowFlags_);
            graphicsAPI_ = other.graphicsAPI_;
            keyboardListener_ = std::move(other.keyboardListener_);
            mouseListener_ = std::move(other.mouseListener_);
            windowShouldClose_ = other.windowShouldClose_;
            other.window_ = nullptr;
        }
        return *this;
    }

    void SdlWindowManager::createWindow(const char* windowName) {
        SDL_WindowFlags windowFlags = 0;
        if (windowDetails_.resizable) windowFlags |= SDL_WINDOW_RESIZABLE;
        if (windowDetails_.fullscreen) windowFlags |= SDL_WINDOW_FULLSCREEN;

        switch (graphicsAPI_) {
            case GraphicsAPI::Vulkan:
                windowFlags |= SDL_WINDOW_VULKAN;
                break;
            case GraphicsAPI::OpenGL:
                windowFlags |= SDL_WINDOW_OPENGL;
                break;
            case GraphicsAPI::CPU:
                break;
            default:
                throw GraphicsException("Unsupported graphics API for SDL");
        }

        window_ = SDL_CreateWindow(
            windowName,
            windowDetails_.width, windowDetails_.height,
            windowFlags
        );

        if (!window_) {
            throw WindowException("Failed to create SDL window: " + std::string(SDL_GetError()));
        }

        if (graphicsAPI_ == GraphicsAPI::OpenGL) {
            auto context = SDL_GL_CreateContext(window_);
            if (!context) {
                SDL_DestroyWindow(window_);
                throw GraphicsException("Failed to create OpenGL context: " + std::string(SDL_GetError()));
            }
            SDL_GL_SetSwapInterval(windowDetails_.vsync ? 1 : 0);
        }

        keyboardListener_->initialize(window_);
        mouseListener_->initialize(window_);
        INK_LOG << "SDL window created: " << windowName;
    }

    void SdlWindowManager::process(std::function<void()>&& actions) {
        FrameTimer timer(windowFlags_);
        timer.setTargetFPS(windowDetails_.targetFPS);

        while (!windowShouldClose_) {
            timer.updateDeltaTime();
            processEvents();
            actions();

            if (graphicsAPI_ == GraphicsAPI::OpenGL) {
                SDL_GL_SwapWindow(window_);
            }
            timer.limitFrameRate();
        }
    }

    void* SdlWindowManager::getWindowInstance() { return window_; }

    u64 SdlWindowManager::getSDLWindowFlags() const {
        return window_ ? SDL_GetWindowFlags(window_) : 0;
    }

    WindowFlags* SdlWindowManager::getWindowFlags() noexcept { return &windowFlags_; }
    const WindowDetails* SdlWindowManager::getWindowDetails() noexcept { return &windowDetails_; }

    const std::vector<const char*> SdlWindowManager::getVulkanExtensions() const {
        u32 extensionCount = 0;
        const char* const* extNames = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
        if (!extNames) {
            throw GraphicsException("Failed to get Vulkan extensions: " + std::string(SDL_GetError()));
        }
        return std::vector<const char*>(extNames, extNames + extensionCount);
    }

    KeyboardListener& SdlWindowManager::getKeyboardListener() noexcept { return *keyboardListener_; }
    MouseListener& SdlWindowManager::getMouseListener() noexcept { return *mouseListener_; }
    bool SdlWindowManager::shouldClose() const { return windowShouldClose_; }
    WindowBackend SdlWindowManager::getBackendType() const { return WindowBackend::SDL3; }
    GraphicsAPI SdlWindowManager::getGraphicsAPI() const { return graphicsAPI_; }

    void SdlWindowManager::processEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    windowShouldClose_ = true;
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                case SDL_EVENT_WINDOW_FOCUS_GAINED:
                case SDL_EVENT_WINDOW_FOCUS_LOST:
                case SDL_EVENT_WINDOW_MINIMIZED:
                case SDL_EVENT_WINDOW_RESTORED:
                    handleWindowEvent(&event);
                    break;
                case SDL_EVENT_KEY_DOWN:
                case SDL_EVENT_KEY_UP:
                    keyboardListener_->handleKeyEvent(event.key);
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                case SDL_EVENT_MOUSE_BUTTON_UP:
                case SDL_EVENT_MOUSE_MOTION:
                case SDL_EVENT_MOUSE_WHEEL:
                    mouseListener_->handleEvent(event);
                    break;
                default:
                    break;
            }
        }
    }

    void SdlWindowManager::handleWindowEvent(const SDL_Event* event) {
        switch (event->type) {
            case SDL_EVENT_WINDOW_RESIZED:
                windowDetails_.width = event->window.data1;
                windowDetails_.height = event->window.data2;
                windowFlags_.resized = true;
                break;
            case SDL_EVENT_WINDOW_FOCUS_GAINED:
                windowFlags_.focused = true;
                break;
            case SDL_EVENT_WINDOW_FOCUS_LOST:
                windowFlags_.focused = false;
                break;
            case SDL_EVENT_WINDOW_MINIMIZED:
                windowFlags_.minimized = true;
                break;
            case SDL_EVENT_WINDOW_RESTORED:
                windowFlags_.minimized = false;
                break;
            default:
                break;
        }
    }

    void SdlWindowManager::initializeSDL() {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
            throw WMAException("Failed to initialize SDL: " + std::string(SDL_GetError()));
        }
        if (graphicsAPI_ == GraphicsAPI::OpenGL) {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        }
        if (graphicsAPI_ == GraphicsAPI::Vulkan) {
            if (!SDL_Vulkan_LoadLibrary(nullptr)) {
                throw GraphicsException("Failed to load Vulkan library: " + std::string(SDL_GetError()));
            }
        }
        mouseListener_->setSensitivity(1.0);
    }

    WmaCode SdlWindowManager::destroy() {
        windowShouldClose_ = true;
        if (graphicsAPI_ == GraphicsAPI::OpenGL) {
            SDL_GL_DestroyContext(SDL_GL_GetCurrentContext());
        }
        if (window_) {
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }
        SDL_Quit();
        return WmaCode::OK;
    }

} // namespace wma
#endif // WMA_ENABLE_SDL
