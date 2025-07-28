#ifndef AGL_H
#define AGL_H

// AGL Game Engine - Example usage header
// This demonstrates how to use the encapsulated window and input systems

#include "Camera.h"
#include "CameraController.h"
#include "DispatchQueue.h"
#include "Gizmos.h"
#include "Logger.h"
#include "ProjectileSystem.h"
#include "Renderer.h"
#include "ShadowSystem.h"
#include "SigSlot.h"
#include "game.h"
#include "input.h"
#include "mesh.h"
#include "window.h"
#include <imgui.h>

// All AGL (A Graphics Library) classes are in the agl namespace
// Usage examples:

/*
// Basic game setup:
agl::Game game;
if (game.Initialize(1024, 768, "My Game")) {
    game.Run();
}

// Accessing window and input:
agl::Window* window = game.GetWindow();
agl::Input* input = game.GetInput();

// Window operations:
window->SetTitle("New Title");
window->SetVSync(true);
int width = window->GetWidth();
int height = window->GetHeight();

// Input checking:
if (input->IsKeyPressed(GLFW_KEY_SPACE)) {
    // Space was just pressed
}

if (input->IsKeyHeld(GLFW_KEY_W)) {
    // W is being held down
}

if (input->IsMouseButtonPressed(agl::MouseButton::Left)) {
    double x, y;
    input->GetMousePosition(x, y);
    // Left mouse button was just clicked at position (x, y)
}

// Mouse movement:
double deltaX, deltaY;
input->GetMouseDelta(deltaX, deltaY);
// deltaX and deltaY contain mouse movement since last frame

// Scroll wheel:
double scrollX, scrollY;
input->GetScrollOffset(scrollX, scrollY);
// scrollX and scrollY contain scroll wheel movement this frame

// Custom game class:
class MyGame : public agl::Game {
public:
    void OnUpdate(float deltaTime) override {
        // Update game logic
    }

    void OnRender() override {
        // Render game
    }

    void OnImGuiRender() override {
        // Render ImGui interface
    }
};
*/

namespace agl {
// Forward declarations for convenience
class Game;
class Window;
class Input;

// Key and mouse constants for convenience
namespace Keys {
constexpr int Space = GLFW_KEY_SPACE;
constexpr int Enter = GLFW_KEY_ENTER;
constexpr int Escape = GLFW_KEY_ESCAPE;
constexpr int W = GLFW_KEY_W;
constexpr int A = GLFW_KEY_A;
constexpr int S = GLFW_KEY_S;
constexpr int D = GLFW_KEY_D;
constexpr int Up = GLFW_KEY_UP;
constexpr int Down = GLFW_KEY_DOWN;
constexpr int Left = GLFW_KEY_LEFT;
constexpr int Right = GLFW_KEY_RIGHT;
} // namespace Keys
} // namespace agl

#endif // AGL_H
