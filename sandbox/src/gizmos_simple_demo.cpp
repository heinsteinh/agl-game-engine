#include "agl.h"
#include <cmath>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class SimpleGizmosDemo : public agl::Game {
private:
    float m_time = 0.0f;
    bool m_showAxes = true;
    bool m_showSphere = true;
    bool m_showCylinder = true;
    bool m_showAABB = true;
    bool m_showTriangles = true;
    bool m_animateObjects = true;

    // Camera controls
    glm::vec3 m_cameraPos = glm::vec3(5, 5, 5);
    glm::vec3 m_cameraTarget = glm::vec3(0, 0, 0);
    glm::vec3 m_cameraUp = glm::vec3(0, 1, 0);
    float m_cameraDistance = 8.0f;
    float m_cameraYaw = 45.0f;   // Horizontal rotation
    float m_cameraPitch = 30.0f; // Vertical rotation
    float m_cameraSpeed = 5.0f;
    float m_mouseSensitivity = 0.1f;
    bool m_firstMouse = true;
    bool m_mouseControlEnabled = false;
    double m_lastMouseX = 400, m_lastMouseY = 300;

public:
    bool Initialize(int width = 800, int height = 600, const char *title = "Gizmos Test Demo") {
        if (!agl::Game::Initialize(width, height, title)) {
            return false;
        }

        std::cout << "SimpleGizmosDemo: Initializing Gizmos..." << std::endl;

        // Initialize Gizmos with reasonable buffer sizes
        agl::Gizmos::create(1000, 1000, 1000, 1000);

        std::cout << "Gizmos initialized successfully" << std::endl;
        return true;
    }

private:
    void HandleCameraInput(float deltaTime) {
        auto *input = GetInput();

        // Keyboard movement
        if (input->IsKeyPressed(GLFW_KEY_W)) {
            m_cameraDistance -= m_cameraSpeed * deltaTime;
        }
        if (input->IsKeyPressed(GLFW_KEY_S)) {
            m_cameraDistance += m_cameraSpeed * deltaTime;
        }
        if (input->IsKeyPressed(GLFW_KEY_A)) {
            m_cameraYaw -= 90.0f * deltaTime;
        }
        if (input->IsKeyPressed(GLFW_KEY_D)) {
            m_cameraYaw += 90.0f * deltaTime;
        }
        if (input->IsKeyPressed(GLFW_KEY_Q)) {
            m_cameraPitch -= 90.0f * deltaTime;
        }
        if (input->IsKeyPressed(GLFW_KEY_E)) {
            m_cameraPitch += 90.0f * deltaTime;
        }

        // Toggle mouse control with right mouse button
        if (input->IsMouseButtonPressed(agl::MouseButton::Right)) {
            m_mouseControlEnabled = true;
            m_firstMouse = true;
        } else {
            m_mouseControlEnabled = false;
        }

        // Mouse look
        if (m_mouseControlEnabled) {
            double mouseX, mouseY;
            input->GetMousePosition(mouseX, mouseY);

            if (m_firstMouse) {
                m_lastMouseX = mouseX;
                m_lastMouseY = mouseY;
                m_firstMouse = false;
            }

            double xOffset = mouseX - m_lastMouseX;
            double yOffset = m_lastMouseY - mouseY; // Reversed since y-coordinates go from bottom to top

            m_lastMouseX = mouseX;
            m_lastMouseY = mouseY;

            xOffset *= m_mouseSensitivity;
            yOffset *= m_mouseSensitivity;

            m_cameraYaw += xOffset;
            m_cameraPitch += yOffset;
        }

        // Constrain pitch
        if (m_cameraPitch > 89.0f)
            m_cameraPitch = 89.0f;
        if (m_cameraPitch < -89.0f)
            m_cameraPitch = -89.0f;

        // Constrain distance
        if (m_cameraDistance < 1.0f)
            m_cameraDistance = 1.0f;
        if (m_cameraDistance > 50.0f)
            m_cameraDistance = 50.0f;
    }

    void UpdateCamera() {
        // Convert spherical coordinates to cartesian
        float yawRad = glm::radians(m_cameraYaw);
        float pitchRad = glm::radians(m_cameraPitch);

        m_cameraPos.x = m_cameraTarget.x + m_cameraDistance * cos(pitchRad) * cos(yawRad);
        m_cameraPos.y = m_cameraTarget.y + m_cameraDistance * sin(pitchRad);
        m_cameraPos.z = m_cameraTarget.z + m_cameraDistance * cos(pitchRad) * sin(yawRad);
    }

