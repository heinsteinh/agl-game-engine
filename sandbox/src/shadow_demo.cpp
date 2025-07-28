#include "ShadowSystem.h"
#include "agl.h"
#include <iostream>
#include <memory>
#include <vector>

using namespace agl;

class ShadowDemoGame : public Game {
private:
    std::shared_ptr<Camera> m_camera;
    std::unique_ptr<CameraController> m_cameraController;

    // Scene objects
    std::unique_ptr<Mesh> m_groundPlane;
    std::vector<std::unique_ptr<Mesh>> m_objects;
    std::vector<glm::mat4> m_objectTransforms;

    // Shadow system
    std::unique_ptr<ShadowSystem> m_shadowSystem;

    // Light control
    Light m_light;
    bool m_lightMoving = true;
    float m_lightTime = 0.0f;

    // Shadow settings
    bool m_shadowsEnabled = true;
    bool m_pcfEnabled = true;
    float m_shadowBias = 0.005f;
    float m_orthographicSize = 20.0f;

    // Demo controls
    bool m_showImGui = true;
    bool m_wireframe = false;

public:
    bool Initialize(int width = 1200, int height = 800, const char *title = "AGL Shadow System Demo") {
        if (!Game::Initialize(width, height, title)) {
            return false;
        }

        std::cout << "[Shadow Demo] Initializing..." << std::endl;

        // Initialize shadow system
        m_shadowSystem = std::make_unique<ShadowSystem>(2048);
        if (!m_shadowSystem->Initialize()) {
            std::cerr << "Failed to initialize shadow system" << std::endl;
            return false;
        }

        // Initialize camera
        m_camera = std::make_shared<Camera>();
        m_camera->SetPosition(glm::vec3(8.0f, 6.0f, 8.0f));
        m_camera->SetPerspective(45.0f, (float)width / height, 0.1f, 100.0f);

        m_cameraController = std::make_unique<CameraController>(m_camera);
        m_cameraController->Initialize(GetInput());
        m_cameraController->SetMode(CameraMode::FirstPerson);

        // Create scene objects
        CreateScene();

        // Setup initial light
        m_light = Light(LightType::Directional, glm::vec3(5.0f, 10.0f, 5.0f), glm::vec3(-0.5f, -1.0f, -0.5f));
        m_shadowSystem->SetLight(m_light);

        std::cout << "[Shadow Demo] Initialization complete" << std::endl;
        return true;
    }

    void CreateScene() {
        // Create ground plane
        m_groundPlane = std::make_unique<Mesh>(Mesh::CreateGroundPlane(20.0f, 20));

        // Create various objects that will cast shadows
        m_objects.clear();
        m_objectTransforms.clear();

        // Cubes
        for (int i = 0; i < 3; ++i) {
            auto cube = std::make_unique<Mesh>(Mesh::CreateCube());

            // Set different materials for variety
            Material material;
            switch (i) {
            case 0:
                material.diffuse = glm::vec3(0.8f, 0.2f, 0.2f); // Red
                break;
            case 1:
                material.diffuse = glm::vec3(0.2f, 0.8f, 0.2f); // Green
                break;
            case 2:
                material.diffuse = glm::vec3(0.2f, 0.2f, 0.8f); // Blue
                break;
            }
            material.specular = glm::vec3(0.8f, 0.8f, 0.8f);
            material.shininess = 64.0f;
            cube->SetMaterial(material);

            m_objects.push_back(std::move(cube));

            // Position cubes
            glm::mat4 transform = glm::mat4(1.0f);
            transform = glm::translate(transform, glm::vec3(-4.0f + i * 4.0f, 1.0f, 0.0f));
            m_objectTransforms.push_back(transform);
        }

        // Spheres
        for (int i = 0; i < 2; ++i) {
            auto sphere = std::make_unique<Mesh>(Mesh::CreateSphere(0.8f, 24, 12));

            Material material;
            if (i == 0) {
                material.diffuse = glm::vec3(0.8f, 0.8f, 0.2f); // Yellow
            } else {
                material.diffuse = glm::vec3(0.8f, 0.2f, 0.8f); // Magenta
            }
            material.specular = glm::vec3(0.9f, 0.9f, 0.9f);
            material.shininess = 128.0f;
            sphere->SetMaterial(material);

            m_objects.push_back(std::move(sphere));

            glm::mat4 transform = glm::mat4(1.0f);
            transform = glm::translate(transform, glm::vec3(-2.0f + i * 4.0f, 0.8f, -3.0f));
            m_objectTransforms.push_back(transform);
        }

        // Add a cylinder
        auto cylinder = std::make_unique<Mesh>(Mesh::CreateCylinder(0.5f, 2.0f, 16, 1));
        Material cylinderMaterial;
        cylinderMaterial.diffuse = glm::vec3(0.6f, 0.4f, 0.2f); // Brown
        cylinderMaterial.specular = glm::vec3(0.5f, 0.5f, 0.5f);
        cylinderMaterial.shininess = 32.0f;
        cylinder->SetMaterial(cylinderMaterial);

        m_objects.push_back(std::move(cylinder));
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(3.0f, 1.0f, 2.0f));
        m_objectTransforms.push_back(transform);

