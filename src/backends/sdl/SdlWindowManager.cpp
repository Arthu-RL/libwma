#include "wma/backends/sdl/SdlWindowManager.hpp"
#include "wma/exceptions/WMAException.hpp"

#include <ink/Inkogger.h>

#ifdef WMA_ENABLE_SDL

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <atomic>
#include <utility>

namespace wma {

//! SDL_Init/SDL_Quit are process-global. Reference-count them so multiple windows
//! can coexist and moving/destroying one never tears SDL down for a live one.
namespace {
    std::atomic<int> g_sdlRefCount{0};
}

    SdlWindowManager::SdlWindowManager(const WindowDetails& windowDetails, GraphicsAPI graphicsAPI)
        : window_(nullptr)
        , glContext_(nullptr)
        , windowSurface_(nullptr)
        , windowDetails_(windowDetails)
        , windowFlags_{}
        , graphicsAPI_(graphicsAPI)
        , keyboardListener_(std::make_unique<SDLKeyboardListener>())
        , mouseListener_(std::make_unique<SDLMouseListener>())
        , windowShouldClose_(false)
        , ownsSubsystem_(false)
    {
        initializeSDL();
    }

    SdlWindowManager::~SdlWindowManager() {
        destroy();
    }

    SdlWindowManager::SdlWindowManager(SdlWindowManager&& other) noexcept
        : window_(std::exchange(other.window_, nullptr))
        , glContext_(std::exchange(other.glContext_, nullptr))
        , windowSurface_(std::exchange(other.windowSurface_, nullptr))
        , windowDetails_(std::move(other.windowDetails_))
        , windowFlags_(std::move(other.windowFlags_))
        , graphicsAPI_(other.graphicsAPI_)
        , keyboardListener_(std::move(other.keyboardListener_))
        , mouseListener_(std::move(other.mouseListener_))
        , windowShouldClose_(other.windowShouldClose_)
        , ownsSubsystem_(std::exchange(other.ownsSubsystem_, false))
    {
    }

