#include "game.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <imgui.h>

#if defined(__APPLE__)
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl3.h>
#else
    #include <GL/gl.h>
#endif



// Custom game class that extends the base Game class
class MyGame : public agl::Game {
public:
    MyGame() : m_rotation(0.0f) {}

    void OnUpdate(float deltaTime) override {
        // Update game logic here
        m_rotation += deltaTime * 50.0f; // Rotate at 50 degrees per second

        // Example: Handle input
        if (GetInput()->IsKeyPressed(GLFW_KEY_SPACE)) {
            std::cout << "Space key pressed!" << std::endl;
        }

        if (GetInput()->IsMouseButtonPressed(agl::MouseButton::Left)) {
            double x, y;
            GetInput()->GetMousePosition(x, y);
            std::cout << "Left mouse clicked at: (" << x << ", " << y << ")" << std::endl;
        }
    }

    void OnRender() override {
        // Custom rendering code would go here
        // For now, just clear with a different color based on input

        float clearColor = GetInput()->IsKeyHeld(GLFW_KEY_C) ? 0.5f : 0.2f;
        glClearColor(clearColor, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OnImGuiRender() override {
        // Custom ImGui rendering
        ImGui::Begin("My Game Debug");
        ImGui::Text("Custom game info");
        ImGui::Text("Rotation: %.2f degrees", m_rotation);

        if (ImGui::Button("Reset Rotation")) {
            m_rotation = 0.0f;
        }

        ImGui::Text("Press 'C' to change background color");
        ImGui::Text("Press 'Space' to test key input");
        ImGui::Text("Click mouse to test mouse input");
        ImGui::End();
    }

private:
    float m_rotation;
};

int main_MyGame() {
    std::cout << "Initializing AGT Game Engine..." << std::endl;

    // Test GLM (showing it works)
    glm::vec3 testVector(1.0f, 2.0f, 3.0f);
    glm::mat4 testMatrix = glm::translate(glm::mat4(1.0f), testVector);
    std::cout << "GLM test vector: (" << testVector.x << ", " << testVector.y << ", " << testVector.z << ")" << std::endl;

    // Create and run the game
    MyGame game;

    if (game.Initialize(1024, 768, "AGT Game Engine - Encapsulated Demo")) {
        std::cout << "Game initialized successfully!" << std::endl;
        game.Run();
    } else {
        std::cerr << "Failed to initialize game!" << std::endl;
        return -1;
    }

    return 0;
}
