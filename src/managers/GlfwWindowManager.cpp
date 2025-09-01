#include "wma/managers/GlfwWindowManager.hpp"
#include "wma/exceptions/WMAException.hpp"
#include "wma/core/FrameTimer.hpp"

#include <ink/Inkogger.h>

#ifdef WMA_ENABLE_GLFW

#ifdef WMA_ENABLE_OPENGL
#include <glad/gl.h>
#endif

#include <GLFW/glfw3.h>

namespace wma {
    GlfwWindowManager::GlfwWindowManager(const WindowDetails& windowDetails, GraphicsAPI graphicsAPI)
        : window_(nullptr)
        , windowDetails_(windowDetails)
        , windowFlags_{}
        , graphicsAPI_(graphicsAPI)
        , keyboardListener_(std::make_unique<KeyboardListener>())
        , mouseListener_(std::make_unique<MouseListener>())
        , userData_(std::make_unique<GlfwUserData>())
        , windowShouldClose_(false)
    {
        userData_->windowManager = this;
        userData_->keyboardListener = keyboardListener_.get();
        userData_->mouseListener = mouseListener_.get();
        initializeGLFW();
    }

    GlfwWindowManager::~GlfwWindowManager() {
        destroy();
    }

    GlfwWindowManager::GlfwWindowManager(GlfwWindowManager&& other) noexcept
        : window_(other.window_)
        , windowDetails_(std::move(other.windowDetails_))
        , windowFlags_(std::move(other.windowFlags_))
        , graphicsAPI_(other.graphicsAPI_)
        , keyboardListener_(std::move(other.keyboardListener_))
        , mouseListener_(std::move(other.mouseListener_))
        , userData_(std::move(other.userData_))
        , windowShouldClose_(other.windowShouldClose_)
    {
        other.window_ = nullptr;
        if (userData_) {
            userData_->windowManager = this;
            userData_->keyboardListener = keyboardListener_.get();
            userData_->mouseListener = mouseListener_.get();
        }
    }

    GlfwWindowManager& GlfwWindowManager::operator=(GlfwWindowManager&& other) noexcept {
        if (this != &other) {
            // Clean up current resources
            if (window_) {
                glfwDestroyWindow(window_);
            }

            // Move data
            window_ = other.window_;
            windowDetails_ = std::move(other.windowDetails_);
            windowFlags_ = std::move(other.windowFlags_);
            graphicsAPI_ = other.graphicsAPI_;
            keyboardListener_ = std::move(other.keyboardListener_);
            mouseListener_ = std::move(other.mouseListener_);
            userData_ = std::move(other.userData_);
            windowShouldClose_ = other.windowShouldClose_;

            other.window_ = nullptr;

            if (userData_) {

                userData_->windowManager = this;
                userData_->keyboardListener = keyboardListener_.get();
                userData_->mouseListener = mouseListener_.get();

                if (window_) {
                    glfwSetWindowUserPointer(window_, userData_.get());
                }
            }
        }
        return *this;
    }

    void GlfwWindowManager::createWindow(const char* windowName) {
        // Set window hints
        glfwWindowHint(GLFW_RESIZABLE, windowDetails_.resizable ? GLFW_TRUE : GLFW_FALSE);
        
        // Set graphics API specific hints
        switch (graphicsAPI_) {
#ifdef WMA_ENABLE_VULKAN
            case GraphicsAPI::Vulkan:
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
                break;
#endif

#ifdef WMA_ENABLE_OPENGL
            case GraphicsAPI::OpenGL:
                glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
                break;
#endif

            case GraphicsAPI::CPU:
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
                break;
                
            default:
                throw GraphicsException("Unsupported graphics API for GLFW");
        }

        window_ = glfwCreateWindow(
            windowDetails_.width,
            windowDetails_.height,
            windowName,
            windowDetails_.fullscreen ? glfwGetPrimaryMonitor() : nullptr,
            nullptr
        );

        if (!window_) {
            glfwTerminate();
            throw WindowException("Failed to create GLFW window: ");
        }

        // Set window user pointer to this instance
        glfwSetWindowUserPointer(window_, userData_.get());

        // Set callbacks
        glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
        glfwSetWindowFocusCallback(window_, windowFocusCallback);
        glfwSetWindowIconifyCallback(window_, windowIconifyCallback);

        // Initialize graphics context
#ifdef WMA_ENABLE_OPENGL
        if (graphicsAPI_ == GraphicsAPI::OpenGL) {
            glfwMakeContextCurrent(window_);
            
            if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
                glfwDestroyWindow(window_);
                glfwTerminate();
                throw GraphicsException("Failed to load OpenGL functions");
            }

            // Set VSync
            glfwSwapInterval(windowDetails_.vsync ? 1 : 0);
        }
#endif

