#include "agl.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

class SignalSlotDemoGame : public agl::Game {
private:
    std::shared_ptr<agl::Camera> m_camera;
    std::unique_ptr<agl::CameraController> m_controller;

    // Slots for handling input events
    std::unique_ptr<agl::Slot<int>> m_escapeKeySlot;
    std::unique_ptr<agl::Slot<agl::MouseButton>> m_mouseClickSlot;
    std::unique_ptr<agl::Slot<double, double>> m_mouseMoveSlot;
    std::unique_ptr<agl::Slot<double, double>> m_scrollSlot;
    std::unique_ptr<agl::Slot<int, int>> m_windowResizeSlot;

    // Stats
    int m_keyPressCount = 0;
    int m_mouseClickCount = 0;
    int m_mouseMoveCount = 0;
    int m_scrollCount = 0;
    bool m_showDemo = true;

public:
    bool Initialize(int width = 1024, int height = 768, const char *title = "SigSlot & DispatchQueue Demo") {
        if (!agl::Game::Initialize(width, height, title)) {
            return false;
        }

        // Create camera and controller
        m_camera = std::make_shared<agl::Camera>();
        m_camera->SetPosition(glm::vec3(0.0f, 0.0f, 3.0f));
        m_camera->SetPerspective(45.0f, (float)width / height, 0.1f, 100.0f);

        m_controller = std::make_unique<agl::CameraController>(m_camera);
        m_controller->Initialize(GetInput());
        m_controller->SetMode(agl::CameraMode::FirstPerson);

        // Set up Signal/Slot connections
        SetupInputSlots();
        SetupWindowSlots();

        AGL_INFO("SignalSlot demo initialized with Signal/Slot system");
        return true;
    }

private:
    void SetupInputSlots() {
        auto *input = GetInput();

        // Escape key handler
        m_escapeKeySlot = std::make_unique<agl::Slot<int>>([this](int key) {
            if (key == GLFW_KEY_ESCAPE) {
                AGL_INFO("Escape key pressed - requesting shutdown via DispatchQueue");
                // Use DispatchQueue to defer the shutdown to avoid immediate exit during callback
                RunOnMainThread([this]() { GetWindow()->SetShouldClose(true); });
            }
            m_keyPressCount++;
        });
        input->OnKeyPressed.bind(*m_escapeKeySlot);

        // Mouse click handler
        m_mouseClickSlot = std::make_unique<agl::Slot<agl::MouseButton>>([this](agl::MouseButton button) {
            const char *buttonName = "Unknown";
            switch (button) {
            case agl::MouseButton::Left:
                buttonName = "Left";
                break;
            case agl::MouseButton::Right:
                buttonName = "Right";
                break;
            case agl::MouseButton::Middle:
                buttonName = "Middle";
                break;
            }

            AGL_INFO("Mouse button clicked: {}", buttonName);
            m_mouseClickCount++;
        });
        input->OnMouseButtonPressed.bind(*m_mouseClickSlot);

        // Mouse movement handler (throttled)
        m_mouseMoveSlot = std::make_unique<agl::Slot<double, double>>([this](double x, double y) {
            static int moveCounter = 0;
            moveCounter++;

            // Only log every 60th movement to avoid spam
            if (moveCounter % 60 == 0) {
                AGL_TRACE("Mouse moved to ({:.1f}, {:.1f})", x, y);
            }
            m_mouseMoveCount++;
        });
        input->OnMouseMoveEvent.bind(*m_mouseMoveSlot);

        // Scroll handler
        m_scrollSlot = std::make_unique<agl::Slot<double, double>>([this](double xoffset, double yoffset) {
            AGL_INFO("Scroll event: ({:.1f}, {:.1f})", xoffset, yoffset);
            m_scrollCount++;
        });
        input->OnScrollEvent.bind(*m_scrollSlot);
    }

