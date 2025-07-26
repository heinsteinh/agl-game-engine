#include "Renderer.h"
#include "agl.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <iostream>

// Enhanced renderer demo showing both old manual camera and new camera system
class AdvancedRendererDemo : public agl::Game {
public:
    AdvancedRendererDemo()
        : m_rotation(0.0f), m_wireframeMode(false), m_animationPaused(false), m_useNewCameraSystem(true)

          // Old camera system variables (for comparison)
          ,
          m_cameraDistance(5.0f), m_cameraYaw(0.0f), m_cameraPitch(0.0f), m_lastMouseX(0.0), m_lastMouseY(0.0),
          m_firstMouse(true), m_mouseLookEnabled(false) {}

    bool Initialize(int width = 1280, int height = 720, const char *title = "AGL Advanced Renderer Demo") {
        if (!agl::Game::Initialize(width, height, title)) {
            AGL_ERROR("Failed to initialize base game class");
            return false;
        }

        AGL_INFO("Initializing AGL Advanced Renderer Demo...");

        // Initialize the AGL renderer
        agl::Renderer::Initialize();
        AGL_INFO("Renderer initialized successfully");

        // === NEW CAMERA SYSTEM ===
        // Create modern camera and controller
        m_camera = std::make_shared<agl::Camera>(glm::vec3(0.0f, 2.0f, 5.0f), // Starting position
                                                 glm::vec3(0.0f, 1.0f, 0.0f), // Up vector
                                                 -90.0f,                      // Yaw (facing forward)
                                                 0.0f                         // Pitch (level)
        );

        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        m_camera->SetPerspective(75.0f, aspectRatio);

        m_cameraController = std::make_unique<agl::CameraController>(m_camera);
        m_cameraController->Initialize(GetInput());

        // Configure camera for smooth demo experience
        agl::CameraSettings settings;
        settings.movementSpeed = 6.0f;
        settings.mouseSensitivity = 0.12f;
        settings.defaultFOV = 75.0f;
        settings.sprintFOV = 85.0f;
        settings.aimFOV = 55.0f;
        settings.movementSmoothing = 0.15f;
        settings.rotationSmoothing = 0.08f;

        m_cameraController->SetSettings(settings);

        // Create a simple shader program
        m_shader = agl::ShaderProgram::CreateBasicColorShader();
        if (!m_shader) {
            AGL_ERROR("Failed to create shader program!");
            return false;
        }
        AGL_INFO("Shader program created successfully");

        // Create triangle vertices
        float triangleVertices[] = {
            -0.5f, -0.5f, 0.0f, // Bottom-left
            0.5f,  -0.5f, 0.0f, // Bottom-right
            0.0f,  0.5f,  0.0f  // Top
        };

        m_triangleVB = std::make_shared<agl::VertexBuffer>(triangleVertices, sizeof(triangleVertices));
        agl::VertexBufferLayout layout = agl::VertexBufferLayout::Position3D();

        m_triangleVA = agl::VertexArray::Create();
        m_triangleVA->AddVertexBuffer(m_triangleVB, layout);

        AGL_INFO("Advanced Renderer Demo initialized successfully!");
        AGL_INFO("Press C to toggle between old/new camera systems");
        AGL_INFO("Current system: NEW Camera System (recommended)");

        return true;
    }

    void OnUpdate(float deltaTime) override {
        // Update rotation animation
        if (!m_animationPaused) {
            m_rotation += deltaTime * 45.0f;
        }

        // Toggle camera system
        if (GetInput()->IsKeyPressed(GLFW_KEY_C)) {
            m_useNewCameraSystem = !m_useNewCameraSystem;
            if (m_useNewCameraSystem) {
                AGL_INFO("Switched to NEW Camera System");
                // Reset camera to a good position
                m_camera->SetPosition(glm::vec3(0.0f, 2.0f, 5.0f));
                m_camera->SetRotation(-90.0f, 0.0f);
            } else {
                AGL_INFO("Switched to OLD Manual Camera System");
                // Reset old camera variables
                m_cameraDistance = 5.0f;
                m_cameraYaw = 0.0f;
                m_cameraPitch = 0.0f;
                m_firstMouse = true;
            }
        }

        // Update appropriate camera system
        if (m_useNewCameraSystem) {
            UpdateNewCameraSystem(deltaTime);
        } else {
            UpdateOldCameraSystem(deltaTime);
        }

        // Common controls
        HandleCommonControls(deltaTime);

        // Update camera matrices
        UpdateMatrices();
    }

