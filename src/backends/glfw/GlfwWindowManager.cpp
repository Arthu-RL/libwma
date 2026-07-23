#include "wma/backends/glfw/GlfwWindowManager.hpp"
#include "wma/exceptions/WMAException.hpp"

#include <ink/Inkogger.h>

#ifdef WMA_ENABLE_GLFW

#include <GLFW/glfw3.h>

#include <atomic>

namespace wma {

//! glfwInit()/glfwTerminate() are process-global; re!ference-count so multiple
//! windows coexist and destroying one never terminates GLFW for a live one.
namespace {
    std::atomic<int> g_glfwRefCount{0};
}

    GlfwWindowManager::GlfwWindowManager(const WindowDetails& windowDetails, GraphicsAPI graphicsAPI)
        : window_(nullptr)
        , windowDetails_(windowDetails)
        , windowFlags_{}
        , graphicsAPI_(graphicsAPI)
        , keyboardListener_(std::make_unique<GLFWKeyboardListener>())
        , mouseListener_(std::make_unique<GLFWMouseListener>())
        , userData_(std::make_unique<GlfwUserData>())
        , windowShouldClose_(false)
        , ownsInit_(false)
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
        , ownsInit_(other.ownsInit_)
    {
        other.window_ = nullptr;
        other.ownsInit_ = false;
        if (userData_) {
            userData_->windowManager = this;
            userData_->keyboardListener = keyboardListener_.get();
            userData_->mouseListener = mouseListener_.get();
        }
    }

    GlfwWindowManager& GlfwWindowManager::operator=(GlfwWindowManager&& other) noexcept {
        if (this != &other) {
            destroy();
            window_ = other.window_;
            windowDetails_ = std::move(other.windowDetails_);
            windowFlags_ = std::move(other.windowFlags_);
            graphicsAPI_ = other.graphicsAPI_;
            keyboardListener_ = std::move(other.keyboardListener_);
            mouseListener_ = std::move(other.mouseListener_);
            userData_ = std::move(other.userData_);
            windowShouldClose_ = other.windowShouldClose_;
            ownsInit_ = other.ownsInit_;
            other.window_ = nullptr;
            other.ownsInit_ = false;
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
            case GraphicsAPI::Vulkan:
                if (!glfwVulkanSupported()) {
                    throw GraphicsException("GLFW reports Vulkan is not supported on this system");
                }
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
                break;
            case GraphicsAPI::OpenGL:
                glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
                break;
            case GraphicsAPI::CPU:
                //! GLFW has no per-OS software-blit path (unlike SDL3/X11/Wayland,
                //! which implement lockFramebuffer()/presentFramebuffer() for real).
                //! Fail fast instead of opening a window nothing can draw into.
                throw GraphicsException(
                    "GLFW has no software rendering path; use SDL3/X11/Wayland for "
                    "CPU rendering, or OpenGL/Vulkan with GLFW");
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
            throw WindowException("Failed to create GLFW window");
        }

        glfwSetWindowUserPointer(window_, userData_.get());
        glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
        glfwSetWindowFocusCallback(window_, windowFocusCallback);
        glfwSetWindowIconifyCallback(window_, windowIconifyCallback);

        if (graphicsAPI_ == GraphicsAPI::OpenGL) {
            glfwMakeContextCurrent(window_);
            glfwSwapInterval(windowDetails_.vsync ? 1 : 0);
            // No GL loader is bundled: load functions yourself via getGLProcAddress().
        }

        keyboardListener_->initialize(window_);
        mouseListener_->initialize(window_);

        INK_LOG << "GLFW window created: " << windowName;
    }

    void GlfwWindowManager::pollEvents() {
        glfwPollEvents();
    }

    void GlfwWindowManager::swapBuffers() {
        if (graphicsAPI_ == GraphicsAPI::OpenGL && window_) {
            glfwSwapBuffers(window_);
        }
    }

    void* GlfwWindowManager::getWindowInstance() { return window_; }

    void* GlfwWindowManager::getGLProcAddress(const char* name) const {
        if (graphicsAPI_ != GraphicsAPI::OpenGL) 
            return nullptr;
        return reinterpret_cast<void*>(glfwGetProcAddress(name));
    }

    WindowFlags* GlfwWindowManager::getWindowFlags() noexcept { return &windowFlags_; }
    const WindowDetails* GlfwWindowManager::getWindowDetails() noexcept { return &windowDetails_; }

    const std::vector<const char*> GlfwWindowManager::getVulkanExtensions() const {
        u32 extensionCount = 0;
        const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
        if (!extensions) 
        {
            throw GraphicsException(
                "Failed to get Vulkan extensions from GLFW (Vulkan loader unavailable?)");
        }
        return std::vector<const char*>(extensions, extensions + extensionCount);
    }

    KeyboardListener& GlfwWindowManager::getKeyboardListener() noexcept { return *keyboardListener_; }
    MouseListener& GlfwWindowManager::getMouseListener() noexcept { return *mouseListener_; }

    bool GlfwWindowManager::shouldClose() const {
        return windowShouldClose_ || (window_ && glfwWindowShouldClose(window_));
    }

    WindowBackend GlfwWindowManager::getBackendType() const { return WindowBackend::GLFW; }
    GraphicsAPI GlfwWindowManager::getGraphicsAPI() const { return graphicsAPI_; }

    WmaCode GlfwWindowManager::destroy() {
        windowShouldClose_ = true;
        if (window_) 
        {
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }
        if (ownsInit_) 
        {
            ownsInit_ = false;
            if (g_glfwRefCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
                glfwTerminate();
        }
        return WmaCode::Ok;
    }

    void GlfwWindowManager::initializeGLFW() {
        if (g_glfwRefCount.fetch_add(1, std::memory_order_acq_rel) == 0)
        {
            if (!glfwInit()) 
            {
                g_glfwRefCount.fetch_sub(1, std::memory_order_acq_rel);
                throw WMAException("Failed to initialize GLFW");
            }
        }
        ownsInit_ = true;
    }

    void GlfwWindowManager::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
        auto* instance = getInstanceFromWindow(window);
        if (instance)
        {
            instance->windowDetails_.width = width;
            instance->windowDetails_.height = height;
            instance->windowFlags_.resized = true;
        }
    }

    void GlfwWindowManager::windowFocusCallback(GLFWwindow* window, int focused) {
        auto* instance = getInstanceFromWindow(window);
        if (instance)
            instance->windowFlags_.focused = (focused == GLFW_TRUE);
    }

    void GlfwWindowManager::windowIconifyCallback(GLFWwindow* window, int iconified) {
        auto* instance = getInstanceFromWindow(window);
        if (instance)
            instance->windowFlags_.minimized = (iconified == GLFW_TRUE);
    }

    GlfwWindowManager* GlfwWindowManager::getInstanceFromWindow(GLFWwindow* window) {
        if (!window)
            return nullptr;
        auto* userData = static_cast<GlfwUserData*>(glfwGetWindowUserPointer(window));
        return userData ? userData->windowManager : nullptr;
    }

} // namespace wma
#endif // WMA_ENABLE_GLFW
