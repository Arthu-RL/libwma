#include <wma/wma.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <chrono>
#include <memory>
#include <cmath>
#include <algorithm>

/**
 * Input Handling Example with Mouse Support
 *
 * This example demonstrates:
 * - Complex key combinations and mappings
 * - Comprehensive mouse input handling
 * - Input state tracking for both keyboard and mouse
 * - Key/mouse timing and repeat handling
 * - Dynamic input binding changes
 * - Cross-platform input handling
 * - Mouse gestures and patterns
 * - FPS-style camera controls
 */

class InputDemo {
public:
    struct KeyStats {
        int pressCount = 0;
        int releaseCount = 0;
        std::chrono::steady_clock::time_point lastPressed;
        std::chrono::steady_clock::time_point lastReleased;
        double totalPressTime = 0.0;
    };

    struct MouseStats {
        int pressCount = 0;
        int releaseCount = 0;
        double totalPressTime = 0.0;
        std::chrono::steady_clock::time_point lastPressed;
        std::chrono::steady_clock::time_point lastReleased;
    };

    struct MouseMovement {
        double totalDistance = 0.0;
        int moveCount = 0;
        double maxSpeed = 0.0;
        wma::MousePosition lastPosition;
        std::vector<wma::MousePosition> recentPositions;
        static constexpr size_t MAX_POSITIONS = 100;
    };

    struct MouseGesture {
        std::vector<wma::MousePosition> points;
        std::chrono::steady_clock::time_point startTime;
        bool isRecording = false;
        
        void clear() {
            points.clear();
            isRecording = false;
        }
        
        double getLength() const {
            if (points.size() < 2) return 0.0;
            double total = 0.0;
            for (size_t i = 1; i < points.size(); ++i) {
                double dx = points[i].x - points[i-1].x;
                double dy = points[i].y - points[i-1].y;
                total += std::sqrt(dx*dx + dy*dy);
            }
            return total;
        }
    };

    InputDemo() : running_(true), demoMode_(0), mouseSensitivity_(1.0),
                         cursorEnabled_(true), cameraYaw_(0.0), cameraPitch_(0.0) {}

    void run() {
        try {
            setupWindow();
            setupInputHandling();
            showInstructions();
            mainLoop();
        } catch (const wma::WMAException& e) {
            std::cerr << "wma Error: " << e.what() << std::endl;
        }
    }

private:
    void setupWindow() {
        wma::WindowDetails config(1200, 800, 1, 60);

        backend_ = wma::getDefaultBackend();
        windowManager_ = wma::createWindowManager(
            backend_,
            config,
            wma::GraphicsAPI::OpenGL
        );

        windowManager_->createWindow("wma - Advanced Input & Mouse Demo");
    }

    void setupInputHandling() {
        setupKeyboardInput();
        setupMouseInput();
        std::cout << "Advanced input handling setup complete!" << std::endl;
    }