    void SetupWindowSlots() {
        auto *window = GetWindow();

        // Window resize handler
        m_windowResizeSlot = std::make_unique<agl::Slot<int, int>>([this](int width, int height) {
            AGL_INFO("Window resized to {}x{}", width, height);
            if (m_camera) {
                m_camera->SetPerspective(45.0f, (float)width / height, 0.1f, 100.0f);
            }

            // Use DispatchQueue to defer UI updates
            RunOnMainThread([width, height]() {
                // Update any UI elements that depend on window size
                AGL_TRACE("UI updated for new window size: {}x{}", width, height);
            });
        });
        window->OnWindowResize.bind(*m_windowResizeSlot);
    }

public:
    void OnUpdate(float deltaTime) override {
        if (m_controller) {
            m_controller->Update(deltaTime);
        }
    }

    void OnRender() override {
        // Basic rendering setup
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Demo: render a simple colored triangle that responds to camera
        if (m_camera) {
            // This would normally be your 3D rendering code
            // For demo purposes, we'll just show the signals working
        }
    }

    void OnImGuiRender() override {
        if (m_showDemo) {
            ImGui::Begin("Signal/Slot & DispatchQueue Demo", &m_showDemo);

            ImGui::Text("AGL Game Engine - Event System Demo");
            ImGui::Separator();

            ImGui::Text("Event Statistics:");
            ImGui::Text("Key Presses: %d", m_keyPressCount);
            ImGui::Text("Mouse Clicks: %d", m_mouseClickCount);
            ImGui::Text("Mouse Moves: %d", m_mouseMoveCount);
            ImGui::Text("Scroll Events: %d", m_scrollCount);

            ImGui::Separator();
            ImGui::Text("Instructions:");
            ImGui::BulletText("Move mouse around to trigger mouse move events");
            ImGui::BulletText("Click mouse buttons to trigger click events");
            ImGui::BulletText("Use scroll wheel to trigger scroll events");
            ImGui::BulletText("Press any key to increment key counter");
            ImGui::BulletText("Press ESC to exit (via DispatchQueue)");
            ImGui::BulletText("Resize window to trigger resize events");

            ImGui::Separator();
            ImGui::Text("Camera Controls:");
            ImGui::BulletText("WASD - Move camera");
            ImGui::BulletText("Mouse - Look around");
            ImGui::BulletText("Scroll - Zoom");
            ImGui::BulletText("Right Click - Aim mode");
            ImGui::BulletText("Shift - Sprint");

            ImGui::Separator();
            if (ImGui::Button("Test DispatchQueue")) {
                // Demonstrate async task execution
                RunOnMainThread([this]() {
                    AGL_INFO("Async task executed on main thread via DispatchQueue!");
                    m_keyPressCount += 10; // Visual feedback
                });
            }

            ImGui::SameLine();
            if (ImGui::Button("Test Multiple Tasks")) {
                // Queue multiple tasks
                for (int i = 0; i < 5; ++i) {
                    RunOnMainThread([i]() { AGL_INFO("Task {} executed", i + 1); });
                }
            }

            // Camera info
            if (m_camera) {
                ImGui::Separator();
                ImGui::Text("Camera Information:");
                auto pos = m_camera->GetPosition();
                ImGui::Text("Position: (%.1f, %.1f, %.1f)", pos.x, pos.y, pos.z);
                ImGui::Text("FOV: %.1f°", m_camera->GetZoom());
                ImGui::Text("Yaw: %.1f°", m_camera->GetYaw());
                ImGui::Text("Pitch: %.1f°", m_camera->GetPitch());
            }

            ImGui::End();
        }

        // Show timing info
        ImGui::Begin("Performance");
        ImGui::Text("FPS: %.1f", GetFPS());
        ImGui::Text("Frame Time: %.3f ms", GetDeltaTime() * 1000.0f);
        ImGui::Text("Average Frame Time: %.3f ms", GetAverageDeltaTime() * 1000.0f);
        ImGui::End();
    }
};

int main() {
    SignalSlotDemoGame game;

    if (game.Initialize(1024, 768, "AGL Engine - Signal/Slot & DispatchQueue Demo")) {
        game.Run();
    }

    game.Shutdown();
    return 0;
}
