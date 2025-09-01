#include "wma/managers/SdlWindowManager.hpp"
#include "wma/core/FrameTimer.hpp"
#include "wma/exceptions/WMAException.hpp"

#ifdef WMA_ENABLE_SDL

#include <SDL2/SDL.h>
#include <iostream>

#ifdef WMA_ENABLE_VULKAN
    #include <SDL2/SDL_vulkan.h>
#endif

#ifdef WMA_ENABLE_OPENGL
    #include <glad/gl.h>
    #include <SDL2/SDL_opengl.h>
#endif

namespace wma {

    SdlWindowManager::SdlWindowManager(const WindowDetails& windowDetails, GraphicsAPI graphicsAPI)
        : window_(nullptr)
        , windowDetails_(windowDetails)
        , windowFlags_{}
        , graphicsAPI_(graphicsAPI)
        , keyboardListener_(std::make_unique<KeyboardListener>())
        , mouseListener_(std::make_unique<MouseListener>())
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
            // Clean up current resources
            if (window_) {
                SDL_DestroyWindow(window_);
            }

            // Move data
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
        window_ = SDL_CreateWindow(
            windowName,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            windowDetails_.width,
            windowDetails_.height,
            SDL_WINDOW_SHOWN | (windowDetails_.resizable ? SDL_WINDOW_RESIZABLE : 0) | 
            (windowDetails_.fullscreen ? SDL_WINDOW_FULLSCREEN : 0) |
            getSDLWindowFlags()
        );

        if (!window_) {
            SDL_Quit();
            throw WindowException("Failed to create SDL window: " + std::string(SDL_GetError()));
        }

        // Set window data
        SDL_SetWindowData(window_, "WindowFlags", &windowFlags_);

        // Initialize graphics context
#ifdef WMA_ENABLE_OPENGL
        if (graphicsAPI_ == GraphicsAPI::OpenGL) {
            SDL_GLContext glContext = SDL_GL_CreateContext(window_);
            if (!glContext) {
                SDL_DestroyWindow(window_);
                SDL_Quit();
                throw GraphicsException("Failed to create OpenGL context: " + std::string(SDL_GetError()));
            }

            if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
                SDL_GL_DeleteContext(glContext);
                SDL_DestroyWindow(window_);
                SDL_Quit();
                throw GraphicsException("Failed to load OpenGL functions");
            }

            // Set VSync
            SDL_GL_SetSwapInterval(windowDetails_.vsync ? 1 : 0);
        }