        std::cout << "[Shadow Demo] Created scene with " << m_objects.size() << " objects" << std::endl;
    }

    void OnUpdate(float deltaTime) override {
        // Update camera
        m_cameraController->Update(deltaTime);

        // Animate light if enabled
        if (m_lightMoving) {
            m_lightTime += deltaTime;
            float radius = 8.0f;
            float height = 8.0f + sin(m_lightTime * 0.5f) * 3.0f;

            m_light.position = glm::vec3(cos(m_lightTime * 0.8f) * radius, height, sin(m_lightTime * 0.8f) * radius);

            // Point light towards center
            m_light.direction = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - m_light.position);

            m_shadowSystem->SetLight(m_light);
        }

        // Update shadow system settings
        m_shadowSystem->SetShadowBias(m_shadowBias);
        m_shadowSystem->SetPCFEnabled(m_pcfEnabled);
        m_shadowSystem->SetOrthographicSize(m_orthographicSize);

        // Handle input
        HandleInput();
    }

    void OnRender() override {
        if (m_wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        if (m_shadowsEnabled) {
            RenderWithShadows();
        } else {
            RenderWithoutShadows();
        }
    }

    void OnImGuiRender() override {
        if (m_showImGui) {
            RenderUI();
        }
    }

    void RenderWithShadows() {
        // 1. Render shadow map
        m_shadowSystem->BeginShadowPass();

        // Render ground plane to shadow map
        glm::mat4 groundTransform = glm::mat4(1.0f);
        m_shadowSystem->RenderShadowCaster(*m_groundPlane, groundTransform);

        // Render all objects to shadow map
        for (size_t i = 0; i < m_objects.size(); ++i) {
            m_shadowSystem->RenderShadowCaster(*m_objects[i], m_objectTransforms[i]);
        }

        m_shadowSystem->EndShadowPass();

        // 2. Render main scene with shadows
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_shadowSystem->BeginMainPass(m_camera->GetViewMatrix(), m_camera->GetProjectionMatrix());

        // Render ground plane with shadows
        m_shadowSystem->RenderWithShadows(*m_groundPlane, groundTransform);

        // Render all objects with shadows
        for (size_t i = 0; i < m_objects.size(); ++i) {
            m_shadowSystem->RenderWithShadows(*m_objects[i], m_objectTransforms[i]);
        }
    }

    void RenderWithoutShadows() {
        // Simple forward rendering without shadows
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use a simple shader (you'd need to create this)
        // For now, just render wireframe or basic shapes
        glm::mat4 groundTransform = glm::mat4(1.0f);

        for (size_t i = 0; i < m_objects.size(); ++i) {
            // Basic rendering without shadows - this would need a different shader
            // For demonstration purposes, we'll just skip this
        }
    }

    void RenderUI() {
        ImGui::Begin("Shadow Demo Controls");

        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Camera Pos: (%.1f, %.1f, %.1f)", m_camera->GetPosition().x, m_camera->GetPosition().y,
                    m_camera->GetPosition().z);

        ImGui::Separator();
        ImGui::Text("Shadow Settings");

        ImGui::Checkbox("Enable Shadows", &m_shadowsEnabled);
        ImGui::Checkbox("PCF (Softer Shadows)", &m_pcfEnabled);
        ImGui::SliderFloat("Shadow Bias", &m_shadowBias, 0.001f, 0.01f, "%.4f");
        ImGui::SliderFloat("Orthographic Size", &m_orthographicSize, 5.0f, 50.0f);

        ImGui::Separator();
        ImGui::Text("Light Settings");

        ImGui::Checkbox("Animate Light", &m_lightMoving);

        if (!m_lightMoving) {
            if (ImGui::SliderFloat3("Light Position", &m_light.position.x, -15.0f, 15.0f)) {
                m_shadowSystem->SetLight(m_light);
            }
            if (ImGui::SliderFloat3("Light Direction", &m_light.direction.x, -1.0f, 1.0f)) {
                m_light.direction = glm::normalize(m_light.direction);
                m_shadowSystem->SetLight(m_light);
            }
        }

        ImGui::ColorEdit3("Light Color", &m_light.color.x);
        if (ImGui::SliderFloat("Light Intensity", &m_light.intensity, 0.1f, 3.0f)) {
            m_shadowSystem->SetLight(m_light);
        }

        ImGui::Separator();
        ImGui::Text("Rendering");

        ImGui::Checkbox("Wireframe", &m_wireframe);

        if (ImGui::Button("Reset Camera")) {
            m_camera->SetPosition(glm::vec3(8.0f, 6.0f, 8.0f));
        }

        if (ImGui::Button("Recreate Scene")) {
            CreateScene();
        }

        ImGui::End();
    }

    void HandleInput() {
        Input *input = GetInput();

        // Toggle UI
        if (input->IsKeyPressed(GLFW_KEY_F1)) {
            m_showImGui = !m_showImGui;
        }

        // Toggle wireframe
        if (input->IsKeyPressed(GLFW_KEY_F2)) {
            m_wireframe = !m_wireframe;
        }

        // Toggle shadows
        if (input->IsKeyPressed(GLFW_KEY_F3)) {
            m_shadowsEnabled = !m_shadowsEnabled;
        }

        // Toggle light animation
        if (input->IsKeyPressed(GLFW_KEY_F4)) {
            m_lightMoving = !m_lightMoving;
        }
    }
};

int main() {
    ShadowDemoGame game;

    if (!game.Initialize()) {
        std::cerr << "Failed to initialize shadow demo" << std::endl;
        return -1;
    }

    std::cout << "\n=== Shadow Demo Controls ===" << std::endl;
    std::cout << "WASD + Mouse: Camera movement" << std::endl;
    std::cout << "F1: Toggle UI" << std::endl;
    std::cout << "F2: Toggle wireframe" << std::endl;
    std::cout << "F3: Toggle shadows" << std::endl;
    std::cout << "F4: Toggle light animation" << std::endl;
    std::cout << "ESC: Quit" << std::endl;
    std::cout << "===========================\n" << std::endl;

    game.Run();

    return 0;
}
