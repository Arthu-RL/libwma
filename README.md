# WMA - Window Management Abstration Library

WMA is a modern C++23 window management library that provides a unified interface for creating and managing windows across different backends (GLFW/SDL3/X11/WAYLAND) and graphics APIs (Vulkan/OpenGL).

# WMA - Window Management Library

## Directory Structure
```
libwma/
├── include/
│   └── wma/
│       ├── core/
│       ├── input/
│       ├── managers/
│       ├── exceptions/
├── src/
│   ├── input/
│   ├── managers/
│   └── exceptions/
├── CMakeLists.txt
├── examples/
└── README.md
```

## ✨ Features

- **Multiple Backends**: GLFW/SDL3/X11/WAYLAND support
- **Graphics APIs**: CPU, Vulkan and OpenGL support
- **Modern C++23**: Clean, type-safe API design
- **Input Handling**: Unified keyboard input system

## 🚀 Quick Start

[Basic usage](https://github.com/Arthu-RL/libwma/blob/main/examples/basic_window/main.cpp)

## 🛠️ Building

### Requirements

- C++23 compatible compiler
- CMake 4.3.3+
- At least one of:
  - GLFW 3.4+
  - SDL3 3.4+

### Build

```bash
git clone https://github.com/Arthu-RL/libwma.git

export LOCAL_PREFIX=/usr/local

cmake -S ./libwma -B ./libwma/build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${LOCAL_PREFIX} \
    -DWMA_ENABLE_GLFW=OFF \
    -DWMA_ENABLE_SDL=ON \
    -DWMA_ENABLE_X11=ON \
    -DWMA_ENABLE_WAYLAND=ON \
    -DWMA_ENABLE_VULKAN=ON \
    -DWMA_ENABLE_OPENGL=ON \
    -DWMA_BUILD_EXAMPLES=ON && \
cmake --build ./libwma/build --target install
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

WMA doesn't automatically selects the best available backend, you need to manually specify:

```cpp
// Use GLFW (recommended for graphics applications)
auto manager = wma::createWindowManager(
    wma::WindowBackend::GLFW, config, api
);

// Use SDL3 (better for games, media applications)
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
wma::GraphicsAPI::CPU  // SDL3 only
```

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

### Using with CMake

```cmake
find_package(wma REQUIRED)
target_link_libraries(your_target PUBLIC wma)
```

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
