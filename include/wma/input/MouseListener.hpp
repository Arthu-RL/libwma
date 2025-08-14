#ifndef WMA_INPUT_MOUSE_LISTENER_HPP
#define WMA_INPUT_MOUSE_LISTENER_HPP

#include "wma/input/MouseAction.hpp"
#include <unordered_map>
#include <memory>
#include <mutex>

#ifdef WMA_ENABLE_GLFW
struct GLFWwindow;
#endif

#ifdef WMA_ENABLE_SDL
struct SDL_Window;
union SDL_Event;
#endif

namespace wma {

class MouseListener {
public:
    MouseListener() = default;
    ~MouseListener() = default;

    // Action management
    void addButtonAction(i32 button, MouseAction action);
    void removeButtonAction(i32 button);
    void setMoveAction(MouseAction action);
    void setScrollAction(MouseAction action);
    void clearAllActions();
    bool hasButtonAction(i32 button) const;

    // State queries
    MousePosition getCurrentPosition() const;
    void setCursorEnabled(bool enabled);
    bool isCursorEnabled() const;
    void setSensitivity(f64 sensitivity);
    f64 getSensitivity() const;

    // Process deferred events - IMPORTANT: Call this from your main loop!
    void processPendingEvents();

#ifdef WMA_ENABLE_GLFW
    void initializeGLFW(GLFWwindow* window);
    static void glfwMouseButtonCallback(GLFWwindow* window, i32 button, i32 action, i32 mods);
    static void glfwCursorPosCallback(GLFWwindow* window, f64 xpos, f64 ypos);
    static void glfwScrollCallback(GLFWwindow* window, f64 xoffset, f64 yoffset);
#endif

#ifdef WMA_ENABLE_SDL
    void initializeSDL(SDL_Window* window);
    void handleSDLEvent(const SDL_Event& event);
#endif

private:
    // Core state
    std::unordered_map<i32, MouseAction> buttonActions_;
    MouseAction moveAction_;
    MouseAction scrollAction_;
    MousePosition currentPosition_;
    MousePosition lastPosition_;
    bool cursorEnabled_ = true;
    f64 sensitivity_ = 1.0;
    bool firstMouse_ = true;

    // Deferred execution for GLFW
    struct PendingEvent {
        enum Type { None, Move, Scroll, ButtonPress, ButtonRelease };
        Type type = None;
        MousePosition position;
        MouseScroll scroll;
        i32 button = -1;
    };

    std::vector<PendingEvent> pendingEvents_;

    // Add pending event (thread-safe)
    void addPendingEvent(const PendingEvent& event);

#ifdef WMA_ENABLE_GLFW
    GLFWwindow* glfwWindow_ = nullptr;

    void handleGLFWButtonEvent(i32 button, i32 action, i32 mods);
    void handleGLFWPositionEvent(f64 xpos, f64 ypos);
    void handleGLFWScrollEvent(f64 xoffset, f64 yoffset);
    void updateCursorStateGLFW();
    i32 convertGLFWButton(i32 glfwButton) const;
    static MouseListener* getInstanceFromGLFW(GLFWwindow* window);
#endif

#ifdef WMA_ENABLE_SDL
    SDL_Window* sdlWindow_ = nullptr;

    void updateCursorStateSDL();
    i32 convertSDLButton(i32 sdlButton) const;
#endif
};

} // namespace wma

#endif // WMA_INPUT_MOUSE_LISTENER_HPP