    SdlWindowManager& SdlWindowManager::operator=(SdlWindowManager&& other) noexcept {
        if (this != &other) {
            destroy();
            window_ = std::exchange(other.window_, nullptr);
            glContext_ = std::exchange(other.glContext_, nullptr);
            windowSurface_ = std::exchange(other.windowSurface_, nullptr);
            windowDetails_ = std::move(other.windowDetails_);
            windowFlags_ = std::move(other.windowFlags_);
            graphicsAPI_ = other.graphicsAPI_;
            keyboardListener_ = std::move(other.keyboardListener_);
            mouseListener_ = std::move(other.mouseListener_);
            windowShouldClose_ = other.windowShouldClose_;
            ownsSubsystem_ = std::exchange(other.ownsSubsystem_, false);
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

#ifdef __EMSCRIPTEN__
        // Emscripten's OpenGL backend is WebGL2 (Aura3D compiles with
        // -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2, since its ES3-only
        // entry points like glBindBufferBase need it). SDL_GL_CreateContext
        // otherwise requests a default profile that maps to WebGL1, which
        // the browser then refuses outright. Attributes must be set before
        // SDL_CreateWindow when the window will host a GL context.
        if (graphicsAPI_ == GraphicsAPI::OpenGL) {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        }
#endif

        window_ = SDL_CreateWindow(
            windowName,
            windowDetails_.width, windowDetails_.height,
            windowFlags
        );

        if (!window_) {
            throw WindowException("Failed to create SDL window: " + std::string(SDL_GetError()));
        }

        if (graphicsAPI_ == GraphicsAPI::OpenGL) {
            glContext_ = SDL_GL_CreateContext(window_);
            if (!glContext_) {
                SDL_DestroyWindow(window_);
                window_ = nullptr;
                throw GraphicsException("Failed to create OpenGL context: " + std::string(SDL_GetError()));
            }
            SDL_GL_MakeCurrent(window_, static_cast<SDL_GLContext>(glContext_));
            SDL_GL_SetSwapInterval(windowDetails_.vsync ? 1 : 0);
        }

        keyboardListener_->initialize(window_);
        mouseListener_->initialize(window_);
        INK_LOG << "SDL window created: " << windowName;
    }

    void SdlWindowManager::pollEvents() {
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

    void SdlWindowManager::swapBuffers() {
        if (graphicsAPI_ == GraphicsAPI::OpenGL && window_)
            SDL_GL_SwapWindow(window_);
    }

    void* SdlWindowManager::getWindowInstance() { return window_; }

    void* SdlWindowManager::getNativeDisplayHandle() const noexcept { return nullptr; }

    void* SdlWindowManager::getGLProcAddress(const char* name) const {
        if (graphicsAPI_ != GraphicsAPI::OpenGL) 
            return nullptr;
        return reinterpret_cast<void*>(SDL_GL_GetProcAddress(name));
    }

    SoftwareFramebuffer SdlWindowManager::lockFramebuffer() {
        if (graphicsAPI_ != GraphicsAPI::CPU || !window_) 
            return {};

        SDL_Surface* surface = SDL_GetWindowSurface(window_);
        if (!surface) 
            return {};

        windowSurface_ = surface;
        if (SDL_MUSTLOCK(surface)) 
            SDL_LockSurface(surface);

        return SoftwareFramebuffer{ surface->pixels, surface->w, surface->h, surface->pitch };
    }

    void SdlWindowManager::presentFramebuffer() {
        if (!windowSurface_ || !window_) return;
        auto* surface = static_cast<SDL_Surface*>(windowSurface_);
        if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
        SDL_UpdateWindowSurface(window_);
        windowSurface_ = nullptr;
    }

    u64 SdlWindowManager::getSDLWindowFlags() const {
        return window_ ? SDL_GetWindowFlags(window_) : 0;
    }

    WindowFlags* SdlWindowManager::getWindowFlags() noexcept { return &windowFlags_; }
    const WindowDetails* SdlWindowManager::getWindowDetails() noexcept { return &windowDetails_; }

    const std::vector<const char*> SdlWindowManager::getVulkanExtensions() const 
    {
        u32 extensionCount = 0;
        const char* const* extNames = SDL_Vulkan_GetInstanceExtensions(&extensionCount);

        if (!extNames)
            throw GraphicsException("Failed to get Vulkan extensions: " + std::string(SDL_GetError()));

        return std::vector<const char*>(extNames, extNames + extensionCount);
    }

    KeyboardListener& SdlWindowManager::getKeyboardListener() noexcept { return *keyboardListener_; }
    MouseListener& SdlWindowManager::getMouseListener() noexcept { return *mouseListener_; }
    bool SdlWindowManager::shouldClose() const { return windowShouldClose_; }
    WindowBackend SdlWindowManager::getBackendType() const { return WindowBackend::SDL3; }
    GraphicsAPI SdlWindowManager::getGraphicsAPI() const { return graphicsAPI_; }

    void SdlWindowManager::handleWindowEvent(const SDL_Event* event) {
        switch (event->type) 
        {
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
        if (g_sdlRefCount.fetch_add(1, std::memory_order_acq_rel) == 0)
        {
            if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) 
            {
                g_sdlRefCount.fetch_sub(1, std::memory_order_acq_rel);
                throw WMAException("Failed to initialize SDL: " + std::string(SDL_GetError()));
            }
        }
        ownsSubsystem_ = true;

        if (graphicsAPI_ == GraphicsAPI::OpenGL) 
        {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        }

        if (graphicsAPI_ == GraphicsAPI::Vulkan)
        {
            if (!SDL_Vulkan_LoadLibrary(nullptr))
            {
                if (ownsSubsystem_ && g_sdlRefCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
                    SDL_Quit();

                ownsSubsystem_ = false;
                throw GraphicsException("Failed to load Vulkan library: " + std::string(SDL_GetError()));
            }
        }
        mouseListener_->setSensitivity(1.0);
    }

    WmaCode SdlWindowManager::destroy() {
        windowShouldClose_ = true;

        if (glContext_)
        {
            SDL_GL_DestroyContext(static_cast<SDL_GLContext>(glContext_));
            glContext_ = nullptr;
        }

        if (graphicsAPI_ == GraphicsAPI::Vulkan && ownsSubsystem_)
            SDL_Vulkan_UnloadLibrary();

        if (window_)
        {
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }

        if (ownsSubsystem_)
        {
            ownsSubsystem_ = false;
            if (g_sdlRefCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
                SDL_Quit();
        }
        return WmaCode::Ok;
    }

} // namespace wma
#endif // WMA_ENABLE_SDL