#endif

        // Initialize keyboard listener
        keyboardListener_->initializeSDL(window_);

        std::cout << "SDL window created: " << windowName << std::endl;
    }

    void SdlWindowManager::process(std::function<void()>&& actions) {
        FrameTimer timer(windowFlags_);
        timer.setTargetFPS(windowDetails_.targetFPS);

        while (!windowShouldClose_) {
            processEvents();

            timer.updateDeltaTime();
            windowFlags_.frameCounter++;
            
            // Reset frame-specific flags
            windowFlags_.resetFrameFlags();
            
            // Execute user actions
            actions();

#ifdef WMA_ENABLE_OPENGL
            if (graphicsAPI_ == GraphicsAPI::OpenGL) {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                SDL_GL_SwapWindow(window_);
            }
#endif

            timer.limitFrameRate();
        }
    }

    u32 SdlWindowManager::getSDLWindowFlags() const {
        u32 flags = 0;

        switch (graphicsAPI_) {
#ifdef WMA_ENABLE_VULKAN
        case GraphicsAPI::Vulkan:
            flags |= SDL_WINDOW_VULKAN;
            break;
#endif

#ifdef WMA_ENABLE_OPENGL
        case GraphicsAPI::OpenGL:
            flags |= SDL_WINDOW_OPENGL;
            break;
#endif

        case GraphicsAPI::CPU:
            // No special flags needed for CPU rendering
            break;

        default:
            throw GraphicsException("Unsupported graphics API for SDL");
        }

        return flags;
    }

    void* SdlWindowManager::getWindowInstance() {
        return window_;
    }

    WindowFlags* SdlWindowManager::getWindowFlags() noexcept {
        return &windowFlags_;
    }

    const WindowDetails* SdlWindowManager::getWindowDetails() noexcept {
        return &windowDetails_;
    }

    const std::vector<const char*> SdlWindowManager::getVulkanExtensions() const {
#ifdef WMA_ENABLE_VULKAN
        u32 extensionCount = 0;
        if (!SDL_Vulkan_GetInstanceExtensions(window_, &extensionCount, nullptr)) {
            throw GraphicsException("Failed to get Vulkan extension count: " + std::string(SDL_GetError()));
        }

        std::vector<const char*> extensions(extensionCount);
        if (!SDL_Vulkan_GetInstanceExtensions(window_, &extensionCount, extensions.data())) {
            throw GraphicsException("Failed to get Vulkan extensions: " + std::string(SDL_GetError()));
        }

        return extensions;
#else
        throw GraphicsException("Vulkan support not compiled in");
#endif
    }

    KeyboardListener& SdlWindowManager::getKeyboardListener() noexcept {
        return *keyboardListener_;
    }

    MouseListener& SdlWindowManager::getMouseListener() noexcept {
        return *mouseListener_;
    }

    const bool SdlWindowManager::shouldClose() const {
        return windowShouldClose_;
    }

    WindowBackend SdlWindowManager::getBackendType() const {
        return WindowBackend::SDL2;
    }

    GraphicsAPI SdlWindowManager::getGraphicsAPI() const {
        return graphicsAPI_;
    }

    void SdlWindowManager::processEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    windowShouldClose_ = true;
                    break;
                    
                case SDL_WINDOWEVENT:
                    handleWindowEvent(event);
                    break;
                    
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    keyboardListener_->handleSDLKeyEvent(event.key);
                    break;

                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEMOTION:
                case SDL_MOUSEWHEEL:
                    mouseListener_->handleSDLEvent(event);
                    break;

                default:
                    break;
            }
        }
    }

    void SdlWindowManager::handleWindowEvent(const SDL_Event& event) {
        switch (event.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                windowDetails_.width = event.window.data1;
                windowDetails_.height = event.window.data2;
                windowFlags_.resized = true;
                break;
                
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                windowFlags_.focused = true;
                break;
                
            case SDL_WINDOWEVENT_FOCUS_LOST:
                windowFlags_.focused = false;
                break;
                
            case SDL_WINDOWEVENT_MINIMIZED:
                windowFlags_.minimized = true;
                break;
                
            case SDL_WINDOWEVENT_RESTORED:
                windowFlags_.minimized = false;
                break;
                
            default:
                break;
        }
    }

    void SdlWindowManager::initializeSDL() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
            throw WMAException("Failed to initialize SDL: " + std::string(SDL_GetError()));
        }

        // Set OpenGL attributes if needed
#ifdef WMA_ENABLE_OPENGL
        if (graphicsAPI_ == GraphicsAPI::OpenGL) {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        }
#endif

        // Load Vulkan library if needed
#ifdef WMA_ENABLE_VULKAN
        if (graphicsAPI_ == GraphicsAPI::Vulkan) {
            if (SDL_Vulkan_LoadLibrary(nullptr) != 0) {
                throw GraphicsException("Failed to load Vulkan library: " + std::string(SDL_GetError()));
            }
        }
#endif
        mouseListener_->setSensitivity(1.0); // Default
    }

    WmaCode SdlWindowManager::destroy()
    {
        windowShouldClose_ = true;

        if (window_) {
            SDL_DestroyWindow(window_);
        }
        SDL_Quit();

        return WmaCode::OK;
    }

} // namespace wma

#endif // WMA_ENABLE_SDL