        // Initialize input listeners
        keyboardListener_->initializeGLFW(window_);
        mouseListener_->initializeGLFW(window_);

        INK_LOG << "GLFW window created: " << windowName;
    }

    void GlfwWindowManager::process(std::function<void()>&& actions) {
        FrameTimer timer(windowFlags_);
        timer.setTargetFPS(windowDetails_.targetFPS);

        while (!windowShouldClose_ && !glfwWindowShouldClose(window_)) {
            glfwPollEvents();

            timer.updateDeltaTime();
            windowFlags_.frameCounter++;
            
            // Reset frame-specific flags
            windowFlags_.resetFrameFlags();
            
            // Execute user actions
            actions();

#ifdef WMA_ENABLE_OPENGL
            if (graphicsAPI_ == GraphicsAPI::OpenGL) {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                glfwSwapBuffers(window_);
            }
#endif

            timer.limitFrameRate();
        }
    }

    void* GlfwWindowManager::getWindowInstance() {
        return window_;
    }

    WindowFlags* GlfwWindowManager::getWindowFlags() noexcept {
        return &windowFlags_;
    }

    const WindowDetails* GlfwWindowManager::getWindowDetails() noexcept {
        return &windowDetails_;
    }

    const std::vector<const char*> GlfwWindowManager::getVulkanExtensions() const {
#ifdef WMA_ENABLE_VULKAN
        u32 extensionCount = 0;
        const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
        
        if (!extensions) {
            throw GraphicsException("Failed to get Vulkan extensions from GLFW");
        }

        return std::vector<const char*>(extensions, extensions + extensionCount);
#else
        throw GraphicsException("Vulkan support not compiled in");
#endif
    }

    KeyboardListener& GlfwWindowManager::getKeyboardListener() noexcept {
        return *keyboardListener_;
    }

    MouseListener& GlfwWindowManager::getMouseListener() noexcept {
        return *mouseListener_;
    }

    const bool GlfwWindowManager::shouldClose() const {
        return windowShouldClose_ || glfwWindowShouldClose(window_);
    }

    WindowBackend GlfwWindowManager::getBackendType() const {
        return WindowBackend::GLFW;
    }

    GraphicsAPI GlfwWindowManager::getGraphicsAPI() const {
        return graphicsAPI_;
    }

    void GlfwWindowManager::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
        auto* userData = static_cast<GlfwUserData*>(glfwGetWindowUserPointer(window));
        if (userData && userData->windowManager) {
            userData->windowManager->windowDetails_.width = width;
            userData->windowManager->windowDetails_.height = height;
            userData->windowManager->windowFlags_.resized = true;
        }
    }

    void GlfwWindowManager::windowFocusCallback(GLFWwindow* window, int focused) {
        auto* userData = static_cast<GlfwUserData*>(glfwGetWindowUserPointer(window));
        if (userData && userData->windowManager) {
            userData->windowManager->windowFlags_.focused = focused == GLFW_TRUE;
        }
    }

    void GlfwWindowManager::windowIconifyCallback(GLFWwindow* window, int iconified) {
        auto* userData = static_cast<GlfwUserData*>(glfwGetWindowUserPointer(window));
        if (userData && userData->windowManager) {
            userData->windowManager->windowFlags_.minimized = iconified == GLFW_TRUE;
        }
    }

    void GlfwWindowManager::initializeGLFW() {
        if (!glfwInit()) {
            throw WMAException("Failed to initialize GLFW");
        }

        // Check for Vulkan support if needed
#ifdef WMA_ENABLE_VULKAN
        if (graphicsAPI_ == GraphicsAPI::Vulkan) {
            if (!glfwVulkanSupported()) {
                glfwTerminate();
                throw GraphicsException("Vulkan is not supported by GLFW");
            }
        }
#endif
        mouseListener_->setSensitivity(1.0); // Default
    }

    GlfwWindowManager* GlfwWindowManager::getInstanceFromWindow(GLFWwindow* window) {
        auto* userData = static_cast<GlfwUserData*>(glfwGetWindowUserPointer(window));
        return userData ? userData->windowManager : nullptr;
    }

    WmaCode GlfwWindowManager::destroy()
    {
        windowShouldClose_ = true;

        if (window_ != nullptr) {
            glfwSetMouseButtonCallback(window_, nullptr);
            glfwSetCursorPosCallback(window_, nullptr);
            glfwSetScrollCallback(window_, nullptr);
            glfwSetKeyCallback(window_, nullptr);
            glfwSetWindowUserPointer(window_, nullptr);

            glfwDestroyWindow(window_);
            window_ = nullptr;
        }

        glfwTerminate();

        return WmaCode::OK;
    }

} // namespace wma

#endif // WMA_ENABLE_GLFW