    void OnRender() override {
        // Clear with a nice gradient-like color
        agl::Renderer::SetClearColor(0.2f, 0.3f, 0.8f, 1.0f);
        agl::Renderer::Clear();

        if (m_shader && m_triangleVA && m_camera) {
            glm::mat4 viewProj = m_camera->GetViewProjectionMatrix();

            // Draw spinning triangle
            glm::mat4 triangleModel =
                glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 triangleMVP = viewProj * triangleModel;

            m_shader->Use();
            m_shader->SetUniform("u_MVP", triangleMVP);
            m_shader->SetUniform("u_Color", glm::vec4(1.0f, 0.5f, 0.2f, 1.0f)); // Orange

            agl::Renderer::DrawArrays(*m_triangleVA, *m_shader, GL_TRIANGLES, 3);

            // Draw multiple cubes in a grid pattern
            for (int x = -2; x <= 2; x++) {
                for (int z = -2; z <= 2; z++) {
                    if (x == 0 && z == 0)
                        continue; // Skip center where triangle is

                    glm::mat4 cubeModel = glm::translate(glm::mat4(1.0f), glm::vec3(x * 3.0f, 0.0f, z * 3.0f));
                    cubeModel = glm::rotate(cubeModel, glm::radians(m_rotation * 0.5f + x * 20.0f + z * 20.0f),
                                            glm::vec3(1.0f, 1.0f, 0.0f));
                    glm::mat4 cubeMVP = viewProj * cubeModel;

                    // Different colors based on position
                    float r = (x + 2) / 4.0f;
                    float b = (z + 2) / 4.0f;
                    glm::vec4 color(r, 0.6f, b, 1.0f);

                    agl::Renderer::DrawCube(cubeMVP, color);
                }
            }

            // Draw a quad platform
            glm::mat4 quadModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            quadModel = glm::scale(quadModel, glm::vec3(10.0f, 0.1f, 10.0f));
            glm::mat4 quadMVP = viewProj * quadModel;

            agl::Renderer::DrawCube(quadMVP, glm::vec4(0.4f, 0.4f, 0.4f, 1.0f)); // Gray platform
        }
    }

    void OnImGuiRender() override {
        ImGui::Begin("Advanced Renderer Demo");

        // Performance metrics
        ImGui::Text("Performance:");
        ImGui::Text("FPS: %.1f", GetFPS());
        ImGui::Text("Frame Time: %.3f ms", GetDeltaTime() * 1000.0f);

        ImGui::Separator();

        // Camera system toggle
        const char *systemName = m_useNewCameraSystem ? "NEW Camera System" : "OLD Manual System";
        ImGui::Text("Current Camera: %s", systemName);

        if (ImGui::Button("Toggle Camera System (C)")) {
            m_useNewCameraSystem = !m_useNewCameraSystem;
        }

        ImGui::Separator();

        // Camera information
        if (m_camera) {
            glm::vec3 pos = m_camera->GetPosition();
            ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)", pos.x, pos.y, pos.z);
            ImGui::Text("Camera Yaw: %.1f°", m_camera->GetYaw());
            ImGui::Text("Camera Pitch: %.1f°", m_camera->GetPitch());
        }

        if (m_useNewCameraSystem && m_cameraController) {
            ImGui::Text("FOV: %.1f°", m_cameraController->GetCurrentFOV());

            const char *modeNames[] = {"First Person", "Third Person", "Spectator", "Fixed"};
            int currentMode = static_cast<int>(m_cameraController->GetMode());
            ImGui::Text("Camera Mode: %s", modeNames[currentMode]);

            ImGui::Text("States:");
            ImGui::BulletText("Aiming: %s", m_cameraController->IsAiming() ? "YES" : "NO");
            ImGui::BulletText("Sprinting: %s", m_cameraController->IsSprinting() ? "YES" : "NO");
        } else {
            ImGui::Text("Manual Controls:");
            ImGui::Text("Distance: %.1f", m_cameraDistance);
            ImGui::Text("Mouse Look: %s", m_mouseLookEnabled ? "ON" : "OFF");
        }

