#include "agl.h"
#include <imgui.h>
#include <iostream>
#include <memory>

using namespace agl;

class GizmosDemo : public Game {
private:
    std::shared_ptr<Camera> m_camera;
    std::unique_ptr<CameraController> m_cameraController;

public:
    void OnStart() {
        std::cout << "[Gizmos Demo] Initializing..." << std::endl;

        // Initialize Gizmos
        Gizmos::create(1000, 1000, 100, 100);

        // Create camera
        m_camera = std::make_shared<Camera>(glm::vec3(5.0f, 5.0f, 5.0f));
        m_camera->SetPerspective(75.0f, 1.5f, 0.1f, 1000.0f);

        // Create camera controller
        m_cameraController = std::make_unique<CameraController>(m_camera);
        m_cameraController->Initialize(GetInput());

        std::cout << "[Gizmos Demo] Initialization complete" << std::endl;
    }

    void OnShutdown() {
        m_cameraController.reset();
        m_camera.reset();
        Gizmos::destroy();
        std::cout << "[Gizmos Demo] Shutdown complete" << std::endl;
    }

    void OnUpdate(float deltaTime) override {
        m_cameraController->Update(deltaTime);

        // Clear previous gizmos
        Gizmos::clear();

        // Draw coordinate system at origin
        glm::mat4 origin = glm::mat4(1.0f);
        Gizmos::addTransform(origin, 2.0f);

        // Draw some basic shapes
        Gizmos::addAABB(glm::vec3(0, 1, 0), glm::vec3(0.5f), glm::vec4(1, 0, 0, 1));
        Gizmos::addAABBFilled(glm::vec3(2, 1, 0), glm::vec3(0.5f), glm::vec4(0, 1, 0, 0.5f));

        // Draw some lines
        Gizmos::addLine(glm::vec3(-2, 0, 0), glm::vec3(-2, 3, 0), glm::vec4(1, 1, 0, 1));
        Gizmos::addLine(glm::vec3(0, 0, -2), glm::vec3(0, 3, -2), glm::vec4(0, 1, 1, 1));

        // Draw a sphere
        Gizmos::addSphere(glm::vec3(0, 2, 2), 0.8f, 8, 8, glm::vec4(1, 0, 1, 0.8f));
    }

    void OnRender() override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Render gizmos
        glm::mat4 projectionView = m_camera->GetProjectionMatrix() * m_camera->GetViewMatrix();
        Gizmos::draw(projectionView);
    }

    void OnImGui() {
        if (ImGui::Begin("Gizmos Demo Controls")) {
            ImGui::Text("WASD + Mouse: Camera movement");
            ImGui::Text("ESC: Exit");

            glm::vec3 pos = m_camera->GetPosition();
            ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
        }
        ImGui::End();
    }
};

int main() {
    GizmosDemo demo;
    demo.Run();
    return 0;
}
