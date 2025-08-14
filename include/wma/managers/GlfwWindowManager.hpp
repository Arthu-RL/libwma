#ifndef WMA_MANAGERS_GLFW_WINDOW_MANAGER_HPP
#define WMA_MANAGERS_GLFW_WINDOW_MANAGER_HPP

#include <memory>

#include "IWindowManager.hpp"

// Forward declarations
struct GLFWwindow;

namespace wma {

    // Forward declaration for user data structure
    struct GlfwUserData;

    /**
     * @brief GLFW-based window manager implementation
     * 
     * Provides window management using GLFW backend with support
     * for Vulkan, OpenGL, and CPU rendering.
     */
    class GlfwWindowManager : public IWindowManager {
    public:
        /**
         * @brief Construct GLFW window manager
         * @param windowDetails Window configuration
         * @param graphicsAPI Graphics API to use
         */
        explicit GlfwWindowManager(const WindowDetails& windowDetails, 
                                   GraphicsAPI graphicsAPI = GraphicsAPI::Vulkan);
        
        /**
         * @brief Destructor - cleans up GLFW resources
         */
        ~GlfwWindowManager() override;
        
        // Non-copyable
        GlfwWindowManager(const GlfwWindowManager&) = delete;
        GlfwWindowManager& operator=(const GlfwWindowManager&) = delete;
        
        // Movable
        GlfwWindowManager(GlfwWindowManager&&) noexcept;
        GlfwWindowManager& operator=(GlfwWindowManager&&) noexcept;
        
        // IWindowManager interface implementation
        void createWindow(const char* windowName) override;
        void process(std::function<void()>&& actions) override;
        WindowFlags* getWindowFlags() override;
        const WindowDetails* getWindowDetails() const override;
        std::vector<const char*> getVulkanExtensions() const override;
        KeyboardListener& getKeyboardListener() override;
        MouseListener& getMouseListener() override;
        bool shouldClose() const override;
        WindowBackend getBackendType() const override;
        GraphicsAPI getGraphicsAPI() const override;
        WmaCode destroy() override;
        
        /**
         * @brief Get raw GLFW window handle
         * @return GLFWwindow pointer
         */
        GLFWwindow* getGLFWWindow() const;
        
    private:
        GLFWwindow* window_;
        WindowDetails windowDetails_;
        WindowFlags windowFlags_;
        GraphicsAPI graphicsAPI_;
        std::unique_ptr<KeyboardListener> keyboardListener_;
        std::unique_ptr<MouseListener> mouseListener_;
        std::unique_ptr<GlfwUserData> userData_;
        bool windowShouldClose_;
        
        // Event handling;
        static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
        static void windowFocusCallback(GLFWwindow* window, int focused);
        static void windowIconifyCallback(GLFWwindow* window, int iconified);
        
        // Helper methods
        void initializeGLFW();
        
        // Static helper to get instance from window
        static GlfwWindowManager* getInstanceFromWindow(GLFWwindow* window);
    };

} // namespace wma

#endif // WMA_MANAGERS_GLFW_WINDOW_MANAGER_HPP