    void setupKeyboardInput() {
        auto& keyboard = windowManager_->getKeyboardListener();

        // Get backend-specific key codes
        int escKey = (backend_ == wma::WindowBackend::GLFW) ? 256 : 27;   // Escape
        int spaceKey = 32;                                                   // Space
        int enterKey = (backend_ == wma::WindowBackend::GLFW) ? 257 : 13; // Enter
        int tabKey = (backend_ == wma::WindowBackend::GLFW) ? 258 : 9;    // Tab
        
        // WASD for camera/movement
        int wKey = (backend_ == wma::WindowBackend::GLFW) ? 87 : 119;     // W
        int aKey = (backend_ == wma::WindowBackend::GLFW) ? 65 : 97;      // A
        int sKey = (backend_ == wma::WindowBackend::GLFW) ? 83 : 115;     // S
        int dKey = (backend_ == wma::WindowBackend::GLFW) ? 68 : 100;     // D
        
        // Function keys for mouse settings
        int f1Key = (backend_ == wma::WindowBackend::GLFW) ? 290 : 282;   // F1
        int f2Key = (backend_ == wma::WindowBackend::GLFW) ? 291 : 283;   // F2
        int f3Key = (backend_ == wma::WindowBackend::GLFW) ? 292 : 284;   // F3

        // Number keys for demo mode switching (1-9)
        std::vector<int> numberKeys;
        if (backend_ == wma::WindowBackend::GLFW) {
            for (int i = 0; i < 9; ++i) {
                numberKeys.push_back(49 + i); // GLFW_KEY_1 to GLFW_KEY_9
            }
        } else {
            for (int i = 0; i < 9; ++i) {
                numberKeys.push_back(49 + i); // SDLK_1 to SDLK_9
            }
        }

        // System keys
        keyboard.addKeyAction(escKey, wma::KeyAction{
            [this]() {
                std::cout << "ESC pressed - Exiting demo..." << std::endl;
                running_ = false;
            }
        });

        keyboard.addKeyAction(spaceKey, wma::KeyAction{
            [this]() { onSpacePress(); },
            [this]() { onSpaceRelease(); }
        });

        keyboard.addKeyAction(enterKey, wma::KeyAction{
            [this]() {
                std::cout << "Enter pressed - Showing statistics..." << std::endl;
                showStatistics();
            }
        });

        keyboard.addKeyAction(tabKey, wma::KeyAction{
            [this]() {
                demoMode_ = (demoMode_ + 1) % 6; // Now 6 modes (added mouse modes)
                std::cout << "Switched to demo mode: " << demoMode_ << std::endl;
                showCurrentModeInfo();
                updateMouseSettings();
            }
        });

        // WASD movement
        keyboard.addKeyAction(wKey, wma::KeyAction{
            [this]() { std::cout << "Moving forward (W)" << std::endl; },
            [this]() { std::cout << "Stopped moving forward" << std::endl; }
        });

        keyboard.addKeyAction(sKey, wma::KeyAction{
            [this]() { std::cout << "Moving backward (S)" << std::endl; },
            [this]() { std::cout << "Stopped moving backward" << std::endl; }
        });

        keyboard.addKeyAction(aKey, wma::KeyAction{
            [this]() { std::cout << "Moving left (A)" << std::endl; },
            [this]() { std::cout << "Stopped moving left" << std::endl; }
        });

        keyboard.addKeyAction(dKey, wma::KeyAction{
            [this]() { std::cout << "Moving right (D)" << std::endl; },
            [this]() { std::cout << "Stopped moving right" << std::endl; }
        });

        // Function keys for mouse control
        keyboard.addKeyAction(f1Key, wma::KeyAction{
            [this]() { toggleCursor(); }
        });

        keyboard.addKeyAction(f2Key, wma::KeyAction{
            [this]() { adjustMouseSensitivity(0.1); }
        });

        keyboard.addKeyAction(f3Key, wma::KeyAction{
            [this]() { adjustMouseSensitivity(-0.1); }
        });

        // Number keys for mode selection
        for (int i = 0; i < 6; ++i) { // Extended to 6 modes
            int modeNum = i;
            keyboard.addKeyAction(numberKeys[i], wma::KeyAction{
                [this, modeNum]() {
                    demoMode_ = modeNum;
                    std::cout << "Switched to demo mode: " << demoMode_ << std::endl;
                    showCurrentModeInfo();
                    updateMouseSettings();
                }
            });
        }
    }