public:
    void OnUpdate(float deltaTime) override {
        m_time += deltaTime;

        // Handle camera input
        HandleCameraInput(deltaTime);

        // Update camera position based on spherical coordinates
        UpdateCamera();

        // Clear previous gizmos each frame
        agl::Gizmos::clear();

        // 1. Coordinate axes
        if (m_showAxes) {
            agl::Gizmos::addLine(glm::vec3(0, 0, 0), glm::vec3(2, 0, 0), glm::vec4(1, 0, 0, 1)); // Red X-axis
            agl::Gizmos::addLine(glm::vec3(0, 0, 0), glm::vec3(0, 2, 0), glm::vec4(0, 1, 0, 1)); // Green Y-axis
            agl::Gizmos::addLine(glm::vec3(0, 0, 0), glm::vec3(0, 0, 2), glm::vec4(0, 0, 1, 1)); // Blue Z-axis
        }

        // 2. Animated sphere
        if (m_showSphere) {
            glm::vec3 spherePos = glm::vec3(m_animateObjects ? 2.0f * sin(m_time) : 2.0f, 1.0f,
                                            m_animateObjects ? 2.0f * cos(m_time) : 0.0f);
            agl::Gizmos::addSphere(spherePos, 0.5f, 8, 8, glm::vec4(1, 1, 0, 1)); // Yellow sphere
        }

        // 3. Cylinder (filled)
        if (m_showCylinder) {
            glm::vec3 cylinderCenter = glm::vec3(-2, m_animateObjects ? 1.0f + sin(m_time) : 1.0f, 0);
            float cylinderHeight = m_animateObjects ? 2.0f + 0.5f * sin(m_time) : 2.0f;
            agl::Gizmos::addCylinderFilled(cylinderCenter, 0.3f, cylinderHeight * 0.5f, 8,
                                           glm::vec4(0, 1, 1, 1)); // Cyan cylinder
        }

        // 4. AABB (Axis-Aligned Bounding Box)
        if (m_showAABB) {
            glm::vec3 aabbCenter = glm::vec3(0, 0, 2.5f);
            glm::vec3 aabbExtents = glm::vec3(1, 1, 0.5f);
            if (m_animateObjects) {
                float scale = 1.0f + 0.3f * sin(m_time * 2.0f);
                aabbExtents *= scale;
            }
            agl::Gizmos::addAABB(aabbCenter, aabbExtents, glm::vec4(1, 0, 1, 1)); // Magenta AABB wireframe

            // Also add a filled AABB next to it
            glm::vec3 filledCenter = aabbCenter + glm::vec3(3, 0, 0);
            agl::Gizmos::addAABBFilled(filledCenter, aabbExtents * 0.8f,
                                       glm::vec4(1, 0, 1, 0.3f)); // Semi-transparent filled
        }

        // 5. Triangle fan
        if (m_showTriangles) {
            glm::vec3 center = glm::vec3(0, 3, 0);
            int numSegments = 6;
            float radius = 1.0f;

            for (int i = 0; i < numSegments; i++) {
                float angle1 = (float)i / numSegments * 2.0f * M_PI;
                float angle2 = (float)(i + 1) / numSegments * 2.0f * M_PI;

                if (m_animateObjects) {
                    angle1 += m_time;
                    angle2 += m_time;
                }

                glm::vec3 p1 = center + glm::vec3(radius * cos(angle1), 0, radius * sin(angle1));
                glm::vec3 p2 = center + glm::vec3(radius * cos(angle2), 0, radius * sin(angle2));

                // Alternating colors for each triangle
                glm::vec4 color = (i % 2 == 0) ? glm::vec4(0.8f, 0.2f, 0.8f, 0.7f) : glm::vec4(0.2f, 0.8f, 0.8f, 0.7f);
                agl::Gizmos::addTri(center, p1, p2, color);
            }
        }

        // 6. Grid lines
        for (int i = -5; i <= 5; i++) {
            if (i != 0) { // Skip center lines (already drawn as axes)
                // X direction lines
                agl::Gizmos::addLine(glm::vec3(i, 0, -5), glm::vec3(i, 0, 5), glm::vec4(0.3f, 0.3f, 0.3f, 1));
                // Z direction lines
                agl::Gizmos::addLine(glm::vec3(-5, 0, i), glm::vec3(5, 0, i), glm::vec4(0.3f, 0.3f, 0.3f, 1));
            }
        }

        // 7. Animated spiral
        if (m_animateObjects) {
            for (int i = 0; i < 50; i++) {
                float t = (float)i / 50.0f;
                float spiralTime = m_time + t * 4.0f;
                glm::vec3 pos1 = glm::vec3(0.5f * spiralTime * cos(spiralTime * 3.0f), t * 3.0f - 1.0f,
                                           0.5f * spiralTime * sin(spiralTime * 3.0f));

                if (i > 0) {
                    float prevT = (float)(i - 1) / 50.0f;
                    float prevSpiralTime = m_time + prevT * 4.0f;
                    glm::vec3 pos0 = glm::vec3(0.5f * prevSpiralTime * cos(prevSpiralTime * 3.0f), prevT * 3.0f - 1.0f,
                                               0.5f * prevSpiralTime * sin(prevSpiralTime * 3.0f));

                    glm::vec4 color = glm::vec4(sin(spiralTime), cos(spiralTime), sin(spiralTime * 2), 1);
                    agl::Gizmos::addLine(pos0, pos1, color);
                }
            }
        }

        // 8. Additional geometric shapes
        if (m_showSphere) {
            // Add a capsule
            glm::vec3 capsuleCenter = glm::vec3(4, 2, 0);
            agl::Gizmos::addCapsule(capsuleCenter, 2.0f, 0.4f, 8, 8, glm::vec4(0.8f, 0.4f, 0.2f, 1));

            // Add some rings
            glm::vec3 ringCenter = glm::vec3(-4, 2, 2);
            agl::Gizmos::addRing(ringCenter, 0.5f, 1.0f, 16, glm::vec4(1, 0.5f, 0, 1));
            agl::Gizmos::addDisk(ringCenter + glm::vec3(0, 1, 0), 0.8f, 12, glm::vec4(0.5f, 1, 0.5f, 0.5f));
        }

        // 9. Hermite spline curve
        if (m_animateObjects) {
            glm::vec3 start = glm::vec3(-3, 0, -3);
            glm::vec3 end = glm::vec3(3, 0, -3);
            glm::vec3 tangentStart = glm::vec3(2, 2 + sin(m_time), 0);
            glm::vec3 tangentEnd = glm::vec3(-2, 2 + cos(m_time), 0);
            agl::Gizmos::addHermiteSpline(start, end, tangentStart, tangentEnd, 20, glm::vec4(1, 1, 0, 1));
        }
    }

    void OnRender() override {
        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        // Set up view and projection matrices using camera
        glm::mat4 view = glm::lookAt(m_cameraPos, m_cameraTarget, m_cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        // Draw gizmos
        agl::Gizmos::draw(projection, view);
    }

    void OnImGuiRender() override {
        ImGui::Begin("Gizmos Test Demo Controls");

        ImGui::Text("Performance: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::Separator();

        // Camera Controls
        ImGui::Text("Camera Controls:");
        ImGui::Text("Position: (%.1f, %.1f, %.1f)", m_cameraPos.x, m_cameraPos.y, m_cameraPos.z);
        ImGui::SliderFloat("Distance", &m_cameraDistance, 1.0f, 50.0f);
        ImGui::SliderFloat("Yaw", &m_cameraYaw, -180.0f, 180.0f);
        ImGui::SliderFloat("Pitch", &m_cameraPitch, -89.0f, 89.0f);
        ImGui::SliderFloat("Speed", &m_cameraSpeed, 1.0f, 20.0f);
        ImGui::SliderFloat("Mouse Sensitivity", &m_mouseSensitivity, 0.01f, 1.0f);

        if (ImGui::Button("Reset Camera")) {
            m_cameraDistance = 8.0f;
            m_cameraYaw = 45.0f;
            m_cameraPitch = 30.0f;
            m_cameraTarget = glm::vec3(0, 0, 0);
        }

        ImGui::Text("Controls:");
        ImGui::BulletText("W/S: Move closer/farther");
        ImGui::BulletText("A/D: Rotate left/right");
        ImGui::BulletText("Q/E: Look up/down");
        ImGui::BulletText("Hold Right Mouse: Mouse look");

        ImGui::Separator();

        ImGui::Text("Geometry Controls:");
        ImGui::Checkbox("Show Coordinate Axes", &m_showAxes);
        ImGui::Checkbox("Show Sphere & Shapes", &m_showSphere);
        ImGui::Checkbox("Show Cylinder", &m_showCylinder);
        ImGui::Checkbox("Show AABB Boxes", &m_showAABB);
        ImGui::Checkbox("Show Triangles", &m_showTriangles);
        ImGui::Checkbox("Animate Objects", &m_animateObjects);

        ImGui::Separator();
        ImGui::Text("Scene Info:");
        ImGui::Text("Time: %.2f seconds", m_time);

        if (ImGui::Button("Reset Animation")) {
            m_time = 0.0f;
        }

        ImGui::Separator();
        ImGui::Text("Shapes in Demo:");
        ImGui::BulletText("RGB Coordinate axes");
        ImGui::BulletText("Animated sphere (yellow)");
        ImGui::BulletText("Filled cylinder (cyan)");
        ImGui::BulletText("AABB wireframe & filled (magenta)");
        ImGui::BulletText("Triangle fan (rotating)");
        ImGui::BulletText("Grid lines");
        ImGui::BulletText("Animated spiral");
        ImGui::BulletText("Capsule (orange)");
        ImGui::BulletText("Ring & disk (green/orange)");
        ImGui::BulletText("Hermite spline curve (yellow)");

        ImGui::End();
    }

    void OnShutdown() {
        std::cout << "Shutting down Gizmos..." << std::endl;
        agl::Gizmos::destroy();
    }
};

int main() {
    std::cout << "Starting SimpleGizmosDemo..." << std::endl;

    SimpleGizmosDemo demo;

    if (!demo.Initialize()) {
        std::cerr << "Failed to initialize demo" << std::endl;
        return -1;
    }

    demo.Run();

    std::cout << "SimpleGizmosDemo finished" << std::endl;
    return 0;
}
