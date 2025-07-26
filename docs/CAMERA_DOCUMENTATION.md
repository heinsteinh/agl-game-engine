# AGL Camera System Documentation

The AGL Game Engine provides a comprehensive 3D camera system designed specifically for modern game development, with particular emphasis on shooter games and interactive 3D applications.

## Table of Contents

- [Overview](#overview)
- [Quick Start](#quick-start)
- [Camera Class](#camera-class)
- [Camera Controller](#camera-controller)
- [Camera Modes](#camera-modes)
- [Shooter Game Features](#shooter-game-features)
- [Configuration](#configuration)
- [Advanced Features](#advanced-features)
- [Code Examples](#code-examples)
- [Best Practices](#best-practices)
- [Troubleshooting](#troubleshooting)

## Overview

The AGL camera system consists of two main components:

- **`agl::Camera`** - Core camera class handling projection, view matrices, and basic transformations
- **`agl::CameraController`** - High-level controller providing input handling, movement modes, and game-specific features

### Key Features

- 🎯 **Multiple Camera Modes**: First-person, third-person, spectator, and fixed modes
- 🎮 **Shooter Game Ready**: Sprint, crouch, aim, camera shake, dynamic FOV
- 📐 **Projection Support**: Both perspective and orthographic projections
- 🎪 **Smooth Movement**: Configurable interpolation and smoothing
- 🖱️ **Advanced Input**: Mouse look, keyboard movement, scroll wheel zoom
- ⚡ **Performance Optimized**: Efficient matrix calculations and view frustum culling
- 🛠️ **Highly Configurable**: Extensive settings for customization

## Quick Start

### Basic Setup

```cpp
#include "agl.h"

class MyGame : public agl::Game {
public:
    bool Initialize(int width, int height, const char* title) override {
        if (!agl::Game::Initialize(width, height, title)) {
            return false;
        }

        // Create camera
        m_camera = std::make_shared<agl::Camera>(
            glm::vec3(0.0f, 2.0f, 5.0f),  // Position
            glm::vec3(0.0f, 1.0f, 0.0f),  // Up vector
            -90.0f,                        // Yaw (facing forward)
            0.0f                          // Pitch (level)
        );

        // Set perspective projection
        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        m_camera->SetPerspective(75.0f, aspectRatio);

        // Create camera controller
        m_cameraController = std::make_unique<agl::CameraController>(m_camera);
        m_cameraController->Initialize(GetInput());

        return true;
    }

    void OnUpdate(float deltaTime) override {
        // Update camera (handles all input automatically)
        m_cameraController->Update(deltaTime);
    }

    void OnRender() override {
        // Use camera for rendering
        glm::mat4 viewProjection = m_camera->GetViewProjectionMatrix();

        // Apply to your shaders and render scene
        // ...
    }

private:
    std::shared_ptr<agl::Camera> m_camera;
    std::unique_ptr<agl::CameraController> m_cameraController;
};
```

### Shooter Game Setup

```cpp
// Configure for shooter gameplay
agl::CameraSettings settings;
settings.movementSpeed = 8.0f;          // Fast movement
settings.sprintMultiplier = 2.0f;       // Double speed when sprinting
settings.crouchMultiplier = 0.4f;       // Slow when crouching
settings.mouseSensitivity = 0.15f;      // Good sensitivity for aiming
settings.defaultFOV = 75.0f;            // Standard FOV
settings.aimFOV = 50.0f;                // Narrow FOV when aiming
settings.sprintFOV = 85.0f;             // Wide FOV when sprinting
settings.fovTransitionSpeed = 8.0f;     // Fast FOV transitions

m_cameraController->SetSettings(settings);
```

## Camera Class

The `agl::Camera` class is the core component providing camera transformation matrices and projection settings.

### Construction

```cpp
// Constructor with vectors
agl::Camera(
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
    float yaw = -90.0f,
    float pitch = 0.0f,
    float roll = 0.0f
);

// Constructor with scalar values
agl::Camera(
    float posX, float posY, float posZ,
    float upX, float upY, float upZ,
    float yaw = -90.0f,
    float pitch = 0.0f,
    float roll = 0.0f
);
```

### Matrix Operations

```cpp
// Get transformation matrices
glm::mat4 viewMatrix = camera->GetViewMatrix();
glm::mat4 projectionMatrix = camera->GetProjectionMatrix();
glm::mat4 viewProjectionMatrix = camera->GetViewProjectionMatrix();
```

### Position and Rotation

```cpp
// Set position
camera->SetPosition(glm::vec3(10.0f, 5.0f, 0.0f));

// Set rotation (yaw, pitch, roll in degrees)
camera->SetRotation(45.0f, -15.0f, 0.0f);

// Look at a target
camera->LookAt(glm::vec3(0.0f, 0.0f, 0.0f));

// Get current values
glm::vec3 position = camera->GetPosition();
float yaw = camera->GetYaw();
float pitch = camera->GetPitch();
```

### Projection Settings

```cpp
// Perspective projection
camera->SetPerspective(
    75.0f,      // Field of view (degrees)
    16.0f/9.0f, // Aspect ratio
    0.1f,       // Near plane
    1000.0f     // Far plane
);

// Orthographic projection
camera->SetOrthographic(
    -10.0f, 10.0f,  // Left, Right
    -10.0f, 10.0f,  // Bottom, Top
    -1.0f, 1.0f     // Near, Far
);

// Update aspect ratio (call when window resizes)
camera->UpdateAspectRatio(newAspectRatio);
```

### Input Processing

```cpp
// Process keyboard movement
camera->ProcessKeyboard(agl::CameraMovement::Forward, deltaTime);
camera->ProcessKeyboard(agl::CameraMovement::Backward, deltaTime);
camera->ProcessKeyboard(agl::CameraMovement::Left, deltaTime);
camera->ProcessKeyboard(agl::CameraMovement::Right, deltaTime);

// Process mouse movement
camera->ProcessMouseMovement(xOffset, yOffset, true); // constrainPitch = true

// Process scroll wheel
camera->ProcessMouseScroll(yOffset);
```

## Camera Controller

The `agl::CameraController` class provides high-level camera control with game-specific features.

### Initialization

```cpp
auto cameraController = std::make_unique<agl::CameraController>(camera);
cameraController->Initialize(inputSystem);
```

### Camera Modes

```cpp
// Set camera mode
cameraController->SetMode(agl::CameraMode::FirstPerson);
cameraController->SetMode(agl::CameraMode::ThirdPerson);
cameraController->SetMode(agl::CameraMode::Spectator);
cameraController->SetMode(agl::CameraMode::Fixed);

// Get current mode
agl::CameraMode currentMode = cameraController->GetMode();
```

### Configuration

```cpp
// Get and modify settings
agl::CameraSettings& settings = cameraController->GetSettings();
settings.movementSpeed = 10.0f;
settings.mouseSensitivity = 0.2f;
settings.invertY = true;

// Apply modified settings
cameraController->SetSettings(settings);
```

## Camera Modes

### First Person Mode

Standard first-person shooter camera with direct control.

**Features:**
- Direct WASD movement
- Mouse look
- Sprint/crouch modifiers
- Aim zoom functionality

**Usage:**
```cpp
cameraController->SetMode(agl::CameraMode::FirstPerson);
```

### Third Person Mode

Camera positioned behind and above a target point.

**Features:**
- Automatic target tracking
- Configurable offset and distance
- Smooth following

**Usage:**
```cpp
cameraController->SetMode(agl::CameraMode::ThirdPerson);
cameraController->SetTarget(glm::vec3(0.0f, 1.0f, 0.0f)); // Player position
```

### Spectator Mode

Free-flying camera for level editing or debugging.

**Features:**
- Unrestricted movement in all directions
- No collision detection
- High movement speed options

**Usage:**
```cpp
cameraController->SetMode(agl::CameraMode::Spectator);
```

### Fixed Mode

Stationary camera that can track targets.

**Features:**
- Fixed position
- Optional target tracking
- Useful for cutscenes or security cameras

**Usage:**
```cpp
cameraController->SetMode(agl::CameraMode::Fixed);
cameraController->SetTarget(playerPosition); // Optional target tracking
```

## Shooter Game Features

### Sprint System

```cpp
// Check if sprinting
bool isSprinting = cameraController->IsSprinting();

// Manual control (usually handled automatically)
cameraController->StartSprinting();
cameraController->StopSprinting();
```

**Configuration:**
```cpp
settings.sprintMultiplier = 2.0f;  // Speed multiplier when sprinting
settings.sprintFOV = 85.0f;        // FOV when sprinting
```

### Crouch System

```cpp
// Check if crouching
bool isCrouching = cameraController->IsCrouching();

// Manual control
cameraController->StartCrouching();
cameraController->StopCrouching();
```

**Configuration:**
```cpp
settings.crouchMultiplier = 0.4f;  // Speed multiplier when crouching
```

### Aim System

```cpp
// Check if aiming
bool isAiming = cameraController->IsAiming();

// Manual control
cameraController->StartAiming();
cameraController->StopAiming();
```

**Configuration:**
```cpp
settings.aimFOV = 50.0f;           // FOV when aiming (zoom effect)
settings.fovTransitionSpeed = 8.0f; // Speed of FOV changes
```

### Camera Shake

```cpp
// Add camera shake (e.g., weapon fire, explosions)
cameraController->AddShake(
    0.5f,  // Intensity (0.0 to 1.0+)
    1.0f   // Duration in seconds
);

// Clear all shake effects
cameraController->ClearShake();
```

**Configuration:**
```cpp
settings.shakeIntensity = 1.0f;    // Global shake multiplier
settings.shakeDamping = 5.0f;      // How quickly shake fades
```

### Dynamic FOV

```cpp
// Set FOV directly
cameraController->SetFOV(90.0f);

// Get current FOV (may be interpolating)
float currentFOV = cameraController->GetCurrentFOV();
```

**Configuration:**
```cpp
settings.defaultFOV = 75.0f;       // Normal FOV
settings.sprintFOV = 85.0f;        // FOV when sprinting
settings.aimFOV = 50.0f;           // FOV when aiming
settings.fovTransitionSpeed = 5.0f; // FOV interpolation speed
```

## Configuration

### CameraSettings Structure

```cpp
struct CameraSettings {
    // Movement settings
    float movementSpeed = 5.0f;
    float sprintMultiplier = 2.0f;
    float crouchMultiplier = 0.5f;

    // Mouse settings
    float mouseSensitivity = 0.1f;
    bool invertY = false;

    // Smoothing settings
    float movementSmoothing = 0.1f;
    float rotationSmoothing = 0.1f;

    // Third person settings
    float thirdPersonDistance = 5.0f;
    float thirdPersonHeight = 2.0f;
    glm::vec3 thirdPersonOffset = glm::vec3(0.0f, 0.0f, 0.0f);

    // Constraints
    bool constrainPitch = true;
    float minPitch = -89.0f;
    float maxPitch = 89.0f;

    // Field of view settings
    float defaultFOV = 75.0f;
    float sprintFOV = 85.0f;
    float aimFOV = 50.0f;
    float fovTransitionSpeed = 5.0f;

    // Shake settings
    float shakeIntensity = 1.0f;
    float shakeDamping = 5.0f;
};
```

### Key Bindings

Default key bindings can be customized by modifying the CameraController source:

```cpp
// Movement keys
int GetMoveForwardKey() const { return GLFW_KEY_W; }
int GetMoveBackwardKey() const { return GLFW_KEY_S; }
int GetMoveLeftKey() const { return GLFW_KEY_A; }
int GetMoveRightKey() const { return GLFW_KEY_D; }
int GetMoveUpKey() const { return GLFW_KEY_SPACE; }
int GetMoveDownKey() const { return GLFW_KEY_LEFT_CONTROL; }

// Special keys
int GetSprintKey() const { return GLFW_KEY_LEFT_SHIFT; }
int GetAimKey() const { return GLFW_MOUSE_BUTTON_RIGHT; }
```

## Advanced Features

### Screen to World Ray Casting

Useful for mouse picking and weapon targeting:

```cpp
glm::vec3 ray = camera->GetScreenToWorldRay(
    mouseX, mouseY,           // Screen coordinates
    windowWidth, windowHeight // Window dimensions
);

// Use ray for collision detection, object selection, etc.
```

### View Frustum Culling

Check if objects are visible to optimize rendering:

```cpp
bool isVisible = camera->IsInView(
    objectPosition,  // World position
    objectRadius     // Bounding radius (optional)
);

if (isVisible) {
    // Render the object
}
```

### Smoothing and Interpolation

```cpp
// Enable/disable smoothing
cameraController->SetSmoothingEnabled(true);

// Configure smoothing amounts
settings.movementSmoothing = 0.15f;  // 0.0 = instant, 1.0 = very smooth
settings.rotationSmoothing = 0.08f;  // Lower values for more responsive look
```

### Input Control

```cpp
// Temporarily disable input processing
cameraController->SetInputEnabled(false);

// Re-enable input
cameraController->SetInputEnabled(true);

// Check input state
bool inputEnabled = cameraController->IsInputEnabled();
```

## Code Examples

### Complete Shooter Game Implementation

```cpp
#include "agl.h"

class ShooterGame : public agl::Game {
public:
    bool Initialize(int width, int height, const char* title) override {
        if (!agl::Game::Initialize(width, height, title)) {
            return false;
        }

        // Initialize renderer
        agl::Renderer::Initialize();

        // Create camera system
        SetupCamera(width, height);

        // Create game objects and shaders
        SetupScene();

        return true;
    }

    void OnUpdate(float deltaTime) override {
        // Update camera
        m_cameraController->Update(deltaTime);

        // Handle game-specific input
        HandleGameInput();

        // Update game objects
        UpdateGameObjects(deltaTime);
    }

    void OnRender() override {
        agl::Renderer::SetClearColor(0.5f, 0.7f, 1.0f, 1.0f);
        agl::Renderer::Clear();

        // Get camera matrices
        glm::mat4 viewProj = m_camera->GetViewProjectionMatrix();

        // Render scene
        RenderScene(viewProj);
    }

private:
    std::shared_ptr<agl::Camera> m_camera;
    std::unique_ptr<agl::CameraController> m_cameraController;

    void SetupCamera(int width, int height) {
        // Create camera
        m_camera = std::make_shared<agl::Camera>(
            glm::vec3(0.0f, 2.0f, 5.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            -90.0f, 0.0f
        );

        // Set projection
        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        m_camera->SetPerspective(75.0f, aspectRatio);

        // Create controller
        m_cameraController = std::make_unique<agl::CameraController>(m_camera);
        m_cameraController->Initialize(GetInput());

        // Configure for shooter gameplay
        agl::CameraSettings settings;
        settings.movementSpeed = 8.0f;
        settings.sprintMultiplier = 2.0f;
        settings.crouchMultiplier = 0.4f;
        settings.mouseSensitivity = 0.15f;
        settings.defaultFOV = 75.0f;
        settings.aimFOV = 50.0f;
        settings.sprintFOV = 85.0f;
        settings.fovTransitionSpeed = 8.0f;
        settings.movementSmoothing = 0.1f;
        settings.rotationSmoothing = 0.05f;

        m_cameraController->SetSettings(settings);

        AGL_INFO("Camera system initialized for shooter gameplay");
    }

    void HandleGameInput() {
        // Weapon fire (example)
        if (GetInput()->IsMouseButtonPressed(agl::MouseButton::Left)) {
            FireWeapon();
        }

        // Camera mode switching
        if (GetInput()->IsKeyPressed(GLFW_KEY_F)) {
            ToggleCameraMode();
        }

        // Reset camera
        if (GetInput()->IsKeyPressed(GLFW_KEY_R)) {
            m_cameraController->Reset();
        }
    }

    void FireWeapon() {
        // Add camera shake for weapon feedback
        m_cameraController->AddShake(0.3f, 0.2f);

        // Get aim ray for projectile
        double mouseX, mouseY;
        GetInput()->GetMousePosition(mouseX, mouseY);

        glm::vec3 aimRay = m_camera->GetScreenToWorldRay(
            static_cast<float>(mouseX),
            static_cast<float>(mouseY),
            GetWindow()->GetWidth(),
            GetWindow()->GetHeight()
        );

        // Create projectile along aim ray
        CreateProjectile(m_camera->GetPosition(), aimRay);

        AGL_INFO("Weapon fired!");
    }

    void ToggleCameraMode() {
        static bool isThirdPerson = false;
        isThirdPerson = !isThirdPerson;

        if (isThirdPerson) {
            m_cameraController->SetMode(agl::CameraMode::ThirdPerson);
            m_cameraController->SetTarget(GetPlayerPosition());
            AGL_INFO("Switched to third person mode");
        } else {
            m_cameraController->SetMode(agl::CameraMode::FirstPerson);
            AGL_INFO("Switched to first person mode");
        }
    }

    // Placeholder methods
    void SetupScene() { /* Load models, textures, etc. */ }
    void UpdateGameObjects(float deltaTime) { /* Update game logic */ }
    void RenderScene(const glm::mat4& viewProj) { /* Render game objects */ }
    void CreateProjectile(const glm::vec3& pos, const glm::vec3& dir) { /* Create projectile */ }
    glm::vec3 GetPlayerPosition() { return glm::vec3(0.0f, 1.0f, 0.0f); }
};
```

### Camera State Management

```cpp
class CameraManager {
public:
    void SaveCameraState(const std::string& stateName) {
        CameraState state;
        state.position = m_camera->GetPosition();
        state.yaw = m_camera->GetYaw();
        state.pitch = m_camera->GetPitch();
        state.fov = m_cameraController->GetCurrentFOV();
        state.mode = m_cameraController->GetMode();

        m_savedStates[stateName] = state;
        AGL_INFO("Camera state '{}' saved", stateName);
    }

    void LoadCameraState(const std::string& stateName) {
        auto it = m_savedStates.find(stateName);
        if (it != m_savedStates.end()) {
            const CameraState& state = it->second;

            m_camera->SetPosition(state.position);
            m_camera->SetRotation(state.yaw, state.pitch);
            m_cameraController->SetFOV(state.fov);
            m_cameraController->SetMode(state.mode);

            AGL_INFO("Camera state '{}' loaded", stateName);
        }
    }

private:
    struct CameraState {
        glm::vec3 position;
        float yaw, pitch, fov;
        agl::CameraMode mode;
    };

    std::shared_ptr<agl::Camera> m_camera;
    std::unique_ptr<agl::CameraController> m_cameraController;
    std::unordered_map<std::string, CameraState> m_savedStates;
};
```

## Best Practices

### Performance Optimization

1. **Update Only When Needed**: Don't call `Update()` on cameras that aren't active
2. **Frustum Culling**: Use `IsInView()` to skip rendering invisible objects
3. **Level of Detail**: Adjust rendering quality based on distance from camera
4. **Batch Matrix Updates**: Update projection matrix only when aspect ratio changes

```cpp
// Efficient camera update
if (m_cameraController->IsInputEnabled() && m_isActiveCamera) {
    m_cameraController->Update(deltaTime);
}

// Frustum culling example
for (const auto& object : m_gameObjects) {
    if (m_camera->IsInView(object.position, object.boundingRadius)) {
        object.Render(viewProjection);
    }
}
```

### Memory Management

1. **Use Smart Pointers**: `std::shared_ptr` for cameras, `std::unique_ptr` for controllers
2. **Avoid Copies**: Pass cameras by reference or pointer
3. **Clean Shutdown**: Reset controllers before cameras

```cpp
// Good memory management
class GameScene {
private:
    std::shared_ptr<agl::Camera> m_camera;           // Can be shared
    std::unique_ptr<agl::CameraController> m_controller; // Unique ownership

public:
    void Shutdown() {
        m_controller.reset();  // Reset controller first
        m_camera.reset();      // Then reset camera
    }
};
```

### Input Handling

1. **Separate Concerns**: Use controller for movement, handle game actions separately
2. **Input Validation**: Check input state before processing
3. **Context Sensitivity**: Disable camera input during menus/cutscenes

```cpp
void OnUpdate(float deltaTime) override {
    // Only update camera if game is active
    if (m_gameState == GameState::Playing) {
        m_cameraController->Update(deltaTime);
    } else {
        // Disable camera input during menus
        m_cameraController->SetInputEnabled(false);
    }
}
```

### Configuration Management

1. **Save Settings**: Persist camera settings between sessions
2. **Validate Ranges**: Ensure settings are within reasonable bounds
3. **Provide Defaults**: Have sensible default values for all settings

```cpp
void SaveCameraSettings(const std::string& filename) {
    const auto& settings = m_cameraController->GetSettings();

    nlohmann::json config;
    config["movementSpeed"] = settings.movementSpeed;
    config["mouseSensitivity"] = settings.mouseSensitivity;
    config["invertY"] = settings.invertY;
    config["defaultFOV"] = settings.defaultFOV;

    std::ofstream file(filename);
    file << config.dump(4);
}
```

## Troubleshooting

### Common Issues

#### Camera Not Moving

**Symptoms**: Camera doesn't respond to input
**Causes & Solutions**:
- Input not initialized: Call `cameraController->Initialize(inputSystem)`
- Input disabled: Check `cameraController->IsInputEnabled()`
- Wrong camera mode: Ensure mode supports movement (not Fixed mode)
- Update not called: Make sure `cameraController->Update(deltaTime)` is called

```cpp
// Debug camera state
AGL_INFO("Camera input enabled: {}", m_cameraController->IsInputEnabled());
AGL_INFO("Camera mode: {}", static_cast<int>(m_cameraController->GetMode()));
AGL_INFO("Camera position: {:.2f}, {:.2f}, {:.2f}",
         pos.x, pos.y, pos.z);
```

#### Jerky Movement

**Symptoms**: Camera movement is not smooth
**Causes & Solutions**:
- High mouse sensitivity: Reduce `mouseSensitivity` value
- No smoothing: Enable smoothing with appropriate values
- Frame rate issues: Check delta time values

```cpp
// Smooth movement configuration
agl::CameraSettings settings;
settings.mouseSensitivity = 0.1f;      // Lower for smoother look
settings.movementSmoothing = 0.15f;     // Add movement smoothing
settings.rotationSmoothing = 0.08f;     // Add rotation smoothing
m_cameraController->SetSettings(settings);
```

#### FOV Not Changing

**Symptoms**: Field of view doesn't update during aim/sprint
**Causes & Solutions**:
- Slow transition speed: Increase `fovTransitionSpeed`
- Same FOV values: Check that aim/sprint FOV differs from default
- Update not called: Ensure controller update is running

```cpp
// Check FOV settings
const auto& settings = m_cameraController->GetSettings();
AGL_INFO("Default FOV: {:.1f}", settings.defaultFOV);
AGL_INFO("Aim FOV: {:.1f}", settings.aimFOV);
AGL_INFO("Current FOV: {:.1f}", m_cameraController->GetCurrentFOV());
```

#### Third Person Camera Issues

**Symptoms**: Camera doesn't follow target properly
**Causes & Solutions**:
- No target set: Call `cameraController->SetTarget(targetPosition)`
- Wrong distance/height: Adjust `thirdPersonDistance` and `thirdPersonHeight`
- Target not updated: Update target position each frame

```cpp
// Third person setup
m_cameraController->SetMode(agl::CameraMode::ThirdPerson);
m_cameraController->SetTarget(playerPosition);

// Update target each frame
glm::vec3 currentPlayerPos = GetPlayerPosition();
m_cameraController->SetTarget(currentPlayerPos);
```

### Performance Issues

#### Low Frame Rate

**Symptoms**: Poor performance with camera system
**Causes & Solutions**:
- Excessive matrix calculations: Cache matrices when possible
- Too many view frustum checks: Optimize culling frequency
- Smooth interpolation overhead: Reduce smoothing or disable when not needed

```cpp
// Performance optimization
if (cameraMoved || windowResized) {
    // Only recalculate when necessary
    glm::mat4 viewProj = m_camera->GetViewProjectionMatrix();
    m_cachedViewProj = viewProj;
    m_matrixDirty = false;
}
```

### Debug Tools

#### Camera Information Display

```cpp
void RenderCameraDebugInfo() {
    if (ImGui::Begin("Camera Debug")) {
        glm::vec3 pos = m_camera->GetPosition();
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
        ImGui::Text("Yaw: %.1f°", m_camera->GetYaw());
        ImGui::Text("Pitch: %.1f°", m_camera->GetPitch());
        ImGui::Text("FOV: %.1f°", m_cameraController->GetCurrentFOV());

        const char* modeNames[] = {"First Person", "Third Person", "Spectator", "Fixed"};
        int mode = static_cast<int>(m_cameraController->GetMode());
        ImGui::Text("Mode: %s", modeNames[mode]);

        ImGui::Text("States:");
        ImGui::Text("  Aiming: %s", m_cameraController->IsAiming() ? "YES" : "NO");
        ImGui::Text("  Sprinting: %s", m_cameraController->IsSprinting() ? "YES" : "NO");
        ImGui::Text("  Crouching: %s", m_cameraController->IsCrouching() ? "YES" : "NO");
    }
    ImGui::End();
}
```

---

*For more information, see the [AGL Game Engine Documentation](https://heinsteinh.github.io/agl-game-engine/) or check the [source code examples](https://github.com/heinsteinh/agl-game-engine/tree/main/sandbox/src).*