    void setupMouseInput() {
        auto& mouse = windowManager_->getMouseListener();

        // Mouse button actions
        mouse.addButtonAction(wma::MouseButton::Left, wma::MouseAction{
            [this]() { onMouseButtonPress(wma::MouseButton::Left); },
            [this]() { onMouseButtonRelease(wma::MouseButton::Left); }
        });

        mouse.addButtonAction(wma::MouseButton::Right, wma::MouseAction{
            [this]() { onMouseButtonPress(wma::MouseButton::Right); },
            [this]() { onMouseButtonRelease(wma::MouseButton::Right); }
        });

        mouse.addButtonAction(wma::MouseButton::Middle, wma::MouseAction{
            [this]() { onMouseButtonPress(wma::MouseButton::Middle); },
            [this]() { onMouseButtonRelease(wma::MouseButton::Middle); }
        });

        // Additional mouse buttons
        mouse.addButtonAction(wma::MouseButton::Button4, wma::MouseAction{
            [this]() { onMouseButtonPress(wma::MouseButton::Button4); },
            [this]() { onMouseButtonRelease(wma::MouseButton::Button4); }
        });

        mouse.addButtonAction(wma::MouseButton::Button5, wma::MouseAction{
            [this]() { onMouseButtonPress(wma::MouseButton::Button5); },
            [this]() { onMouseButtonRelease(wma::MouseButton::Button5); }
        });

        // Mouse movement
        mouse.setMoveAction(wma::MouseAction{
            nullptr, nullptr,  // no button actions
            [this](const wma::MousePosition& pos) { onMouseMove(pos); }
        });

        // Mouse scroll
        mouse.setScrollAction(wma::MouseAction{
            nullptr, nullptr, nullptr,  // no button/move actions
            [this](const wma::MouseScroll& scroll) { onMouseScroll(scroll); }
        });

        // Set initial mouse settings
        mouse.setSensitivity(mouseSensitivity_);
        mouse.setCursorEnabled(cursorEnabled_);
    }

