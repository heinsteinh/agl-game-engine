#include "agl.h"
#include "Renderer.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

// Example game class demonstrating the AGL renderer
class RendererDemo : public agl::Game {
public:
    RendererDemo()
        : m_rotation(0.0f)
        , m_wireframeMode(false)
        , m_animationPaused(false)
        , m_cameraDistance(5.0f)
        , m_cameraYaw(0.0f)
        , m_cameraPitch(0.0f)
        , m_lastMouseX(0.0)
        , m_lastMouseY(0.0)
        , m_firstMouse(true)
        , m_mouseLookEnabled(false) {
    }

    bool Initialize(int width = 1024, int height = 768, const char* title = "AGL Renderer Demo") {
        if (!agl::Game::Initialize(width, height, title)) {
            AGL_ERROR("Failed to initialize base game class");
            return false;
        }

        AGL_INFO("Initializing AGL Renderer Demo...");

        // Initialize the AGL renderer
        agl::Renderer::Initialize();
        AGL_INFO("Renderer initialized successfully");

        // Create a simple shader program
        m_shader = agl::ShaderProgram::CreateBasicColorShader();
        if (!m_shader) {
            AGL_ERROR("Failed to create shader program!");
            return false;
        }
        AGL_INFO("Shader program created successfully");

        // Create triangle vertices with improved positioning
        float triangleVertices[] = {
            -0.5f, -0.5f, 0.0f,  // Bottom-left
             0.5f, -0.5f, 0.0f,  // Bottom-right
             0.0f,  0.5f, 0.0f   // Top
        };

        // Create vertex buffer and layout
        m_triangleVB = std::make_shared<agl::VertexBuffer>(triangleVertices, sizeof(triangleVertices));
        agl::VertexBufferLayout layout = agl::VertexBufferLayout::Position3D();

        // Create vertex array and set up the triangle
        m_triangleVA = agl::VertexArray::Create();
        m_triangleVA->AddVertexBuffer(m_triangleVB, layout);

        // Set up camera matrices
        UpdateMatrices();

        AGL_INFO("AGL Renderer Demo initialized successfully!");
        AGL_INFO("Controls: TAB=wireframe, Right Mouse=mouse look, W/S=zoom, R=reset camera");
        return true;
    }

