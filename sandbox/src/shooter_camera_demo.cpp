#include "agl.h"
#include "Renderer.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

// Example game class demonstrating the camera system for shooter games
class ShooterCameraDemo : public agl::Game {
public:
    ShooterCameraDemo()
        : m_rotation(0.0f)
        , m_showInstructions(true)
        , m_wireframeMode(false) {
    }

    bool Initialize(int width = 1280, int height = 720, const char* title = "AGL Shooter Camera Demo") {
        if (!agl::Game::Initialize(width, height, title)) {
            AGL_ERROR("Failed to initialize base game class");
            return false;
        }


        AGL_INFO("Initializing AGL Shooter Camera Demo...");

        // Initialize the AGL renderer
        agl::Renderer::Initialize();
        AGL_INFO("Renderer initialized successfully");

        // Create camera and camera controller
        m_camera = std::make_shared<agl::Camera>(
            glm::vec3(0.0f, 2.0f, 5.0f),  // Position
            glm::vec3(0.0f, 1.0f, 0.0f),  // Up vector
            -90.0f,                        // Yaw
            0.0f                          // Pitch
        );

        // Set up perspective projection
        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        m_camera->SetPerspective(75.0f, aspectRatio);

        // Create camera controller
        m_cameraController = std::make_unique<agl::CameraController>(m_camera);
        m_cameraController->Initialize(GetInput());

        // Configure camera settings for shooter gameplay
        agl::CameraSettings settings;
        settings.movementSpeed = 8.0f;          // Fast movement for FPS
        settings.sprintMultiplier = 2.0f;       // Double speed when sprinting
        settings.crouchMultiplier = 0.4f;       // Slow when crouching
        settings.mouseSensitivity = 0.15f;      // Good sensitivity for aiming
        settings.defaultFOV = 75.0f;            // Standard FOV
        settings.sprintFOV = 85.0f;             // Wider FOV when sprinting
        settings.aimFOV = 50.0f;                // Narrow FOV when aiming
        settings.fovTransitionSpeed = 8.0f;     // Fast FOV transitions
        settings.movementSmoothing = 0.2f;      // Some smoothing for fluid movement
        settings.rotationSmoothing = 0.1f;      // Light rotation smoothing
        settings.shakeIntensity = 1.0f;         // Full shake intensity
        settings.constrainPitch = true;         // Prevent camera flipping

        m_cameraController->SetSettings(settings);

        // Create a simple shader program
        m_shader = agl::ShaderProgram::CreateBasicColorShader();
        if (!m_shader) {
            AGL_ERROR("Failed to create shader program!");
            return false;
        }
        AGL_INFO("Shader program created successfully");

        // Create simple ground plane vertices
        float planeVertices[] = {
            // Ground plane (larger)
            -20.0f, 0.0f, -20.0f,
             20.0f, 0.0f, -20.0f,
             20.0f, 0.0f,  20.0f,
            -20.0f, 0.0f,  20.0f,
        };

        unsigned int planeIndices[] = {
            0, 1, 2,
            2, 3, 0
        };

        // Create vertex buffer and index buffer for ground
        m_planeVB = std::make_shared<agl::VertexBuffer>(planeVertices, sizeof(planeVertices));
        m_planeIB = std::make_shared<agl::IndexBuffer>(planeIndices, 6);
        agl::VertexBufferLayout layout = agl::VertexBufferLayout::Position3D();

        // Create vertex array for ground
        m_planeVA = agl::VertexArray::Create();
        m_planeVA->AddVertexBuffer(m_planeVB, layout);
        m_planeVA->SetIndexBuffer(m_planeIB);

        AGL_INFO("AGL Shooter Camera Demo initialized successfully!");
        AGL_INFO("==== SHOOTER CAMERA CONTROLS ====");
        AGL_INFO("Movement: WASD keys");
        AGL_INFO("Look: Mouse movement");
        AGL_INFO("Sprint: Hold LEFT SHIFT");
        AGL_INFO("Crouch: Hold LEFT CTRL");
        AGL_INFO("Aim: Hold RIGHT MOUSE BUTTON");
        AGL_INFO("Jump/Up: SPACE");
        AGL_INFO("Toggle Wireframe: TAB");
        AGL_INFO("Camera Shake Test: X key");
        AGL_INFO("Switch Camera Mode: 1-4 keys");
        AGL_INFO("==================================");

        return true;
    }