        ImGui::Separator();

        // Render controls
        ImGui::Text("Render Controls:");
        if (ImGui::Checkbox("Wireframe Mode", &m_wireframeMode)) {
            if (m_wireframeMode) {
                agl::Renderer::EnableWireframe();
            } else {
                agl::Renderer::DisableWireframe();
            }
        }

        ImGui::Checkbox("Pause Animation", &m_animationPaused);

        if (ImGui::Button("Reset Camera")) {
            if (m_useNewCameraSystem && m_cameraController) {
                m_cameraController->Reset();
            } else {
                // Reset old camera
                m_cameraDistance = 5.0f;
                m_cameraYaw = 0.0f;
                m_cameraPitch = 0.0f;
            }
        }

        ImGui::Separator();

        // Controls help
        ImGui::Text("Controls:");
        if (m_useNewCameraSystem) {
            ImGui::BulletText("WASD - Move camera");
            ImGui::BulletText("Mouse - Look around");
            ImGui::BulletText("SHIFT - Sprint");
            ImGui::BulletText("RIGHT CLICK - Aim (zoom)");
            ImGui::BulletText("SPACE - Move up");
            ImGui::BulletText("CTRL - Move down");
        } else {
            ImGui::BulletText("RIGHT CLICK - Toggle mouse look");
            ImGui::BulletText("W/S - Zoom in/out");
            ImGui::BulletText("A/D - Rotate left/right");
            ImGui::BulletText("Q/E - Look up/down");
            ImGui::BulletText("R - Reset camera");
        }
        ImGui::BulletText("TAB - Toggle wireframe");
        ImGui::BulletText("SPACE - Pause animation");
        ImGui::BulletText("C - Toggle camera system");

        ImGui::End();
    }

    void Shutdown() {
        AGL_INFO("Shutting down Advanced Renderer Demo...");

        if (m_shader)
            m_shader.reset();
        if (m_triangleVA)
            m_triangleVA.reset();
        if (m_triangleVB)
            m_triangleVB.reset();
        if (m_cameraController)
            m_cameraController.reset();
        if (m_camera)
            m_camera.reset();

        agl::Renderer::Shutdown();
        agl::Game::Shutdown();

        AGL_INFO("Advanced Renderer Demo shutdown complete");
    }