    // Mouse event handlers
    void onMouseButtonPress(int button) {
        auto now = std::chrono::steady_clock::now();
        mouseStats_[button].pressCount++;
        mouseStats_[button].lastPressed = now;
        
        auto pos = windowManager_->getMouseListener().getCurrentPosition();
        
        switch (demoMode_) {
            case 0: // Basic mode
                std::cout << "Mouse button " << getButtonName(button)
                         << " pressed at (" << pos.x << ", " << pos.y << ")" << std::endl;
                break;
            case 1: // Timing mode
                std::cout << "Mouse " << getButtonName(button)
                         << " pressed at frame " << frameCount_ << std::endl;
                break;
            case 2: // Statistics mode
                std::cout << "Mouse " << getButtonName(button)
                         << " pressed (total: " << mouseStats_[button].pressCount << ")" << std::endl;
                break;
            case 3: // Camera mode
                if (button == wma::MouseButton::Left) {
                    std::cout << "Camera drag started" << std::endl;
                } else if (button == wma::MouseButton::Right) {
                    std::cout << "Camera zoom mode started" << std::endl;
                }
                break;
            case 4: // Gesture mode
                if (button == wma::MouseButton::Left) {
                    startGesture(pos);
                }
                break;
            case 5: // Advanced mode
                {
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now - programStart_).count();
                    std::cout << "Mouse " << getButtonName(button)
                             << " pressed at " << duration << "ms, position ("
                             << pos.x << ", " << pos.y << ")" << std::endl;
                }
                break;
        }
    }

    void onMouseButtonRelease(int button) {
        auto now = std::chrono::steady_clock::now();
        auto& stats = mouseStats_[button];
        stats.releaseCount++;
        stats.lastReleased = now;
        
        if (stats.lastPressed.time_since_epoch().count() > 0) {
            auto holdTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - stats.lastPressed).count();
            stats.totalPressTime += holdTime;
            
            if (demoMode_ >= 2) {
                std::cout << "Mouse " << getButtonName(button)
                         << " released (held for " << holdTime << "ms)" << std::endl;
            }
        }

        // Handle gesture completion
        if (demoMode_ == 4 && button == wma::MouseButton::Left) {
            finishGesture();
        }
    }

    void onMouseMove(const wma::MousePosition& pos) {
        // Update movement statistics
        mouseMovement_.moveCount++;
        
        if (mouseMovement_.lastPosition.x != 0 || mouseMovement_.lastPosition.y != 0) {
            double dx = pos.x - mouseMovement_.lastPosition.x;
            double dy = pos.y - mouseMovement_.lastPosition.y;
            double distance = std::sqrt(dx*dx + dy*dy);
            mouseMovement_.totalDistance += distance;
            
            // Calculate speed (pixels per frame, roughly)
            double speed = std::sqrt(pos.deltaX*pos.deltaX + pos.deltaY*pos.deltaY);
            mouseMovement_.maxSpeed = std::max(mouseMovement_.maxSpeed, speed);
        }
        
        mouseMovement_.lastPosition = pos;
        
        // Store recent positions for gesture recognition
        mouseMovement_.recentPositions.push_back(pos);
        if (mouseMovement_.recentPositions.size() > MouseMovement::MAX_POSITIONS) {
            mouseMovement_.recentPositions.erase(mouseMovement_.recentPositions.begin());
        }

        switch (demoMode_) {
            case 0: // Basic mode - only show significant movements
                if (std::abs(pos.deltaX) > 5 || std::abs(pos.deltaY) > 5) {
                    std::cout << "Mouse moved to (" << pos.x << ", " << pos.y
                             << ") delta(" << pos.deltaX << ", " << pos.deltaY << ")" << std::endl;
                }
                break;
            case 1: // Timing mode - periodic updates
                if (frameCount_ % 30 == 0 && (std::abs(pos.deltaX) > 1 || std::abs(pos.deltaY) > 1)) {
                    std::cout << "Mouse at (" << pos.x << ", " << pos.y << ") [Frame " << frameCount_ << "]" << std::endl;
                }
                break;
            case 3: // Camera mode
                updateCamera(pos);
                break;
            case 4: // Gesture mode
                if (currentGesture_.isRecording) {
                    currentGesture_.points.push_back(pos);
                }
                break;
            case 5: // Advanced mode - detailed movement info
                if (frameCount_ % 10 == 0) { // Every 10 frames
                    std::cout << std::fixed << std::setprecision(1)
                             << "Mouse: pos(" << pos.x << ", " << pos.y
                             << ") delta(" << pos.deltaX << ", " << pos.deltaY
                             << ") speed(" << std::sqrt(pos.deltaX*pos.deltaX + pos.deltaY*pos.deltaY)
                             << ")" << std::endl;
                }
                break;
        }
    }

    void onMouseScroll(const wma::MouseScroll& scroll) {
        switch (demoMode_) {
            case 0: // Basic mode
                std::cout << "Mouse scrolled: X=" << scroll.xOffset << ", Y=" << scroll.yOffset << std::endl;
                break;
            case 1: // Timing mode
                std::cout << "Scroll at frame " << frameCount_ << ": Y=" << scroll.yOffset << std::endl;
                break;
            case 2: // Statistics mode
                scrollCount_++;
                totalScrollY_ += scroll.yOffset;
                std::cout << "Scroll #" << scrollCount_ << ", total Y: " << totalScrollY_ << std::endl;
                break;
            case 3: // Camera mode
                {
                    double zoomFactor = 1.0 + (scroll.yOffset * 0.1);
                    cameraZoom_ *= zoomFactor;
                    cameraZoom_ = std::clamp(cameraZoom_, 0.1, 10.0);
                    std::cout << "Camera zoom: " << std::fixed << std::setprecision(2)
                             << cameraZoom_ << "x" << std::endl;
                }
                break;
            case 4: // Gesture mode
                std::cout << "Scroll gesture: " << (scroll.yOffset > 0 ? "Up" : "Down")
                         << " (magnitude: " << std::abs(scroll.yOffset) << ")" << std::endl;
                break;
            case 5: // Advanced mode
                {
                    auto pos = windowManager_->getMouseListener().getCurrentPosition();
                    std::cout << "Scroll at (" << pos.x << ", " << pos.y << "): "
                             << "X=" << scroll.xOffset << ", Y=" << scroll.yOffset;
                    if (scroll.yOffset > 0) std::cout << " [ZOOM IN]";
                    else if (scroll.yOffset < 0) std::cout << " [ZOOM OUT]";
                    std::cout << std::endl;
                }
                break;
        }
    }

    // Gesture handling
    void startGesture(const wma::MousePosition& pos) {
        currentGesture_.clear();
        currentGesture_.isRecording = true;
        currentGesture_.startTime = std::chrono::steady_clock::now();
        currentGesture_.points.push_back(pos);
        std::cout << "Gesture recording started at (" << pos.x << ", " << pos.y << ")" << std::endl;
    }

    void finishGesture() {
        if (!currentGesture_.isRecording) return;
        
        currentGesture_.isRecording = false;
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - currentGesture_.startTime).count();
        
        double length = currentGesture_.getLength();
        std::string gestureType = recognizeGesture();
        
        std::cout << "Gesture completed!" << std::endl;
        std::cout << "  Type: " << gestureType << std::endl;
        std::cout << "  Duration: " << duration << "ms" << std::endl;
        std::cout << "  Length: " << std::fixed << std::setprecision(1) << length << " pixels" << std::endl;
        std::cout << "  Points: " << currentGesture_.points.size() << std::endl;
        
        currentGesture_.clear();
    }

    std::string recognizeGesture() {
        if (currentGesture_.points.size() < 3) return "Too short";
        
        auto& points = currentGesture_.points;
        double totalAngleChange = 0.0;
        int directionChanges = 0;
        
        // Analyze the gesture path
        for (size_t i = 2; i < points.size(); ++i) {
            // Calculate vectors
            double dx1 = points[i-1].x - points[i-2].x;
            double dy1 = points[i-1].y - points[i-2].y;
            double dx2 = points[i].x - points[i-1].x;
            double dy2 = points[i].y - points[i-1].y;
            
            // Calculate angle change
            double angle1 = std::atan2(dy1, dx1);
            double angle2 = std::atan2(dy2, dx2);
            double angleDiff = angle2 - angle1;
            
            // Normalize angle difference
            while (angleDiff > M_PI) angleDiff -= 2*M_PI;
            while (angleDiff < -M_PI) angleDiff += 2*M_PI;
            
            totalAngleChange += std::abs(angleDiff);
            
            if (std::abs(angleDiff) > 0.5) { // Significant direction change
                directionChanges++;
            }
        }
        
        // Simple gesture recognition
        if (totalAngleChange > 6.0) return "Circular";
        if (directionChanges > points.size() / 4) return "Zigzag";
        if (directionChanges < 2) return "Straight line";
        return "Complex path";
    }

    // Camera handling
    void updateCamera(const wma::MousePosition& pos) {
        if (mouseStats_[wma::MouseButton::Left].pressCount > mouseStats_[wma::MouseButton::Left].releaseCount) {
            // Left button is held - rotate camera
            cameraYaw_ += pos.deltaX * 0.5;
            cameraPitch_ += pos.deltaY * 0.5;
            
            // Clamp pitch
            cameraPitch_ = std::clamp(cameraPitch_, -90.0, 90.0);
            
            if (frameCount_ % 10 == 0) { // Update every 10 frames to avoid spam
                std::cout << "Camera rotation - Yaw: " << std::fixed << std::setprecision(1)
                         << cameraYaw_ << "°, Pitch: " << cameraPitch_ << "°" << std::endl;
            }
        }
    }

    // Helper functions
    std::string getButtonName(int button) {
        switch (button) {
            case wma::MouseButton::Left: return "Left";
            case wma::MouseButton::Right: return "Right";
            case wma::MouseButton::Middle: return "Middle";
            case wma::MouseButton::Button4: return "Button4";
            case wma::MouseButton::Button5: return "Button5";
            default: return "Unknown";
        }
    }

    void toggleCursor() {
        cursorEnabled_ = !cursorEnabled_;
        windowManager_->getMouseListener().setCursorEnabled(cursorEnabled_);
        std::cout << "Cursor " << (cursorEnabled_ ? "enabled" : "disabled") << std::endl;
    }

    void adjustMouseSensitivity(double delta) {
        mouseSensitivity_ += delta;
        mouseSensitivity_ = std::clamp(mouseSensitivity_, 0.1, 5.0);
        windowManager_->getMouseListener().setSensitivity(mouseSensitivity_);
        std::cout << "Mouse sensitivity: " << std::fixed << std::setprecision(1)
                 << mouseSensitivity_ << std::endl;
    }

    void updateMouseSettings() {
        auto& mouse = windowManager_->getMouseListener();
        
        switch (demoMode_) {
            case 3: // Camera mode - disable cursor for FPS-style control
                mouse.setCursorEnabled(false);
                mouse.setSensitivity(0.2);
                break;
            case 4: // Gesture mode - enable cursor for precise tracking
                mouse.setCursorEnabled(true);
                mouse.setSensitivity(1.0);
                break;
            default: // Other modes - use current settings
                mouse.setCursorEnabled(cursorEnabled_);
                mouse.setSensitivity(mouseSensitivity_);
                break;
        }
    }

    // Original keyboard handlers
    void onSpacePress() {
        auto now = std::chrono::steady_clock::now();
        keyStats_[' '].pressCount++;
        keyStats_[' '].lastPressed = now;
        
        switch (demoMode_) {
            case 0:
                std::cout << "SPACE pressed (Basic mode)" << std::endl;
                break;
            case 1:
                std::cout << "SPACE pressed at frame " << frameCount_ << std::endl;
                break;
            case 2:
                std::cout << "SPACE pressed (total: " << keyStats_[' '].pressCount << ")" << std::endl;
                break;
            case 3:
                std::cout << "SPACE pressed - Camera jump/reset" << std::endl;
                cameraYaw_ = 0.0;
                cameraPitch_ = 0.0;
                break;
            case 4:
                std::cout << "SPACE pressed - Clear gesture history" << std::endl;
                currentGesture_.clear();
                break;
            case 5:
                {
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now - programStart_).count();
                    std::cout << "SPACE pressed at " << duration << "ms since start" << std::endl;
                }
                break;
        }
    }

    void onSpaceRelease() {
        auto now = std::chrono::steady_clock::now();
        auto& stats = keyStats_[' '];
        stats.releaseCount++;
        stats.lastReleased = now;
        
        if (stats.lastPressed.time_since_epoch().count() > 0) {
            auto holdTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - stats.lastPressed).count();
            stats.totalPressTime += holdTime;
            
            if (demoMode_ >= 2) {
                std::cout << "SPACE released (held for " << holdTime << "ms)" << std::endl;
            }
        }
    }

    void showInstructions() {
        std::cout << "\n=== wma Advanced Input & Mouse Demo ===" << std::endl;
        std::cout << "Backend: " << (backend_ == wma::WindowBackend::GLFW ? "GLFW" : "SDL2") << std::endl;
        std::cout << "\nKeyboard Controls:" << std::endl;
        std::cout << "  ESC     - Exit demo" << std::endl;
        std::cout << "  SPACE   - Test key (behavior changes by mode)" << std::endl;
        std::cout << "  ENTER   - Show input statistics" << std::endl;
        std::cout << "  TAB     - Cycle through demo modes" << std::endl;
        std::cout << "  1-6     - Select demo mode directly" << std::endl;
        std::cout << "  WASD    - Movement keys" << std::endl;
        std::cout << "  F1      - Toggle cursor on/off" << std::endl;
        std::cout << "  F2/F3   - Increase/decrease mouse sensitivity" << std::endl;
        
        std::cout << "\nMouse Controls:" << std::endl;
        std::cout << "  Left Click    - Primary action (varies by mode)" << std::endl;
        std::cout << "  Right Click   - Secondary action" << std::endl;
        std::cout << "  Middle Click  - Special action" << std::endl;
        std::cout << "  Button 4/5    - Extra buttons (if available)" << std::endl;
        std::cout << "  Mouse Move    - Movement tracking/camera control" << std::endl;
        std::cout << "  Scroll Wheel  - Zoom/navigation" << std::endl;
        
        showCurrentModeInfo();
        programStart_ = std::chrono::steady_clock::now();
    }

    void showCurrentModeInfo() {
        std::cout << "\nCurrent Mode: ";
        switch (demoMode_) {
            case 0:
                std::cout << "0 - Basic Input (simple key/mouse presses)" << std::endl;
                break;
            case 1:
                std::cout << "1 - Timing Mode (frame-based timing)" << std::endl;
                break;
            case 2:
                std::cout << "2 - Statistics Mode (press/release counting)" << std::endl;
                break;
            case 3:
                std::cout << "3 - Camera Mode (FPS-style mouse look, cursor disabled)" << std::endl;
                std::cout << "    - Hold left mouse and move for camera rotation" << std::endl;
                std::cout << "    - Scroll to zoom, SPACE to reset camera" << std::endl;
                break;
            case 4:
                std::cout << "4 - Gesture Mode (mouse gesture recognition)" << std::endl;
                std::cout << "    - Hold left mouse and drag to draw gestures" << std::endl;
                std::cout << "    - Release to complete gesture" << std::endl;
                break;
            case 5:
                std::cout << "5 - Advanced Mode (detailed input analysis)" << std::endl;
                break;
        }
    }

    void showStatistics() {
        std::cout << "\n=== COMPREHENSIVE INPUT STATISTICS ===" << std::endl;
        std::cout << "Frame Count: " << frameCount_ << std::endl;
        
        auto now = std::chrono::steady_clock::now();
        auto runtime = std::chrono::duration_cast<std::chrono::seconds>(
            now - programStart_).count();
        std::cout << "Runtime: " << runtime << " seconds" << std::endl;
        
        // Window stats
        auto* flags = windowManager_->getWindowFlags();
        std::cout << "FPS: " << static_cast<int>(flags->fps) << std::endl;
        std::cout << "Delta Time: " << flags->deltaTime << "ms" << std::endl;
        
        // Keyboard statistics
        std::cout << "\n--- KEYBOARD STATISTICS ---" << std::endl;
        int totalKeyPresses = 0;
        for (const auto& [key, stats] : keyStats_) {
            if (stats.pressCount > 0) {
                totalKeyPresses += stats.pressCount;
                char keyChar = key;
                double avgHoldTime = stats.totalPressTime / stats.pressCount;
                std::cout << "  " << keyChar << ": "
                         << stats.pressCount << " presses, "
                         << "avg hold: " << std::fixed << std::setprecision(1)
                         << avgHoldTime << "ms" << std::endl;
            }
        }
        std::cout << "Total key presses: " << totalKeyPresses << std::endl;
        
        // Mouse statistics
        std::cout << "\n--- MOUSE STATISTICS ---" << std::endl;
        int totalMousePresses = 0;
        for (const auto& [button, stats] : mouseStats_) {
            if (stats.pressCount > 0) {
                totalMousePresses += stats.pressCount;
                double avgHoldTime = stats.totalPressTime / stats.pressCount;
                std::cout << "  " << getButtonName(button) << ": "
                         << stats.pressCount << " presses, "
                         << "avg hold: " << std::fixed << std::setprecision(1)
                         << avgHoldTime << "ms" << std::endl;
            }
        }
        std::cout << "Total mouse presses: " << totalMousePresses << std::endl;
        
        // Mouse movement statistics
        std::cout << "\n--- MOUSE MOVEMENT ---" << std::endl;
        std::cout << "Total distance: " << std::fixed << std::setprecision(0)
                 << mouseMovement_.totalDistance << " pixels" << std::endl;
        std::cout << "Move events: " << mouseMovement_.moveCount << std::endl;
        std::cout << "Max speed: " << std::fixed << std::setprecision(1)
                 << mouseMovement_.maxSpeed << " pixels/frame" << std::endl;
        
        if (mouseMovement_.moveCount > 0) {
            double avgDistance = mouseMovement_.totalDistance / mouseMovement_.moveCount;
            std::cout << "Avg distance per move: " << std::fixed << std::setprecision(2)
                     << avgDistance << " pixels" << std::endl;
        }
        
        // Scroll statistics
        std::cout << "\n--- SCROLL STATISTICS ---" << std::endl;
        std::cout << "Scroll events: " << scrollCount_ << std::endl;
        std::cout << "Total Y scroll: " << std::fixed << std::setprecision(1)
                 << totalScrollY_ << std::endl;
        
        // Camera state (if in camera mode)
        if (demoMode_ == 3) {
            std::cout << "\n--- CAMERA STATE ---" << std::endl;
            std::cout << "Yaw: " << std::fixed << std::setprecision(1) << cameraYaw_ << "°" << std::endl;
            std::cout << "Pitch: " << std::fixed << std::setprecision(1) << cameraPitch_ << "°" << std::endl;
            std::cout << "Zoom: " << std::fixed << std::setprecision(2) << cameraZoom_ << "x" << std::endl;
        }
        
        // Mouse settings
        std::cout << "\n--- MOUSE SETTINGS ---" << std::endl;
        std::cout << "Sensitivity: " << std::fixed << std::setprecision(1) << mouseSensitivity_ << std::endl;
        std::cout << "Cursor enabled: " << (cursorEnabled_ ? "Yes" : "No") << std::endl;
        
        std::cout << "=================================" << std::endl;
    }

    void mainLoop() {
        std::cout << "\nStarting main loop..." << std::endl;
        
        windowManager_->process([this]() {
            frameCount_++;
            
            // Show periodic updates based on mode
            if (frameCount_ % (60 * 5) == 0) { // Every 5 seconds
                switch (demoMode_) {
                    case 1: // Timing mode
                        std::cout << "Frame " << frameCount_ << " - Still running..." << std::endl;
                        break;
                    case 2: // Statistics mode
                        {
                            int totalPresses = 0;
                            for (const auto& [key, stats] : keyStats_) {
                                totalPresses += stats.pressCount;
                            }
                            for (const auto& [button, stats] : mouseStats_) {
                                totalPresses += stats.pressCount;
                            }
                            std::cout << "Total input events so far: " << totalPresses << std::endl;
                        }
                        break;
                }
            }
            
            // Handle window events
            auto* flags = windowManager_->getWindowFlags();
            if (flags->resized) {
                auto* details = windowManager_->getWindowDetails();
                std::cout << "Window resized to: " << details->width << "x" << details->height << std::endl;
            }
            
            // Exit condition
            if (!running_) {
                return;
            }
        });
        
        std::cout << "\nDemo completed!" << std::endl;
        showStatistics();
    }

private:
    // Core components
    std::unique_ptr<wma::IWindowManager> windowManager_;
    wma::WindowBackend backend_;
    bool running_;
    int demoMode_;
    int frameCount_ = 0;
    std::chrono::steady_clock::time_point programStart_;
    
    // Input statistics
    std::unordered_map<char, KeyStats> keyStats_;
    std::unordered_map<int, MouseStats> mouseStats_;
    MouseMovement mouseMovement_;
    
    // Mouse-specific state
    double mouseSensitivity_;
    bool cursorEnabled_;
    int scrollCount_ = 0;
    double totalScrollY_ = 0.0;
    
    // Camera state
    double cameraYaw_;
    double cameraPitch_;
    double cameraZoom_ = 1.0;
    
    // Gesture recognition
    MouseGesture currentGesture_;
};

int main() {
    InputDemo demo;
    demo.run();
    return 0;
}