    void OnUpdate(float deltaTime) override {
        // Update rotation for scene objects
        m_rotation += deltaTime * 30.0f; // 30 degrees per second

        // Update camera controller
        m_cameraController->Update(deltaTime);

        // Handle mode switching
        if (GetInput()->IsKeyPressed(GLFW_KEY_1)) {
            m_cameraController->SetMode(agl::CameraMode::FirstPerson);
            AGL_INFO("Switched to First Person mode");
        }
        if (GetInput()->IsKeyPressed(GLFW_KEY_2)) {
            m_cameraController->SetMode(agl::CameraMode::ThirdPerson);
            m_cameraController->SetTarget(glm::vec3(0.0f, 1.0f, 0.0f)); // Look at center
            AGL_INFO("Switched to Third Person mode");
        }
        if (GetInput()->IsKeyPressed(GLFW_KEY_3)) {
            m_cameraController->SetMode(agl::CameraMode::Spectator);
            AGL_INFO("Switched to Spectator mode");
        }
        if (GetInput()->IsKeyPressed(GLFW_KEY_4)) {
            m_cameraController->SetMode(agl::CameraMode::Fixed);
            m_cameraController->SetTarget(glm::vec3(0.0f, 1.0f, 0.0f)); // Look at center
            AGL_INFO("Switched to Fixed mode");
        }

        // Handle wireframe toggle
        if (GetInput()->IsKeyPressed(GLFW_KEY_TAB)) {
            m_wireframeMode = !m_wireframeMode;
            if (m_wireframeMode) {
                agl::Renderer::EnableWireframe();
                AGL_INFO("Wireframe mode enabled");
            } else {
                agl::Renderer::DisableWireframe();
                AGL_INFO("Wireframe mode disabled");
            }
        }

        // Test camera shake
        if (GetInput()->IsKeyPressed(GLFW_KEY_X)) {
            m_cameraController->AddShake(0.5f, 1.0f); // Medium intensity, 1 second
            AGL_INFO("Camera shake triggered!");
        }

        // Handle instructions toggle
        if (GetInput()->IsKeyPressed(GLFW_KEY_H)) {
            m_showInstructions = !m_showInstructions;
        }

        // Update camera aspect ratio if window size changed
        int width = GetWindow()->GetWidth();
        int height = GetWindow()->GetHeight();
        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        m_camera->UpdateAspectRatio(aspectRatio);
    }

    void OnRender() override {
        // Clear with a sky blue color
        agl::Renderer::SetClearColor(0.5f, 0.7f, 1.0f, 1.0f);
        agl::Renderer::Clear();

        if (m_shader && m_camera) {
            glm::mat4 viewProj = m_camera->GetViewProjectionMatrix();

            // Draw ground plane
            if (m_planeVA) {
                glm::mat4 groundModel = glm::mat4(1.0f);
                glm::mat4 groundMVP = viewProj * groundModel;

                m_shader->Use();
                m_shader->SetUniform("u_MVP", groundMVP);
                m_shader->SetUniform("u_Color", glm::vec4(0.3f, 0.6f, 0.3f, 1.0f)); // Green ground

                agl::Renderer::DrawElements(*m_planeVA, *m_shader, GL_TRIANGLES);
            }

            // Draw some cubes as reference objects
            for (int x = -10; x <= 10; x += 5) {
                for (int z = -10; z <= 10; z += 5) {
                    if (x == 0 && z == 0) continue; // Skip center

                    glm::mat4 cubeModel = glm::translate(glm::mat4(1.0f), glm::vec3(x, 1.0f, z));
                    cubeModel = glm::rotate(cubeModel, glm::radians(m_rotation), glm::vec3(0.0f, 1.0f, 0.0f));
                    glm::mat4 cubeMVP = viewProj * cubeModel;

                    // Color based on distance from center
                    float distance = sqrt(x*x + z*z);
                    glm::vec4 color = glm::vec4(1.0f - distance/15.0f, 0.5f, distance/15.0f, 1.0f);

                    agl::Renderer::DrawCube(cubeMVP, color);
                }
            }

            // Draw a central tall object as a reference point
            glm::mat4 centerModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f));
            centerModel = glm::scale(centerModel, glm::vec3(0.5f, 4.0f, 0.5f));
            centerModel = glm::rotate(centerModel, glm::radians(m_rotation * 2.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 centerMVP = viewProj * centerModel;

            agl::Renderer::DrawCube(centerMVP, glm::vec4(1.0f, 0.8f, 0.2f, 1.0f)); // Golden tower
        }
    }

    void OnImGuiRender() override {
        // Main camera info window
        ImGui::Begin("Shooter Camera Demo");

        // Performance metrics
        ImGui::Text("Performance:");
        ImGui::Text("FPS: %.1f", GetFPS());
        ImGui::Text("Frame Time: %.3f ms", GetDeltaTime() * 1000.0f);

        ImGui::Separator();

        // Camera information
        if (m_camera) {
            glm::vec3 pos = m_camera->GetPosition();
            ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)", pos.x, pos.y, pos.z);
            ImGui::Text("Camera Yaw: %.1f°", m_camera->GetYaw());
            ImGui::Text("Camera Pitch: %.1f°", m_camera->GetPitch());
            ImGui::Text("Camera FOV: %.1f°", m_cameraController->GetCurrentFOV());
        }

        ImGui::Separator();

