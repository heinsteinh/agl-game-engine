# AGL Game Engine - Modern C++17 Game Framework

A modern C++17 cross-platform game engine built with OpenGL, GLFW, and ImGui. This project provides a clean, object-oriented wrapper around OpenGL graphics programming with comprehensive abstractions for modern game development.

## Features

- **Modern C++17**: Uses modern C++ features including RAII, smart pointers, and move semantics
- **Cross-platform support** (Windows, macOS, Linux)
- **OpenGL 3.3**: Modern programmable pipeline with comprehensive abstractions
- **Encapsulated systems**: Window, Input, Renderer, Texture, and Shader management
- **ImGui integration** for real-time debugging and UI
- **CMake build system** with FetchContent dependency management
- **Multiple demo projects** in sandbox environment
- **Pre-commit hooks** for code quality assurance

## Project Structure

```
include/
  ├── agt.h         # Main header with usage examples
  ├── game.h        # Main game class
  ├── window.h      # Window management
  └── input.h       # Input handling
src/
  ├── main.cpp      # Example implementation
  ├── game.cpp      # Game class implementation
  ├── window.cpp    # Window implementation
  └── input.cpp     # Input implementation
```

## Dependencies

All dependencies are automatically fetched using CMake FetchContent:

- **GLFW 3.3.8** - Window and input handling
- **GLM 0.9.9.8** - OpenGL Mathematics library
- **STB (latest)** - Image loading and other utilities
- **Dear ImGui 1.89.9** - Immediate mode GUI

## Building

### Prerequisites
- CMake 3.14+
- C++17 compatible compiler
- OpenGL support

### Build Steps

```bash
# Make build script executable (on Unix systems)
chmod +x build.sh

# Run the build script
./build.sh

# Or manually:
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Running
```bash
./build/bin/TutoFromBook
```

## Usage

### Basic Game Setup

```cpp
#include "agt.h"

int main() {
    agt::Game game;

    if (game.Initialize(1024, 768, "My Game")) {
        game.Run();
    }

    return 0;
}
```

### Custom Game Class

```cpp
#include "agt.h"

class MyGame : public agt::Game {
public:
    void OnUpdate(float deltaTime) override {
        // Update game logic
        if (GetInput()->IsKeyPressed(agt::Keys::Space)) {
            std::cout << "Space pressed!" << std::endl;
        }
    }

    void OnRender() override {
        // Custom rendering
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OnImGuiRender() override {
        // ImGui interface
        ImGui::Begin("Debug Window");
        ImGui::Text("Game is running!");
        ImGui::End();
    }
};

int main() {
    MyGame game;
    if (game.Initialize()) {
        game.Run();
    }
    return 0;
}
```

### Window Management

```cpp
agt::Window* window = game.GetWindow();

// Window operations
window->SetTitle("New Title");
window->SetVSync(true);
window->SetSize(1920, 1080);

// Get window properties
int width = window->GetWidth();
int height = window->GetHeight();
bool shouldClose = window->ShouldClose();

// Set callbacks
window->SetResizeCallback([](int w, int h) {
    std::cout << "Window resized: " << w << "x" << h << std::endl;
});
```

### Input Handling

```cpp
agt::Input* input = game.GetInput();

// Keyboard input
if (input->IsKeyPressed(agt::Keys::Space)) {
    // Space was just pressed (one frame only)
}

if (input->IsKeyHeld(agt::Keys::W)) {
    // W is being held down
}

if (input->IsKeyReleased(agt::Keys::Escape)) {
    // Escape was just released
}

// Mouse input
if (input->IsMouseButtonPressed(agt::MouseButton::Left)) {
    double x, y;
    input->GetMousePosition(x, y);
    std::cout << "Clicked at: " << x << ", " << y << std::endl;
}

// Mouse movement
double deltaX, deltaY;
input->GetMouseDelta(deltaX, deltaY);
// Use deltaX and deltaY for camera movement, etc.

// Scroll wheel
double scrollX, scrollY;
input->GetScrollOffset(scrollX, scrollY);

// Set cursor mode
input->SetCursorMode(GLFW_CURSOR_DISABLED); // For FPS-style camera
```

### Input Callbacks

```cpp
// Set custom input callbacks
input->SetKeyCallback([](int key, agt::KeyState state) {
    if (key == GLFW_KEY_F1 && state == agt::KeyState::Pressed) {
        std::cout << "F1 pressed!" << std::endl;
    }
});

input->SetMouseButtonCallback([](agt::MouseButton button, agt::KeyState state) {
    if (button == agt::MouseButton::Right && state == agt::KeyState::Pressed) {
        std::cout << "Right mouse button pressed!" << std::endl;
    }
});
```

## Key Features

### Namespace Organization
All classes are in the `agt` namespace to avoid conflicts:
- `agt::Game` - Main game class
- `agt::Window` - Window management
- `agt::Input` - Input handling
- `agt::KeyState` - Key/button states
- `agt::MouseButton` - Mouse button enumeration

### Event-Driven Architecture
- Callback-based input system
- Window event handling
- Extensible game loop

### Cross-Platform Support
- Works on Windows, macOS, and Linux
- Proper OpenGL context management
- Platform-specific optimizations

## Examples

The `main.cpp` file contains a complete example showing:
- Basic game setup
- Input handling
- ImGui integration
- Custom rendering
- Event callbacks

## Contributing

This is a tutorial project demonstrating game engine architecture patterns. Feel free to extend and modify for your own projects!

## License

This project is for educational purposes. Dependencies have their own licenses:
- GLFW: zlib/libpng license
- GLM: MIT license
- STB: Public domain
- Dear ImGui: MIT license