    void OnUpdate(float deltaTime) override {
        // Update rotation with performance logging (respect pause state)
        if (!m_animationPaused) {
            m_rotation += deltaTime * 45.0f; // 45 degrees per second
        }

        // Log performance periodically
        static float perfTimer = 0.0f;
        perfTimer += deltaTime;
        if (perfTimer >= 5.0f) {
            AGL_INFO("Renderer Demo Performance - FPS: {:.1f}, Delta: {:.3f}ms, Objects: 3",
                    GetFPS(), deltaTime * 1000.0f);
            perfTimer = 0.0f;
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

        // Toggle mouse look with right mouse button
        if (GetInput()->IsMouseButtonPressed(agl::MouseButton::Right)) {
            m_mouseLookEnabled = !m_mouseLookEnabled;
            m_firstMouse = true; // Reset to avoid jumpy camera
            if (m_mouseLookEnabled) {
                AGL_INFO("Mouse look enabled");
            } else {
                AGL_INFO("Mouse look disabled");
            }
        }

        // Handle pause/resume with spacebar
        if (GetInput()->IsKeyPressed(GLFW_KEY_SPACE)) {
            m_animationPaused = !m_animationPaused;
            if (m_animationPaused) {
                AGL_INFO("Animation paused");
            } else {
                AGL_INFO("Animation resumed");
            }
        }

        // Handle camera controls
        HandleCameraInput(deltaTime);

        // Update matrices
        UpdateMatrices();
    }

    void OnRender() override {
        // Clear with a nice blue color
        agl::Renderer::SetClearColor(0.2f, 0.3f, 0.8f, 1.0f);
        agl::Renderer::Clear();

        if (m_shader && m_triangleVA) {
            // Create model matrix with rotation (respect pause state)
            float currentRotation = m_animationPaused ? m_rotation : m_rotation;
            glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(currentRotation), glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 mvp = m_projection * m_view * model;

            // Draw triangle with enhanced positioning
            m_shader->Use();
            m_shader->SetUniform("u_MVP", mvp);
            m_shader->SetUniform("u_Color", glm::vec4(1.0f, 0.5f, 0.2f, 1.0f)); // Orange color

            agl::Renderer::DrawArrays(*m_triangleVA, *m_shader, GL_TRIANGLES, 3);

            // Draw built-in quad with improved positioning
            glm::mat4 quadModel = glm::translate(glm::mat4(1.0f), glm::vec3(-2.5f, 0.0f, 0.0f));
            quadModel = glm::rotate(quadModel, glm::radians(currentRotation * 0.7f), glm::vec3(0.0f, 1.0f, 0.0f));
            quadModel = glm::scale(quadModel, glm::vec3(0.8f));
            glm::mat4 quadMVP = m_projection * m_view * quadModel;

            agl::Renderer::DrawQuad(quadMVP, glm::vec4(0.8f, 0.2f, 0.8f, 1.0f)); // Purple

            // Draw cube with enhanced animation
            glm::mat4 cubeModel = glm::translate(glm::mat4(1.0f), glm::vec3(2.5f, 0.0f, -1.0f));
            cubeModel = glm::rotate(cubeModel, glm::radians(currentRotation * 0.5f), glm::vec3(1.0f, 1.0f, 0.0f));
            cubeModel = glm::scale(cubeModel, glm::vec3(0.6f));
            glm::mat4 cubeMVP = m_projection * m_view * cubeModel;

            agl::Renderer::DrawCube(cubeMVP, glm::vec4(0.2f, 0.8f, 0.2f, 1.0f)); // Green
        }
    }

    void OnImGuiRender() override {
        ImGui::Begin("AGL Renderer Demo");

        // Performance section
        ImGui::Text("Performance Metrics:");
        ImGui::Text("FPS: %.1f", GetFPS());
        ImGui::Text("Frame Time: %.3f ms", GetDeltaTime() * 1000.0f);
        ImGui::Text("Average Delta: %.3f ms", GetAverageDeltaTime() * 1000.0f);

        ImGui::Separator();

        // Render statistics
        ImGui::Text("Render Statistics:");
        ImGui::Text("Draw Calls: %u", agl::Renderer::GetDrawCallCount());
        ImGui::Text("Vertices: %u", agl::Renderer::GetVertexCount());
        ImGui::Text("Triangles: %u", agl::Renderer::GetTriangleCount());

        if (ImGui::Button("Reset Stats")) {
            agl::Renderer::ResetStats();
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

        if (ImGui::Checkbox("Pause Animation", &m_animationPaused)) {
            if (m_animationPaused) {
                AGL_INFO("Animation paused");
            } else {
                AGL_INFO("Animation resumed");
            }
        }

        ImGui::Separator();

        // Camera controls
        ImGui::Text("Camera Controls:");
        ImGui::SliderFloat("Distance", &m_cameraDistance, 1.0f, 20.0f);
        ImGui::SliderFloat("Yaw", &m_cameraYaw, -180.0f, 180.0f);
        ImGui::SliderFloat("Pitch", &m_cameraPitch, -89.0f, 89.0f);

        if (ImGui::Button("Reset Camera")) {
            m_cameraDistance = 5.0f;
            m_cameraYaw = 0.0f;
            m_cameraPitch = 0.0f;
        }

        ImGui::Text("Mouse Look: %s", m_mouseLookEnabled ? "ON" : "OFF");

        ImGui::Separator();

        // Controls help
        ImGui::Text("Keyboard Controls:");
        ImGui::BulletText("TAB - Toggle wireframe mode");
        ImGui::BulletText("SPACE - Pause/resume animation");
        ImGui::BulletText("W/S - Zoom in/out");
        ImGui::BulletText("A/D - Rotate camera left/right");
        ImGui::BulletText("Q/E - Look up/down");
        ImGui::BulletText("R - Reset camera");

        ImGui::Text("Mouse Controls:");
        ImGui::BulletText("Right Click - Toggle mouse look");
        ImGui::BulletText("Mouse Wheel - Zoom in/out");
        ImGui::BulletText("Mouse Move - Camera look (when enabled)");

        ImGui::Separator();

        // Scene objects
        ImGui::Text("Scene Objects:");
        ImGui::BulletText("Orange Triangle - Custom vertex buffer");
        ImGui::BulletText("Purple Quad - Built-in primitive");
        ImGui::BulletText("Green Cube - Built-in primitive");

        // Debug information
        ImGui::Separator();
        ImGui::Text("Debug Information:");
        ImGui::Text("Rotation: %.1f°", m_rotation);
        ImGui::Text("Window: %dx%d", GetWindow()->GetWidth(), GetWindow()->GetHeight());

        ImGui::End();

        // Reset stats each frame for accurate per-frame statistics
        agl::Renderer::ResetStats();
    }

    void Shutdown() {
        AGL_INFO("Shutting down AGL Renderer Demo...");

        // Clean up renderer resources
        if (m_shader) {
            AGL_INFO("Cleaning up shader resources");
            m_shader.reset();
        }

        if (m_triangleVA) {
            AGL_INFO("Cleaning up vertex array");
            m_triangleVA.reset();
        }

        if (m_triangleVB) {
            AGL_INFO("Cleaning up vertex buffer");
            m_triangleVB.reset();
        }

        // Shutdown renderer
        AGL_INFO("Shutting down renderer");
        agl::Renderer::Shutdown();

        // Call parent shutdown
        agl::Game::Shutdown();

        AGL_INFO("Renderer Demo shutdown complete");
    }

private:
    // Rendering objects
    std::unique_ptr<agl::ShaderProgram> m_shader;
    std::unique_ptr<agl::VertexArray> m_triangleVA;
    std::shared_ptr<agl::VertexBuffer> m_triangleVB;

    // Camera matrices
    glm::mat4 m_projection;
    glm::mat4 m_view;

    // Demo state
    float m_rotation;
    bool m_wireframeMode;
    bool m_animationPaused;

    // Camera control
    float m_cameraDistance;
    float m_cameraYaw;
    float m_cameraPitch;
    double m_lastMouseX;
    double m_lastMouseY;
    bool m_firstMouse;
    bool m_mouseLookEnabled;

    void UpdateMatrices() {
        // Perspective projection
        float aspect = static_cast<float>(GetWindow()->GetWidth()) / static_cast<float>(GetWindow()->GetHeight());
        m_projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

        // Calculate camera position using spherical coordinates
        float x = m_cameraDistance * cos(glm::radians(m_cameraPitch)) * cos(glm::radians(m_cameraYaw));
        float y = m_cameraDistance * sin(glm::radians(m_cameraPitch));
        float z = m_cameraDistance * cos(glm::radians(m_cameraPitch)) * sin(glm::radians(m_cameraYaw));

        glm::vec3 cameraPos(x, y, z);

        // View matrix (camera)
        m_view = glm::lookAt(
            cameraPos,                      // Camera position
            glm::vec3(0.0f, 0.0f, 0.0f),   // Look at origin
            glm::vec3(0.0f, 1.0f, 0.0f)    // Up vector
        );
    }

    void HandleCameraInput(float deltaTime) {
        // Handle mouse look
        if (m_mouseLookEnabled) {
            double mouseX, mouseY;
            GetInput()->GetMousePosition(mouseX, mouseY);

            if (m_firstMouse) {
                m_lastMouseX = mouseX;
                m_lastMouseY = mouseY;
                m_firstMouse = false;
            }

            double deltaX = mouseX - m_lastMouseX;
            double deltaY = m_lastMouseY - mouseY; // Reversed since y-coordinates go from bottom to top

            m_lastMouseX = mouseX;
            m_lastMouseY = mouseY;

            // Camera sensitivity
            float sensitivity = 0.1f;
            deltaX *= sensitivity;
            deltaY *= sensitivity;

            m_cameraYaw += static_cast<float>(deltaX);
            m_cameraPitch += static_cast<float>(deltaY);

            // Constrain pitch to prevent camera flipping
            if (m_cameraPitch > 89.0f) m_cameraPitch = 89.0f;
            if (m_cameraPitch < -89.0f) m_cameraPitch = -89.0f;
        }

        // Handle scroll wheel zoom
        double scrollX, scrollY;
        GetInput()->GetScrollOffset(scrollX, scrollY);
        if (scrollY != 0.0) {
            m_cameraDistance -= static_cast<float>(scrollY) * 0.5f;
            AGL_TRACE("Camera zoom: {:.2f}", m_cameraDistance);
        }

        // Handle keyboard input for camera distance (zoom)
        float zoomSpeed = 5.0f;
        if (GetInput()->IsKeyHeld(GLFW_KEY_W)) {
            m_cameraDistance -= deltaTime * zoomSpeed; // Zoom in
        }
        if (GetInput()->IsKeyHeld(GLFW_KEY_S)) {
            m_cameraDistance += deltaTime * zoomSpeed; // Zoom out
        }

        // Enhanced keyboard controls for camera movement
        if (GetInput()->IsKeyHeld(GLFW_KEY_A)) {
            m_cameraYaw -= deltaTime * 45.0f; // Rotate left
        }
        if (GetInput()->IsKeyHeld(GLFW_KEY_D)) {
            m_cameraYaw += deltaTime * 45.0f; // Rotate right
        }
        if (GetInput()->IsKeyHeld(GLFW_KEY_Q)) {
            m_cameraPitch -= deltaTime * 45.0f; // Look up
        }
        if (GetInput()->IsKeyHeld(GLFW_KEY_E)) {
            m_cameraPitch += deltaTime * 45.0f; // Look down
        }

        // Constrain camera distance
        if (m_cameraDistance < 1.0f) m_cameraDistance = 1.0f;
        if (m_cameraDistance > 20.0f) m_cameraDistance = 20.0f;

        // Constrain pitch
        if (m_cameraPitch > 89.0f) m_cameraPitch = 89.0f;
        if (m_cameraPitch < -89.0f) m_cameraPitch = -89.0f;

        // Reset camera with R key
        if (GetInput()->IsKeyPressed(GLFW_KEY_R)) {
            m_cameraDistance = 5.0f;
            m_cameraYaw = 0.0f;
            m_cameraPitch = 0.0f;
            AGL_INFO("Camera reset to default position");
        }
    }
};

int main_renderer() {
    RendererDemo demo;

    if (demo.Initialize(1280, 720, "AGL Renderer Demo - Modern OpenGL Showcase")) {
        AGL_INFO("Demo initialized successfully! Starting main loop...");
        demo.Run();
        AGL_INFO("Demo completed successfully");
    } else {
        AGL_ERROR("Failed to initialize renderer demo!");
        return -1;
    }

    return 0;
}
