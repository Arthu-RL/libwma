# WMA - Cross-Platform Window Management Abstration Library

WMA is a modern C++17 cross-platform window management library that provides a unified interface for creating and managing windows across different backends (GLFW/SDL2) and graphics APIs (Vulkan/OpenGL).

# WMA - Cross-Platform Window Management Library

## Directory Structure
```
libwma/
├── include/
│   └── wma/
│       ├── core/
│       │   ├── Types.hpp
│       │   ├── WindowDetails.hpp
│       │   └── WindowFlags.hpp
│       ├── input/
│       │   ├── KeyAction.hpp
│       │   └── KeyboardListener.hpp
│       ├── managers/
│       │   ├── IWindowManager.hpp
│       │   ├── GlfwWindowManager.hpp
│       │   └── SdlWindowManager.hpp
│       ├── exceptions/
│       │   └── WMAException.hpp
│       └── WMA.hpp
├── src/
│   ├── input/
│   │   └── KeyboardListener.cpp
│   ├── managers/
│   │   ├── GlfwWindowManager.cpp
│   │   └── SdlWindowManager.cpp
│   └── exceptions/
│       └── WMAException.cpp
├── CMakeLists.txt
├── examples/
│   ├── basic_window/
│   └── input_handling/
└── README.md
```

## ✨ Features

- **Cross-Platform**: Windows, macOS, Linux support
- **Multiple Backends**: GLFW and SDL2 support
- **Graphics APIs**: Vulkan and OpenGL support
- **Modern C++17**: Clean, type-safe API design
- **Input Handling**: Unified keyboard input system
- **RAII Resource Management**: Automatic cleanup
- **Exception Safety**: Comprehensive error handling
- **Header-Only Option**: Easy integration

## 🚀 Quick Start

### Basic Usage

```cpp
#include <wma/WMA.hpp>

int main() {
    // Configure your window
    wma::WindowDetails config{
        .width = 1280,
        .height = 720,
        .resizable = true,
        .targetFPS = 60
    };

    // Create window manager
    auto windowManager = wma::createWindowManager(
        wma::getDefaultBackend(),
        config,
        wma::GraphicsAPI::Vulkan
    );

    // Create window
    windowManager->createWindow("My Application");

    // Setup input
    auto& keyboard = windowManager->getKeyboardListener();
    keyboard.addKeyAction(27, wma::KeyAction{
        []() { std::cout << "Escape pressed!\n"; }
    });

    // Main loop
    windowManager->process([]() {
        // Your rendering code here
    });

    return 0;
}
```

## 🛠️ Building

### Requirements

- C++17 compatible compiler
- CMake 3.16+
- At least one of:
  - GLFW 3.3+
  - SDL2 2.0.12+

### Build Options

```bash
mkdir build && cd build
cmake .. \
    -DWMA_ENABLE_GLFW=ON \
    -DWMA_ENABLE_SDL=ON \
    -DWMA_ENABLE_VULKAN=ON \
    -DWMA_ENABLE_OPENGL=ON \
    -DWMA_BUILD_EXAMPLES=ON

make -j$(nproc)
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `WMA_BUILD_SHARED` | ON | Build shared library |
| `WMA_ENABLE_GLFW` | ON | Enable GLFW backend |
| `WMA_ENABLE_SDL` | ON | Enable SDL2 backend |
| `WMA_ENABLE_VULKAN` | ON | Enable Vulkan support |
| `WMA_ENABLE_OPENGL` | ON | Enable OpenGL support |
| `WMA_BUILD_EXAMPLES` | ON | Build example applications |
| `WMA_BUILD_TESTS` | OFF | Build unit tests |

## 📚 Documentation

### Core Classes

#### WindowDetails
Configuration structure for window creation:
```cpp
wma::WindowDetails config{
    .width = 1920,
    .height = 1080,
    .resizable = true,
    .targetFPS = 144,
    .vsync = false,
    .fullscreen = false
};
```

#### KeyAction
Define keyboard input responses:
```cpp
wma::KeyAction action{
    []() { /* on press */ },
    []() { /* on release */ }
};
```

#### IWindowManager
Base interface for all window managers:
```cpp
auto manager = wma::createWindowManager(
    wma::WindowBackend::GLFW,
    windowDetails,
    wma::GraphicsAPI::Vulkan
);
```

### Backend Selection

WMA automatically selects the best available backend, but you can specify:

```cpp
// Use GLFW (recommended for graphics applications)
auto manager = wma::createWindowManager(
    wma::WindowBackend::GLFW, config, api
);

// Use SDL2 (better for games, media applications)
auto manager = wma::createWindowManager(
    wma::WindowBackend::SDL2, config, api
);
```

### Graphics API Support

```cpp
// Vulkan (modern, explicit API)
wma::GraphicsAPI::Vulkan

// OpenGL (traditional, easier to use)
wma::GraphicsAPI::OpenGL

// CPU rendering (software)
wma::GraphicsAPI::CPU  // SDL2 only
```

## 🎯 Examples

Check the `examples/` directory for complete examples:

- **basic_window**: Simple window creation and input handling
- **vulkan_rendering**: Vulkan integration example
- **opengl_rendering**: OpenGL integration example
- **input_handling**: Advanced keyboard and mouse input

## 🏗️ Architecture

```
wma/
├── core/           # Core types and structures
├── input/          # Input handling system
├── managers/       # Window manager implementations
└── exceptions/     # Exception handling
```

### Design Principles

1. **RAII**: Automatic resource management
2. **Type Safety**: Strong typing throughout
3. **Modularity**: Optional backends and APIs
4. **Performance**: Zero-cost abstractions where possible
5. **Extensibility**: Easy to add new backends

## 🤝 Integration

### Using with CMake

```cmake
find_package(wma REQUIRED)
target_link_libraries(your_target wma::wma)
```

### Manual Integration

Copy the `include/wma/` directory to your project and link against the required dependencies.

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **GLFW** - Multi-platform library for OpenGL, OpenGL ES and Vulkan
- **SDL2** - Cross-platform development library
- **Vulkan** - Low-level graphics API
- **OpenGL** - Cross-language, cross-platform API

## 🐛 Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📞 Support

- 📧 Create an issue on GitHub
- 💬 Discussions tab for questions
- 📖 Check the examples and documentation

---

Made with ❤️ by the WMA team
