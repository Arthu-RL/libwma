#ifdef WMA_ENABLE_X11
#include "wma/backends/x11/X11WindowManager.hpp"
#include "wma/core/FrameTimer.hpp"

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
    , keyboardListener_(std::make_unique<X11KeyboardListener>())
    , mouseListener_(std::make_unique<X11MouseListener>())
    , windowShouldClose_(false)
{
}

X11WindowManager::~X11WindowManager() {
    destroy();
}

X11WindowManager::X11WindowManager(X11WindowManager&& other) noexcept
    : display_(other.display_)
    , window_(other.window_)
    , colormap_(other.colormap_)
    , wmDeleteWindow_(other.wmDeleteWindow_)
    , windowDetails_(other.windowDetails_)
    , windowFlags_(other.windowFlags_)
    , graphicsAPI_(other.graphicsAPI_)
    , keyboardListener_(std::move(other.keyboardListener_))
    , mouseListener_(std::move(other.mouseListener_))
    , windowShouldClose_(other.windowShouldClose_)
{
    other.display_ = nullptr;
    other.window_ = 0;
}

X11WindowManager& X11WindowManager::operator=(X11WindowManager&& other) noexcept {
    if (this != &other) {
        destroy();
        display_ = other.display_;
        window_ = other.window_;
        colormap_ = other.colormap_;
        wmDeleteWindow_ = other.wmDeleteWindow_;
        windowDetails_ = other.windowDetails_;
        windowFlags_ = other.windowFlags_;
        graphicsAPI_ = other.graphicsAPI_;
        keyboardListener_ = std::move(other.keyboardListener_);
        mouseListener_ = std::move(other.mouseListener_);
        windowShouldClose_ = other.windowShouldClose_;
        other.display_ = nullptr;
        other.window_ = 0;
    }
    return *this;
}

void X11WindowManager::createWindow(const char* windowName)
{
    display_ = XOpenDisplay(nullptr);
    INK_ASSERT_MSG(display_ != nullptr, "Failed to open X11 display.");

    int screen = DefaultScreen(display_);
    Window rootWindow = RootWindow(display_, screen);

    XSetWindowAttributes windowAttributes;
    windowAttributes.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
                                  ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
                                  StructureNotifyMask;

    window_ = XCreateWindow(display_, rootWindow,
                            100, 100,
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

    XStoreName(display_, window_, windowName);

    wmDeleteWindow_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display_, window_, &wmDeleteWindow_, 1);

    XMapWindow(display_, window_);
    XFlush(display_);
}

void X11WindowManager::process(std::function<void()>&& actions)
{
    FrameTimer timer(windowFlags_);
    timer.setTargetFPS(windowDetails_.targetFPS);

    while (!windowShouldClose_) {
        timer.updateDeltaTime();
        processEvents();
        actions();
        timer.limitFrameRate();
    }
}

void X11WindowManager::processEvents()
{
    while (XPending(display_) > 0) {
        XEvent event;
        XNextEvent(display_, &event);
        handleWindowEvent(&event);

        switch (event.type) {
            case Expose:
                break;
            case KeyPress:
            case KeyRelease:
                keyboardListener_->handleKeyEvent(XLookupKeysym(&event.xkey, 0), event.xkey);
                break;
            case ButtonPress:
            case ButtonRelease:
            case MotionNotify:
                mouseListener_->handleEvent(&event);
                break;
            case ClientMessage:
                if (static_cast<Atom>(event.xclient.data.l[0]) == wmDeleteWindow_) {
                    windowShouldClose_ = true;
                }
                break;
            default:
                break;
        }
    }
}

void X11WindowManager::handleWindowEvent(const XEvent* event)
{
    if (event->type == ConfigureNotify) {
        XConfigureEvent xce = event->xconfigure;
        if (xce.width != windowDetails_.width || xce.height != windowDetails_.height) {
            windowDetails_.width = xce.width;
            windowDetails_.height = xce.height;
            windowFlags_.resized = true;
        }
    }
}

void* X11WindowManager::getWindowInstance() {
    return reinterpret_cast<void*>(window_);
}

const std::vector<const char*> X11WindowManager::getVulkanExtensions() const {
    return {"VK_KHR_surface", "VK_KHR_xlib_surface"};
}

WindowFlags* X11WindowManager::getWindowFlags() noexcept { return &windowFlags_; }
const WindowDetails* X11WindowManager::getWindowDetails() noexcept { return &windowDetails_; }
KeyboardListener& X11WindowManager::getKeyboardListener() noexcept { return *keyboardListener_; }
MouseListener& X11WindowManager::getMouseListener() noexcept { return *mouseListener_; }
bool X11WindowManager::shouldClose() const { return windowShouldClose_; }
WindowBackend X11WindowManager::getBackendType() const { return WindowBackend::X11; }
GraphicsAPI X11WindowManager::getGraphicsAPI() const { return graphicsAPI_; }

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

} // namespace wma
#endif
