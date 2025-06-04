#include <iostream>
#include <memory>

#include "../../include/wma/wma.hpp"

int main() {
    try {
        // Print library information
        std::cout << wma::getLibraryInfo() << std::endl;

        // Configure window
        wma::WindowDetails windowConfig(1280, 720, true, 60);

        // Create window manager (uses default backend)
        auto windowManager = wma::createWindowManager(
            wma::getDefaultBackend(),
            windowConfig,
            wma::GraphicsAPI::Vulkan
        );

        // Create the window
        windowManager->createWindow("wma Example - Basic Window");

        // Setup input handling
        auto& keyboard = windowManager->getKeyboardListener();
        
        // Add escape key to close window
        keyboard.addKeyAction(27, wma::KeyAction{
            []() { std::cout << "Escape pressed - closing window\n"; },
            nullptr
        });

        // Add some debug keys
        keyboard.addKeyAction(32, wma::KeyAction{ // Spacebar
            []() { std::cout << "Space pressed\n"; },
            []() { std::cout << "Space released\n"; }
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
                std::cout << "Frame: " << flags->frameCounter 
                         << " | FPS: " << flags->fps 
                         << " | Delta: " << flags->deltaTime << "ms\n";
            }

            // Handle window resize
            auto* flags = windowManager->getWindowFlags();
            if (flags->resized) {
                std::cout << "Window resized!\n";
                flags->resetFrameFlags();
            }

            // Example: Exit after 10 seconds (600 frames at 60 FPS)
            if (frameCount > 600) {
                running = false;
                // For GLFW: glfwSetWindowShouldClose(window, true);
                // For SDL2: handled internally by the manager
            }
        });

        std::cout << "Window closed successfully. Total frames: " << frameCount << std::endl;

    } catch (const wma::WMAException& e) {
        std::cerr << "wma Error: " << e.what() << std::endl;
        return -1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
