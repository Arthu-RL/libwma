#include <ink/Inkogger.h>
#include <memory>

#include "wma/wma.hpp"

int main() {
    try {
        wma::WindowDetails windowConfig(1280, 720, true, 0);

        INK_LOG << "Starting Window...";

        auto windowManager = wma::createWindowManager(
            wma::getDefaultBackend(),
            windowConfig,
            wma::GraphicsAPI::CPU
        );

        windowManager->createWindow("wma Example - Basic Window");

        auto& keyboard = windowManager->getKeyboardListener();

        const auto gameplay = keyboard.createContext();
        const auto menu = keyboard.createContext();

        keyboard.addKeyAction(wma::Key::KEY_ESCAPE, wma::KeyAction {
            [&]() { windowManager->destroy(); INK_LOG << "Escape pressed - closing window\n"; },
            nullptr
        });

        keyboard.addKeyAction(wma::Key::KEY_D, wma::KeyAction {
            []() { INK_LOG << "D pressed - gameplay move right\n"; },
            nullptr
        }, gameplay);

        keyboard.addKeyAction(wma::Key::KEY_D, wma::KeyAction {
            []() { INK_LOG << "D pressed - menu select\n"; },
            nullptr
        }, menu);

        keyboard.addKeyAction(wma::Key::KEY_TAB, wma::KeyAction {
            [&]() {
                if (keyboard.getActiveContext() == gameplay) {
                    keyboard.setActiveContext(menu);
                    INK_LOG << "Switched to menu context\n";
                } else {
                    keyboard.setActiveContext(gameplay);
                    INK_LOG << "Switched to gameplay context\n";
                }
            },
            nullptr
        });

        keyboard.setActiveContext(gameplay);

        keyboard.addKeyAction(wma::Key::KEY_SPACE, wma::KeyAction {
            []() { INK_LOG << "Space pressed\n"; },
            []() { INK_LOG << "Space released\n"; }
        });

        auto& mouse = windowManager->getMouseListener();

        mouse.addButtonAction(wma::MouseButton::WMALeft, wma::MouseAction({
            []() { INK_LOG << "mouse left pressed\n"; },
            []() { INK_LOG << "mouse left released\n"; }
        }));

        mouse.setMoveAction(wma::MouseAction([&](const wma::WMAMousePosition& pos){
            INK_LOG << "mouse move " << pos.x << " " << pos.y;
        }));

        mouse.setScrollAction(wma::MouseAction([&](const wma::WMAMouseScroll& offset){
            INK_LOG << "scroll offset " << offset.xOffset << " " << offset.yOffset;
        }));

        unsigned long long frameCount = 0;

        windowManager->process([&]() {
            frameCount++;
            auto* flags = windowManager->getWindowFlags();
            if (flags->resized) {
                INK_LOG << "Window resized!\n";
                flags->resized = false;
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
