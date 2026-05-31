#include "wma/backends/glfw/GlfwWindowManager.hpp"
#include "wma/exceptions/WMAException.hpp"
#include "wma/core/FrameTimer.hpp"

#include <ink/Inkogger.h>

#ifdef WMA_ENABLE_GLFW

#ifdef WMA_ENABLE_OPENGL
#include <glad/glad.h>
#endif

#include <GLFW/glfw3.h>

namespace wma {
    GlfwWindowManager::GlfwWindowManager(const WindowDetails& windowDetails, GraphicsAPI graphicsAPI)
        : window_(nullptr)
        , windowDetails_(windowDetails)
        , windowFlags_{}
        , graphicsAPI_(graphicsAPI)
        , keyboardListener_(std::make_unique<GLFWKeyboardListener>())
        , mouseListener_(std::make_unique<GLFWMouseListener>())
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
            if (window_) {
                glfwDestroyWindow(window_);
            }
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
        glfwWindowHint(GLFW_RESIZABLE, windowDetails_.resizable ? GLFW_TRUE : GLFW_FALSE);

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
            windowDetails_.width, windowDetails_.height,
            windowName,
            windowDetails_.fullscreen ? glfwGetPrimaryMonitor() : nullptr,
            nullptr
        );

        if (!window_) {
            glfwTerminate();
            throw WindowException("Failed to create GLFW window");
        }

        glfwSetWindowUserPointer(window_, userData_.get());
        glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
        glfwSetWindowFocusCallback(window_, windowFocusCallback);
        glfwSetWindowIconifyCallback(window_, windowIconifyCallback);

#ifdef WMA_ENABLE_OPENGL
        if (graphicsAPI_ == GraphicsAPI::OpenGL) {
            glfwMakeContextCurrent(window_);
            if (!GLADloadproc((GLADloadproc)glfwGetProcAddress)) {
                glfwDestroyWindow(window_);
                glfwTerminate();
                throw GraphicsException("Failed to load OpenGL functions");
            }
            glfwSwapInterval(windowDetails_.vsync ? 1 : 0);
        }
#endif

        keyboardListener_->initialize(window_);
        mouseListener_->initialize(window_);

        INK_LOG << "GLFW window created: " << windowName;
    }

    void GlfwWindowManager::process(std::function<void()>&& actions) {
        FrameTimer timer(windowFlags_);
        timer.setTargetFPS(windowDetails_.targetFPS);

        while (!windowShouldClose_ && !glfwWindowShouldClose(window_)) {
            glfwPollEvents();
            timer.updateDeltaTime();
            actions();

            if (graphicsAPI_ == GraphicsAPI::OpenGL) {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                glfwSwapBuffers(window_);
            }

            timer.limitFrameRate();
        }
    }

    void* GlfwWindowManager::getWindowInstance() { return window_; }
    WindowFlags* GlfwWindowManager::getWindowFlags() noexcept { return &windowFlags_; }
    const WindowDetails* GlfwWindowManager::getWindowDetails() noexcept { return &windowDetails_; }

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

    KeyboardListener& GlfwWindowManager::getKeyboardListener() noexcept { return *keyboardListener_; }
    MouseListener& GlfwWindowManager::getMouseListener() noexcept { return *mouseListener_; }

    bool GlfwWindowManager::shouldClose() const {
        return windowShouldClose_ || glfwWindowShouldClose(window_);
    }

    WindowBackend GlfwWindowManager::getBackendType() const { return WindowBackend::GLFW; }
    GraphicsAPI GlfwWindowManager::getGraphicsAPI() const { return graphicsAPI_; }

    WmaCode GlfwWindowManager::destroy() {
        windowShouldClose_ = true;
        if (window_) {
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }
        glfwTerminate();
        return WmaCode::OK;
    }

    void GlfwWindowManager::initializeGLFW() {
        if (!glfwInit()) {
            throw WMAException("Failed to initialize GLFW");
        }
    }

    void GlfwWindowManager::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
        auto* instance = getInstanceFromWindow(window);
        if (instance) {
            instance->windowDetails_.width = width;
            instance->windowDetails_.height = height;
            instance->windowFlags_.resized = true;
        }
    }

    void GlfwWindowManager::windowFocusCallback(GLFWwindow* window, int focused) {
        auto* instance = getInstanceFromWindow(window);
        if (instance) {
            instance->windowFlags_.focused = (focused == GLFW_TRUE);
        }
    }

    void GlfwWindowManager::windowIconifyCallback(GLFWwindow* window, int iconified) {
        auto* instance = getInstanceFromWindow(window);
        if (instance) {
            instance->windowFlags_.minimized = (iconified == GLFW_TRUE);
        }
    }

    GlfwWindowManager* GlfwWindowManager::getInstanceFromWindow(GLFWwindow* window) {
        if (!window) return nullptr;
        auto* userData = static_cast<GlfwUserData*>(glfwGetWindowUserPointer(window));
        return userData ? userData->windowManager : nullptr;
    }

} // namespace wma
#endif // WMA_ENABLE_GLFW