        // Camera controller status
        if (m_cameraController) {
            const char* modeNames[] = { "First Person", "Third Person", "Spectator", "Fixed" };
            int currentMode = static_cast<int>(m_cameraController->GetMode());
            ImGui::Text("Camera Mode: %s", modeNames[currentMode]);

            ImGui::Text("States:");
            ImGui::BulletText("Aiming: %s", m_cameraController->IsAiming() ? "YES" : "NO");
            ImGui::BulletText("Sprinting: %s", m_cameraController->IsSprinting() ? "YES" : "NO");
            ImGui::BulletText("Crouching: %s", m_cameraController->IsCrouching() ? "YES" : "NO");
        }

        ImGui::Separator();

        // Settings controls
        if (m_cameraController) {
            auto& settings = m_cameraController->GetSettings();

            if (ImGui::CollapsingHeader("Camera Settings")) {
                ImGui::SliderFloat("Movement Speed", &settings.movementSpeed, 1.0f, 20.0f);
                ImGui::SliderFloat("Mouse Sensitivity", &settings.mouseSensitivity, 0.01f, 1.0f);
                ImGui::SliderFloat("Default FOV", &settings.defaultFOV, 45.0f, 120.0f);
                ImGui::SliderFloat("Sprint FOV", &settings.sprintFOV, 45.0f, 120.0f);
                ImGui::SliderFloat("Aim FOV", &settings.aimFOV, 20.0f, 90.0f);

                ImGui::Checkbox("Invert Y", &settings.invertY);

                if (ImGui::Button("Reset Settings")) {
                    settings = agl::CameraSettings(); // Reset to defaults
                    m_cameraController->SetSettings(settings);
                }
            }
        }

        ImGui::Separator();

        // Controls
        if (ImGui::Button("Test Camera Shake")) {
            m_cameraController->AddShake(1.0f, 0.8f);
        }

        if (ImGui::Button("Reset Camera")) {
            m_cameraController->Reset();
        }

        ImGui::Checkbox("Wireframe Mode", &m_wireframeMode);
        if (m_wireframeMode) {
            agl::Renderer::EnableWireframe();
        } else {
            agl::Renderer::DisableWireframe();
        }

        ImGui::End();

        // Instructions window
        if (m_showInstructions) {
            ImGui::Begin("Controls (Press H to toggle)", &m_showInstructions);

            ImGui::Text("=== MOVEMENT ===");
            ImGui::BulletText("W/A/S/D - Move forward/left/backward/right");
            ImGui::BulletText("SPACE - Move up/jump");
            ImGui::BulletText("LEFT CTRL - Crouch/move down");
            ImGui::BulletText("LEFT SHIFT - Sprint (hold)");

            ImGui::Text("\n=== CAMERA ===");
            ImGui::BulletText("Mouse - Look around");
            ImGui::BulletText("RIGHT MOUSE - Aim (changes FOV)");
            ImGui::BulletText("Mouse Wheel - Zoom");

            ImGui::Text("\n=== MODES ===");
            ImGui::BulletText("1 - First Person");
            ImGui::BulletText("2 - Third Person");
            ImGui::BulletText("3 - Spectator");
            ImGui::BulletText("4 - Fixed");

            ImGui::Text("\n=== OTHER ===");
            ImGui::BulletText("TAB - Toggle wireframe");
            ImGui::BulletText("X - Test camera shake");
            ImGui::BulletText("H - Toggle this help");

            ImGui::End();
        }
    }

    void Shutdown() {
        AGL_INFO("Shutting down AGL Shooter Camera Demo...");

        // Clean up resources
        if (m_shader) {
            m_shader.reset();
        }

        if (m_planeVA) {
            m_planeVA.reset();
        }

        if (m_planeVB) {
            m_planeVB.reset();
        }

        if (m_planeIB) {
            m_planeIB.reset();
        }

        if (m_cameraController) {
            m_cameraController.reset();
        }

        if (m_camera) {
            m_camera.reset();
        }

        // Shutdown renderer
        agl::Renderer::Shutdown();

        // Call parent shutdown
        agl::Game::Shutdown();

        AGL_INFO("Shooter Camera Demo shutdown complete");
    }

private:
    // Rendering objects
    std::unique_ptr<agl::ShaderProgram> m_shader;
    std::unique_ptr<agl::VertexArray> m_planeVA;
    std::shared_ptr<agl::VertexBuffer> m_planeVB;
    std::shared_ptr<agl::IndexBuffer> m_planeIB;

    // Camera system
    std::shared_ptr<agl::Camera> m_camera;
    std::unique_ptr<agl::CameraController> m_cameraController;

    // Demo state
    float m_rotation;
    bool m_showInstructions;
    bool m_wireframeMode;
};


int main() {

    ShooterCameraDemo demo;

    if (demo.Initialize(1280, 720, "AGL Shooter Camera Demo - FPS Style Controls")) {
        AGL_INFO("Demo initialized successfully! Starting main loop...");
        demo.Run();
        AGL_INFO("Demo completed successfully");
    } else {
        AGL_ERROR("Failed to initialize shooter camera demo!");
        return -1;
    }

    return 0;
}