private:
    // Rendering objects
    std::unique_ptr<agl::ShaderProgram> m_shader;
    std::unique_ptr<agl::VertexArray> m_triangleVA;
    std::shared_ptr<agl::VertexBuffer> m_triangleVB;

    // NEW Camera system
    std::shared_ptr<agl::Camera> m_camera;
    std::unique_ptr<agl::CameraController> m_cameraController;

    // Demo state
    float m_rotation;
    bool m_wireframeMode;
    bool m_animationPaused;
    bool m_useNewCameraSystem;

    // OLD camera system variables (for comparison)
    float m_cameraDistance;
    float m_cameraYaw;
    float m_cameraPitch;
    double m_lastMouseX;
    double m_lastMouseY;
    bool m_firstMouse;
    bool m_mouseLookEnabled;

    void UpdateNewCameraSystem(float deltaTime) {
        if (m_cameraController) {
            m_cameraController->Update(deltaTime);
        }
    }

    void UpdateOldCameraSystem(float deltaTime) {
        // Handle old-style mouse look toggle
        if (GetInput()->IsMouseButtonPressed(agl::MouseButton::Right)) {
            m_mouseLookEnabled = !m_mouseLookEnabled;
            m_firstMouse = true;
        }

        // Handle old-style camera controls
        HandleOldCameraInput(deltaTime);
    }

    void HandleCommonControls(float deltaTime) {
        // Wireframe toggle
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

        // Animation pause
        if (GetInput()->IsKeyPressed(GLFW_KEY_SPACE)) {
            m_animationPaused = !m_animationPaused;
        }

        // Update aspect ratio
        int width = GetWindow()->GetWidth();
        int height = GetWindow()->GetHeight();
        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        m_camera->UpdateAspectRatio(aspectRatio);
    }

    void HandleOldCameraInput(float deltaTime) {
        // Mouse look (old system)
        if (m_mouseLookEnabled) {
            double mouseX, mouseY;
            GetInput()->GetMousePosition(mouseX, mouseY);

            if (m_firstMouse) {
                m_lastMouseX = mouseX;
                m_lastMouseY = mouseY;
                m_firstMouse = false;
            }

            double deltaX = mouseX - m_lastMouseX;
            double deltaY = m_lastMouseY - mouseY;

            m_lastMouseX = mouseX;
            m_lastMouseY = mouseY;

            float sensitivity = 0.1f;
            deltaX *= sensitivity;
            deltaY *= sensitivity;

            m_cameraYaw += static_cast<float>(deltaX);
            m_cameraPitch += static_cast<float>(deltaY);

            if (m_cameraPitch > 89.0f)
                m_cameraPitch = 89.0f;
            if (m_cameraPitch < -89.0f)
                m_cameraPitch = -89.0f;
        }

        // Keyboard camera controls (old system)
        float zoomSpeed = 5.0f;
        if (GetInput()->IsKeyHeld(GLFW_KEY_W)) {
            m_cameraDistance -= deltaTime * zoomSpeed;
        }
        if (GetInput()->IsKeyHeld(GLFW_KEY_S)) {
            m_cameraDistance += deltaTime * zoomSpeed;
        }
        if (GetInput()->IsKeyHeld(GLFW_KEY_A)) {
            m_cameraYaw -= deltaTime * 45.0f;
        }
        if (GetInput()->IsKeyHeld(GLFW_KEY_D)) {
            m_cameraYaw += deltaTime * 45.0f;
        }
        if (GetInput()->IsKeyHeld(GLFW_KEY_Q)) {
            m_cameraPitch -= deltaTime * 45.0f;
        }
        if (GetInput()->IsKeyHeld(GLFW_KEY_E)) {
            m_cameraPitch += deltaTime * 45.0f;
        }

        // Constrain values
        if (m_cameraDistance < 1.0f)
            m_cameraDistance = 1.0f;
        if (m_cameraDistance > 20.0f)
            m_cameraDistance = 20.0f;
        if (m_cameraPitch > 89.0f)
            m_cameraPitch = 89.0f;
        if (m_cameraPitch < -89.0f)
            m_cameraPitch = -89.0f;

        // Reset with R key
        if (GetInput()->IsKeyPressed(GLFW_KEY_R)) {
            m_cameraDistance = 5.0f;
            m_cameraYaw = 0.0f;
            m_cameraPitch = 0.0f;
        }
    }

    void UpdateMatrices() {
        if (!m_useNewCameraSystem) {
            // Update camera manually for old system
            float x = m_cameraDistance * cos(glm::radians(m_cameraPitch)) * cos(glm::radians(m_cameraYaw));
            float y = m_cameraDistance * sin(glm::radians(m_cameraPitch));
            float z = m_cameraDistance * cos(glm::radians(m_cameraPitch)) * sin(glm::radians(m_cameraYaw));

            glm::vec3 cameraPos(x, y, z);
            m_camera->SetPosition(cameraPos);
            m_camera->LookAt(glm::vec3(0.0f, 0.0f, 0.0f));
        }
        // New system updates automatically in CameraController::Update()
    }
};

int main() {
    AdvancedRendererDemo demo;

    if (demo.Initialize(1280, 720, "AGL Advanced Renderer Demo - Old vs New Camera")) {
        AGL_INFO("Demo initialized successfully! Starting main loop...");
        demo.Run();
        AGL_INFO("Demo completed successfully");
    } else {
        AGL_ERROR("Failed to initialize advanced renderer demo!");
        return -1;
    }

    return 0;
}
