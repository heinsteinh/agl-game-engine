# AGL Game Engine - Modern C++17 Game Framework

[![CI/CD](https://github.com/heinsteinh/agl-game-engine/actions/workflows/workflow.yml/badge.svg)](https://github.com/heinsteinh/agl-game-engine/actions/workflows/workflow.yml)
[![Documentation](https://img.shields.io/badge/docs-github%20pages-blue)](https://heinsteinh.github.io/agl-game-engine/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

A modern C++17 cross-platform game engine built with OpenGL, GLFW, ImGui, and spdlog. This project provides a clean, object-oriented wrapper around OpenGL graphics programming with comprehensive abstractions for modern game development.

## ✨ Features

- **🎯 Modern C++17**: Uses modern C++ features including RAII, smart pointers, and move semantics
- **🔄 Cross-platform support**: Linux, macOS (Windows support planned)
- **🎨 OpenGL 3.3+**: Modern programmable pipeline with comprehensive abstractions
- **📦 Modular Architecture**: GameLib library + Sandbox demo applications
- **🎮 Complete Systems**: Window, Input, Renderer, Texture, Shader, and Buffer management
- **� Advanced Camera System**: First-person, third-person, spectator modes with shooter game features
- **🎯 Shooter Game Ready**: Sprint, crouch, aim, camera shake, dynamic FOV, and mouse look
- **�🖥️ ImGui Integration**: Real-time debugging and UI development
- **📝 Advanced Logging**: Comprehensive spdlog integration with core/client separation
- **⚡ Performance Monitoring**: Delta time management, FPS tracking, and performance analytics
- **🏗️ CMake Build System**: FetchContent dependency management and export targets
- **🔄 CI/CD Pipeline**: Comprehensive GitHub Actions workflow with multiple demo builds
- **📚 Documentation**: Auto-generated Doxygen documentation deployed to GitHub Pages

## 📁 Project Structure

```
gamelib/                    # Core game engine library
├── include/               # Public headers
│   ├── agl.h             # Main unified header
│   ├── game.h            # Game loop and timing
│   ├── window.h          # Window management
│   ├── input.h           # Input handling
│   ├── Camera.h          # 3D camera system
│   ├── CameraController.h # Advanced camera control
│   ├── Renderer.h        # OpenGL renderer
│   ├── Shader.h          # Shader programs
│   ├── Texture.h         # Texture management
│   ├── Logger.h          # Logging system
│   └── ...
└── src/                   # Implementation files
    ├── game.cpp
    ├── window.cpp
    ├── input.cpp
    ├── Camera.cpp
    ├── CameraController.cpp
    ├── Renderer.cpp
    └── ...

sandbox/                   # Demo applications
├── src/                   # Demo source files
│   ├── renderer_demo.cpp         # Basic OpenGL rendering
│   ├── shooter_camera_demo.cpp   # Advanced camera system
│   ├── advanced_renderer_demo.cpp # Camera comparison demo
│   ├── texture_demo.cpp
│   ├── example_logger_demo.cpp
│   └── benchmark_deltatime_demo.cpp
└── CMakeLists.txt        # Sandbox build configuration

.github/workflows/         # CI/CD pipeline
├── workflow.yml          # Main workflow
build/                    # Build outputs
docs/                     # Documentation
CMakeLists.txt           # Root build configuration
```

## 📦 Dependencies

All dependencies are automatically fetched using CMake FetchContent:

- **🪟 GLFW 3.3.8** - Cross-platform window and input handling
- **🧮 GLM 0.9.9.8** - OpenGL Mathematics library for vectors and matrices
- **🖼️ STB (latest)** - Image loading and texture utilities
- **🎨 Dear ImGui 1.89.9** - Immediate mode GUI for debugging and tools
- **📝 spdlog 1.12.0** - Fast C++ logging library with multiple sinks

## 🚀 Quick Start

### Prerequisites
- **CMake 3.14+**
- **C++17 compatible compiler** (GCC 8+, Clang 7+, MSVC 2019+)
- **OpenGL 3.3+ support**
- **Git** (for dependency fetching)

### Build Options

#### Option 1: Integrated Build (Recommended)
Build everything together with shared dependencies:

```bash
# Clone the repository
git clone https://github.com/heinsteinh/agl-game-engine.git
cd agl-game-engine

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Run demos
./build/bin/sandbox  # Runs the main renderer demo
```

### Running Individual Demos

The sandbox executable contains multiple demos, each with different main functions:
- `main()` - Enhanced renderer demo with camera controls
- `main_shooter_camera()` - Advanced camera system for shooters
- `main_advanced_renderer()` - Comparison between old and new camera systems
- `main_Texture()` - Texture loading and management demo
- `main_logger()` - Logging system demonstration
- `main_Benchmark()` - Performance and delta time benchmarking
```

#### Option 2: Standalone Demo Build
Build individual demos with automatic gamelib dependency:

```bash
cd sandbox

# Build all demos in sandbox
cmake -B demo_build -DCMAKE_BUILD_TYPE=Release
cmake --build demo_build --parallel
./demo_build/bin/sandbox  # Main executable with multiple demos
```

#### Option 3: GameLib Only
Build just the library for integration into other projects:

```bash
cd gamelib
cmake -B lib_build -DCMAKE_BUILD_TYPE=Release
cmake --build lib_build --parallel
cmake --install lib_build --prefix install
```

## 💻 Usage Examples

### Basic Game Setup

```cpp
#include "agl.h"

int main() {
    agl::Game game;

    if (game.Initialize(1024, 768, "My AGL Game")) {
        AGL_INFO("Game initialized successfully!");
        game.Run();
        AGL_INFO("Game shutting down");
    } else {
        AGL_ERROR("Failed to initialize game!");
        return -1;
    }

    return 0;
}
```

### Custom Game Class with All Systems

```cpp
#include "agl.h"

class MyGame : public agl::Game {
public:
    void OnUpdate(float deltaTime) override {
        // Update game logic with frame-rate independent timing
        static float timer = 0.0f;
        timer += deltaTime;

        // Log performance every 5 seconds
        if (timer >= 5.0f) {
            AGL_INFO("Game running for {:.2f}s | FPS: {:.1f} | Delta: {:.3f}ms",
                    timer, GetFPS(), deltaTime * 1000.0f);
            timer = 0.0f;
        }

        // Handle input
        if (GetInput()->IsKeyPressed(GLFW_KEY_SPACE)) {
            AGL_INFO("Space key pressed!");
        }

        if (GetInput()->IsKeyHeld(GLFW_KEY_W)) {
            // Move forward logic
        }
    }

    void OnRender() override {
        // Custom OpenGL rendering
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Your rendering code here
    }

    void OnImGuiRender() override {
        // Real-time debugging interface
        ImGui::Begin("Game Debug");

        ImGui::Text("Performance:");
        ImGui::Text("FPS: %.1f", GetFPS());
        ImGui::Text("Delta Time: %.3f ms", GetDeltaTime() * 1000.0f);
        ImGui::Text("Average Delta: %.3f ms", GetAverageDeltaTime() * 1000.0f);

        ImGui::Separator();

        if (ImGui::Button("Log Info")) {
            AGL_INFO("User clicked debug button!");
        }

        if (ImGui::Button("Log Warning")) {
            AGL_WARN("This is a warning message");
        }

        ImGui::End();
    }
};

int main() {
    MyGame game;

    if (game.Initialize(1280, 720, "Custom AGL Game")) {
        game.Run();
    }

    return 0;
}
```

### Advanced Camera System

AGL includes a comprehensive camera system designed specifically for shooter games and 3D applications:

```cpp
#include "agl.h"

class ShooterGame : public agl::Game {
public:
    bool Initialize(int width, int height, const char* title) override {
        if (!agl::Game::Initialize(width, height, title)) {
            return false;
        }

        // Create camera and controller
        m_camera = std::make_shared<agl::Camera>(
            glm::vec3(0.0f, 2.0f, 5.0f),  // Position
            glm::vec3(0.0f, 1.0f, 0.0f),  // Up vector
            -90.0f,                        // Yaw
            0.0f                          // Pitch
        );

        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        m_camera->SetPerspective(75.0f, aspectRatio);

        m_cameraController = std::make_unique<agl::CameraController>(m_camera);
        m_cameraController->Initialize(GetInput());

        // Configure for shooter gameplay
        agl::CameraSettings settings;
        settings.movementSpeed = 8.0f;          // Fast movement
        settings.sprintMultiplier = 2.0f;       // Sprint speed boost
        settings.mouseSensitivity = 0.15f;      // Good for aiming
        settings.defaultFOV = 75.0f;            // Standard FOV
        settings.aimFOV = 50.0f;                // Zoom when aiming
        settings.sprintFOV = 85.0f;             // Wider when sprinting

        m_cameraController->SetSettings(settings);
        return true;
    }

    void OnUpdate(float deltaTime) override {
        // Update camera (handles all input automatically)
        m_cameraController->Update(deltaTime);

        // Camera mode switching
        if (GetInput()->IsKeyPressed(GLFW_KEY_1)) {
            m_cameraController->SetMode(agl::CameraMode::FirstPerson);
        }
        if (GetInput()->IsKeyPressed(GLFW_KEY_2)) {
            m_cameraController->SetMode(agl::CameraMode::ThirdPerson);
            m_cameraController->SetTarget(glm::vec3(0.0f, 1.0f, 0.0f));
        }

        // Test camera shake (e.g., weapon fire)
        if (GetInput()->IsKeyPressed(GLFW_KEY_X)) {
            m_cameraController->AddShake(0.5f, 1.0f);
        }
    }

    void OnRender() override {
        // Use camera for rendering
        glm::mat4 viewProj = m_camera->GetViewProjectionMatrix();

        // Render your scene with the camera
        // ...
    }

private:
    std::shared_ptr<agl::Camera> m_camera;
    std::unique_ptr<agl::CameraController> m_cameraController;
};
```

#### Camera Features:
- **🎮 Shooter Controls**: WASD movement, mouse look, sprint (Shift), crouch (Ctrl), aim (Right click)
- **📷 Multiple Modes**: First-person, third-person, spectator, and fixed camera modes
- **🎯 Advanced Features**: Dynamic FOV, camera shake, smooth transitions, input validation
- **⚙️ Configurable**: Customizable movement speed, sensitivity, FOV settings, and key bindings
- **🎪 Smooth Interpolation**: Optional movement and rotation smoothing for cinematic feel

```cpp
#include "agl.h"

int main() {
    // Core engine logging (used internally by AGL)
    AGL_CORE_INFO("Engine starting up...");
    AGL_CORE_WARN("This is a core warning");
    AGL_CORE_ERROR("Core error message");

    // Application logging (for your game code)
    AGL_INFO("Application message");
    AGL_WARN("Application warning");
    AGL_ERROR("Application error: {}", "Something went wrong");

    // Logs are automatically written to both console and file (agl-engine.log)

    return 0;
}
```

### Window Management

```cpp
agl::Window* window = game.GetWindow();

// Window operations
window->SetTitle("New Title");
window->SetVSync(true);

// Get window properties
int width = window->GetWidth();
int height = window->GetHeight();
bool shouldClose = window->ShouldClose();
```

### Input Handling

```cpp
agl::Input* input = game.GetInput();

// Keyboard input
if (input->IsKeyPressed(GLFW_KEY_SPACE)) {
    // Space was just pressed (one frame only)
}

if (input->IsKeyHeld(GLFW_KEY_W)) {
    // W is being held down
}

// Mouse input
if (input->IsMouseButtonPressed(agl::MouseButton::Left)) {
    double x, y;
    input->GetMousePosition(x, y);
    AGL_INFO("Clicked at: ({:.1f}, {:.1f})", x, y);
}

// Mouse movement for camera controls
double deltaX, deltaY;
input->GetMouseDelta(deltaX, deltaY);
// Use for FPS camera or mouse look
```

## 🎮 Demo Applications

The sandbox contains several demo applications showcasing different engine features:

### � Shooter Camera Demo (`shooter_camera_demo`)
- **Demonstrates**: Advanced camera system designed for shooter games
- **Features**: First-person controls, sprinting, crouching, aiming, camera shake effects
- **Camera Modes**: First-person, third-person, spectator, and fixed modes
- **Key Bindings**:
  - `WASD` - Movement (forward/left/backward/right)
  - `SPACE` - Jump/move up
  - `LEFT CTRL` - Crouch/move down
  - `LEFT SHIFT` - Sprint (hold)
  - `RIGHT MOUSE` - Aim (zoom FOV)
  - `1-4` - Switch camera modes
  - `X` - Test camera shake
  - `TAB` - Toggle wireframe
  - `H` - Toggle help overlay

### 🔄 Advanced Renderer Demo (`advanced_renderer_demo`)
- **Demonstrates**: Comparison between old manual camera and new camera system
- **Features**: Side-by-side camera system comparison, enhanced rendering
- **Controls**: Switch between camera systems to see the difference
- **Key Bindings**:
  - `C` - Toggle between old/new camera systems
  - All standard camera controls for both systems

### �🎨 Renderer Demo (`renderer_demo`)
- **Demonstrates**: Basic OpenGL rendering, 3D transformations, enhanced camera controls
- **Features**: Triangle, quad, and cube rendering with improved camera system
- **Controls**: Mouse look, WASD movement, wireframe toggle, animation controls
- **Key Bindings**:
  - `TAB` - Toggle wireframe mode
  - `SPACE` - Pause/resume animation
  - `W/A/S/D` - Camera movement
  - `Q/E` - Look up/down
  - `R` - Reset camera

### 🖼️ Texture Demo (`texture_demo`)
- **Demonstrates**: Texture loading, sampling, and multiple texture management
- **Features**: Various texture types (checkerboard, noise, gradients, random colors)
- **Controls**: Keyboard shortcuts to switch between textures
- **Key Bindings**:
  - `1-6` - Switch between different textures
  - `R` - Regenerate random texture

### 📝 Logger Demo (`example_logger_demo`)
- **Demonstrates**: Comprehensive logging system with spdlog integration
- **Features**: Interactive logging buttons, performance monitoring, file output
- **Output**: Both console and file logging (`agl-engine.log`)
- **Monitoring**: Real-time FPS, delta time, and performance metrics

### ⚡ Delta Time Benchmark (`benchmark_deltatime_demo`)
- **Demonstrates**: Frame timing, performance monitoring, and frame rate analysis
- **Features**: Frame stutter simulation, delta time clamping, performance graphs
- **Tools**: Real-time frame time graphing, performance categorization
- **Testing**: Simulated lag spikes to test delta time stability

## 🔧 Engine Architecture

### Core Namespace: `agl`
All engine classes are in the `agl` namespace:
- `agl::Game` - Main game loop with delta time management
- `agl::Window` - Cross-platform window management
- `agl::Input` - Unified input handling system
- `agl::Camera` - 3D camera with perspective/orthographic projection
- `agl::CameraController` - Advanced camera control with shooter game features
- `agl::Renderer` - OpenGL abstraction layer
- `agl::Shader` - Shader program management
- `agl::Texture2D` - Texture loading and management
- `agl::VertexArray`, `agl::VertexBuffer`, `agl::IndexBuffer` - OpenGL buffer abstractions

### Logging Macros
- **Core Engine**: `AGL_CORE_INFO`, `AGL_CORE_WARN`, `AGL_CORE_ERROR`, `AGL_CORE_TRACE`
- **Application**: `AGL_INFO`, `AGL_WARN`, `AGL_ERROR`, `AGL_TRACE`

### Camera System Enums
- **Camera Modes**: `CameraMode::FirstPerson`, `CameraMode::ThirdPerson`, `CameraMode::Spectator`, `CameraMode::Fixed`
- **Camera Movement**: `CameraMovement::Forward`, `CameraMovement::Backward`, `CameraMovement::Left`, `CameraMovement::Right`, `CameraMovement::Up`, `CameraMovement::Down`
- **Camera Types**: `CameraType::Perspective`, `CameraType::Orthographic`

### Event-Driven Architecture
- Callback-based input system
- Window event handling (resize, close)
- Extensible game loop with proper timing

### Performance Features
- **Delta Time Management**: Automatic frame-rate independent timing
- **FPS Tracking**: Real-time performance monitoring
- **Frame Time Clamping**: Prevention of spiral-of-death scenarios
- **Performance Logging**: Comprehensive metrics and analytics
    void OnUpdate(float deltaTime) override {
        // Update game logic
        if (GetInput()->IsKeyPressed(agl::Keys::Space)) {
            std::cout << "Space pressed!" << std::endl;
        }
    }

## 🚀 Getting Started for Developers

### Integrating AGL into Your Project

1. **Add as Git Submodule**:
```bash
git submodule add https://github.com/heinsteinh/agl-game-engine.git external/agl
```

2. **CMake Integration**:
```cmake
# In your CMakeLists.txt
add_subdirectory(external/agl/gamelib)
target_link_libraries(your_target PRIVATE gamelib)
```

3. **Include in Your Code**:
```cpp
#include "agl.h"  // Includes everything you need
```

### Development Workflow

1. **Clone and Setup**:
```bash
git clone --recursive https://github.com/heinsteinh/agl-game-engine.git
cd agl-game-engine
```

2. **Quick Test Build**:
```bash
cmake -B build && cmake --build build
./build/bin/agl_renderer_demo
```

3. **Development Build with Debug Info**:
```bash
cmake -B debug_build -DCMAKE_BUILD_TYPE=Debug
cmake --build debug_build
```

## 📊 CI/CD Pipeline

The project includes a comprehensive GitHub Actions workflow:

- **✅ Multi-Platform Builds**: Ubuntu and macOS with Release/Debug configurations
- **🎯 Demo Testing**: Individual builds for all sandbox applications
- **🔍 Code Quality**: Static analysis with cppcheck and clang-tidy
- **📚 Documentation**: Auto-generated Doxygen docs deployed to GitHub Pages
- **🏷️ Automated Releases**: Tagged releases with platform-specific artifacts
- **⚡ Performance Monitoring**: Build size analysis and performance benchmarks

## 🔧 Troubleshooting

### Common Issues

**Segmentation Fault on Startup**
- **Cause**: Calling logging macros (`AGL_INFO`, `AGL_ERROR`, etc.) before `Game::Initialize()`
- **Solution**: Ensure `Game::Initialize()` is called before any logging operations
- **Example**:
```cpp
// ❌ Wrong - logging before initialization
AGL_INFO("Starting game...");
MyGame game;
game.Initialize(1280, 720, "My Game");

// ✅ Correct - logging after initialization
MyGame game;
if (game.Initialize(1280, 720, "My Game")) {
    AGL_INFO("Game initialized successfully!");
    game.Run();
}
```

**Build Errors with Camera System**
- Ensure you're including the correct header: `#include "agl.h"`
- Make sure CMake has found and built the new camera source files
- Clean and rebuild: `rm -rf build && cmake -B build && cmake --build build`

## 🤝 Contributing

1. **Fork the repository**
2. **Create a feature branch**: `git checkout -b feature/amazing-feature`
3. **Make your changes** with proper logging and documentation
4. **Test thoroughly** with all demo applications
5. **Submit a pull request** with detailed description

### Code Style Guidelines
- Use modern C++17 features
- Follow RAII principles
- Add comprehensive logging for debugging
- Include ImGui interfaces for runtime debugging
- Write self-documenting code with clear variable names

## 📜 License

This project is released under the **MIT License** - see the [LICENSE](LICENSE) file for details.

### Third-Party Licenses
- **GLFW**: zlib/libpng license
- **GLM**: MIT license
- **STB**: Public domain
- **Dear ImGui**: MIT license
- **spdlog**: MIT license

## 🔗 Links

- **📚 [Documentation](https://heinsteinh.github.io/agl-game-engine/)** - Auto-generated API docs
- **🚀 [CI/CD Status](https://github.com/heinsteinh/agl-game-engine/actions)** - Build and test status
- **🐛 [Issues](https://github.com/heinsteinh/agl-game-engine/issues)** - Bug reports and feature requests
- **📋 [Releases](https://github.com/heinsteinh/agl-game-engine/releases)** - Download pre-built binaries

---

**Happy Game Development! 🎮✨**

*Built with ❤️ using modern C++17, OpenGL, and best practices for game engine architecture.*
