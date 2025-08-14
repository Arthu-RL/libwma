#include <ink/Inkogger.h>
#include <memory>

#include "../../include/wma/wma.hpp"

int main() {
    try {
        // Configure window
        wma::WindowDetails windowConfig(1280, 720, true, 60);

        // Create window manager (uses default backend)
        auto windowManager = wma::createWindowManager(
            wma::getDefaultBackend(),
            windowConfig,
            wma::GraphicsAPI::CPU
        );

        // Create the window
        windowManager->createWindow("wma Example - Basic Window");

        // Setup input handling
        auto& keyboard = windowManager->getKeyboardListener();
        
        // Add escape key to close window
        keyboard.addKeyAction(wma::Key::KEY_ESCAPE, wma::KeyAction {
            [&]() { windowManager->destroy(); INK_LOG << "Escape pressed - closing window\n"; },
            nullptr
        });

        // Add some debug keys
        keyboard.addKeyAction(wma::Key::KEY_SPACE, wma::KeyAction {
            []() { INK_LOG << "Space pressed\n"; },
            []() { INK_LOG << "Space released\n"; }
        });

        // Variable to track state
        bool running = true;
        int frameCount = 0;

        // Main loop
        windowManager->process([&]() {
            // Your rendering/update code goes here
            frameCount++;

            // Print FPS every 60 frames
            if (frameCount % 60 == 0) {
                auto* flags = windowManager->getWindowFlags();
                INK_LOG << "Frame: " << flags->frameCounter
                         << " | FPS: " << flags->fps 
                         << " | Delta: " << flags->deltaTime << "ms\n";
            }

            // Handle window resize
            auto* flags = windowManager->getWindowFlags();
            if (flags->resized) {
                INK_LOG << "Window resized!\n";
                flags->resetFrameFlags();
            }

            // Example: Exit after 10 seconds (600 frames at 60 FPS)
            if (frameCount > 600) {
                running = false;
                // For GLFW: glfwSetWindowShouldClose(window, true);
                // For SDL2: handled internally by the manager
            }
        });

        INK_LOG << "Window closed successfully. Total frames: " << frameCount << '\n';

    } catch (const wma::WMAException& e) {
        INK_LOG << "wma Error: " << e.what() << '\n';
        return -1;
    } catch (const std::exception& e) {
        INK_LOG << "Error: " << e.what() << '\n';
        return -1;
    }

    return 0;
}
