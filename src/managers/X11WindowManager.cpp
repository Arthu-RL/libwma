#include "wma/managers/X11WindowManager.hpp"

#include <ink/InkAssert.h>
#include <ink/InkException.h>

namespace wma {

X11WindowManager::X11WindowManager(const WindowDetails& windowDetails,
                                   GraphicsAPI graphicsAPI)
    : display_(nullptr)
    , window_(0)
    , colormap_(0)
    , wmDeleteWindow_(0)
    , windowDetails_(windowDetails)
    , windowFlags_{}
    , graphicsAPI_(graphicsAPI)
    , keyboardListener_(std::make_unique<KeyboardListener>())
    , mouseListener_(std::make_unique<MouseListener>())
    , windowShouldClose_(false)
{
    // Empty
}

wma::X11WindowManager::~X11WindowManager() {
    destroy();
}

void X11WindowManager::createWindow(const char* windowName)
{
    // Open a connection to the X server.
    display_ = XOpenDisplay(nullptr);
    INK_ASSERT_MSG(display_ != nullptr, "Failed to open X11 display.");

    int screen = DefaultScreen(display_);
    Window rootWindow = RootWindow(display_, screen);

    // Set up window attributes.
    XSetWindowAttributes windowAttributes;
    // receive keyboard, mouse, resize, and exposure events.
    windowAttributes.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
                                  ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
                                  StructureNotifyMask; // For resize events

    window_ = XCreateWindow(display_,
                            rootWindow,
                            100, 100, // x, y position
                            windowDetails_.width, windowDetails_.height,
                            1,
                            DefaultDepth(display_, screen),
                            InputOutput,
                            DefaultVisual(display_, screen),
                            CWEventMask,
                            &windowAttributes);

    if (!window_) {
        XCloseDisplay(display_);
        INK_THROW("Failed to create X11 window.");
    }

    // Set the window title.
    XStoreName(display_, window_, windowName);

    // Set up the protocol to listen for the window manager closing the window (e.g., clicking the 'X' button).
    wmDeleteWindow_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display_, window_, &wmDeleteWindow_, 1);

    // Map the window to the screen to make it visible.
    XMapWindow(display_, window_);
    XFlush(display_); // Ensure all commands are sent to the X server.
}

void X11WindowManager::process(std::function<void()>&& actions)
{
    if (windowShouldClose_) return;

    windowFlags_.resized = false;
    XEvent event;

    while (XPending(display_) > 0) {
        XNextEvent(display_, &event);

        switch (event.type) {
            case Expose:
                break;
            // Fired on window resize or move.
            case ConfigureNotify: {
                XConfigureEvent xce = event.xconfigure;
                if (xce.width != windowDetails_.width || xce.height != windowDetails_.height) {
                    windowDetails_.width = xce.width;
                    windowDetails_.height = xce.height;
                    windowFlags_.resized = true;
                }
                break;
            }

            // --- Input Events ---
            case KeyPress:
                // std::cout << "Key pressed: " << XLookupKeysym(&event.xkey, 0) << std::endl;
                // TODO: Update keyboardListener_
                break;

            case KeyRelease:
                // std::cout << "Key released: " << XLookupKeysym(&event.xkey, 0) << std::endl;
                // TODO: Update keyboardListener_
                break;

            case ButtonPress:
                // std::cout << "Mouse button pressed: " << event.xbutton.button << std::endl;
                // TODO: Update mouseListener_
                break;

            // Fired when the user clicks the window's close button.
            case ClientMessage:
                if (static_cast<Atom>(event.xclient.data.l[0]) == wmDeleteWindow_) {
                    windowShouldClose_ = true;
                }
                break;

            default:
                break;
        }

        actions();
    }
}

void* X11WindowManager::getWindowInstance()
{
    // This provides the native handle required by graphics APIs like Vulkan or OpenGL.
    // Note the cast to void*. The reinterpret_cast back to a Window is needed on the caller's side.
    return reinterpret_cast<void*>(window_);
}

const std::vector<const char*> X11WindowManager::getVulkanExtensions() const
{
    return {"VK_KHR_surface", "VK_KHR_xlib_surface"};
}

WindowFlags* X11WindowManager::getWindowFlags() noexcept
{
    return &windowFlags_;
}
const WindowDetails* X11WindowManager::getWindowDetails() noexcept
{
    return &windowDetails_;
}

KeyboardListener& X11WindowManager::getKeyboardListener() noexcept
{
    return *keyboardListener_;
}

MouseListener& X11WindowManager::getMouseListener() noexcept
{
    return *mouseListener_;
}

const bool X11WindowManager::shouldClose() const
{
    return windowShouldClose_;
}

WindowBackend X11WindowManager::getBackendType() const
{
    return WindowBackend::X11;
}

GraphicsAPI X11WindowManager::getGraphicsAPI() const
{
    return graphicsAPI_;
}

WmaCode X11WindowManager::destroy()
{
    if (display_) {
        if (window_) {
            XDestroyWindow(display_, window_);
            window_ = 0;
        }
        XCloseDisplay(display_);
        display_ = nullptr;
    }
    return WmaCode::OK;
}

}
