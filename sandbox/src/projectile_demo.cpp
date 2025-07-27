#include "agl.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

class ProjectileDemoGame : public agl::Game {
private:
    std::shared_ptr<agl::Camera> m_camera;
    std::unique_ptr<agl::CameraController> m_controller;

    // Shader for 3D rendering
    std::unique_ptr<agl::ShaderProgram> m_meshShader;

    // Projectile system
    std::unique_ptr<agl::ProjectileSystem> m_projectileSystem;
    std::unique_ptr<agl::Shooter> m_shooter;

    // Demo parameters
    float m_autoFireTimer = 0.0f;
    float m_autoFireInterval = 0.5f; // Fire every 0.5 seconds
    bool m_autoFire = false;

    // Current projectile type for firing
    int m_currentProjectileIndex = 0;
    const char *m_projectileNames[5] = {"Bullet", "Missile", "Laser", "Plasma", "Default"};
    agl::ProjectileType m_projectileTypes[5] = {agl::ProjectileType::Bullet, agl::ProjectileType::Missile,
                                                agl::ProjectileType::Laser, agl::ProjectileType::Plasma,
                                                agl::ProjectileType::Default};

    // Shooter position and direction
    glm::vec3 m_shooterPosition{0.0f, 0.0f, 0.0f};
    glm::vec3 m_shooterDirection{0.0f, 0.0f, -1.0f};

public:
    bool Initialize(int width = 1200, int height = 800, const char *title = "AGL Projectile System Demo") {
        if (!agl::Game::Initialize(width, height, title)) {
            return false;
        }

        std::cout << "Initializing Projectile Demo..." << std::endl;

        // Setup camera
        m_camera = std::make_shared<agl::Camera>();
        m_camera->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
        m_camera->SetPerspective(45.0f, (float)width / height, 0.1f, 100.0f);

        m_controller = std::make_unique<agl::CameraController>(m_camera);
        m_controller->Initialize(GetInput());
        m_controller->SetMode(agl::CameraMode::FirstPerson);

        // Create shader
        CreateShader();

        // Initialize projectile system
        m_projectileSystem = std::make_unique<agl::ProjectileSystem>();
        m_projectileSystem->Initialize(1000); // Support up to 1000 projectiles

        // Create shooter
        m_shooter = std::make_unique<agl::Shooter>(m_projectileSystem.get());
        m_shooter->fireRate = 5.0f; // 5 shots per second
        m_shooter->projectileSpeed = 15.0f;
        m_shooter->projectileLifetime = 10.0f;

        std::cout << "Projectile Demo initialized successfully!" << std::endl;
        return true;
    }

    void OnUpdate(float deltaTime) override {
        // Update controller
        m_controller->Update(deltaTime);

        // Update projectile system
        m_projectileSystem->Update(deltaTime);
        m_shooter->Update(deltaTime);

        // Handle input
        HandleInput(deltaTime);

        // Auto fire
        if (m_autoFire) {
            m_autoFireTimer += deltaTime;
            if (m_autoFireTimer >= m_autoFireInterval) {
                FireProjectile();
                m_autoFireTimer = 0.0f;
            }
        }
    }

    void OnRender() override {
        // Get camera matrices
        glm::mat4 view = m_camera->GetViewMatrix();
        glm::mat4 projection = m_camera->GetProjectionMatrix();

        // Clear screen
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Enable depth testing
        glEnable(GL_DEPTH_TEST);

        // Render projectiles
        m_projectileSystem->Render(*m_meshShader, view, projection);

        // Render a simple target grid
        RenderTargetGrid(view, projection);
    }

    void OnImGuiRender() override {
        ImGui::Begin("Projectile System Demo");

        // Projectile system stats
        ImGui::Text("Active Projectiles: %zu", m_projectileSystem->GetActiveCount());
        ImGui::Text("Max Projectiles: %zu", m_projectileSystem->GetMaxProjectiles());

        ImGui::Separator();

        // Projectile type selection
        ImGui::Text("Projectile Type:");
        if (ImGui::Combo("Type", &m_currentProjectileIndex, m_projectileNames, 5)) {
            // Type changed
        }

        ImGui::Separator();

        // Shooter controls
        ImGui::Text("Shooter Controls:");
        ImGui::SliderFloat("Fire Rate", &m_shooter->fireRate, 0.1f, 20.0f, "%.1f shots/sec");
        ImGui::SliderFloat("Projectile Speed", &m_shooter->projectileSpeed, 1.0f, 50.0f, "%.1f units/sec");
        ImGui::SliderFloat("Projectile Lifetime", &m_shooter->projectileLifetime, 1.0f, 30.0f, "%.1f sec");

        ImGui::Separator();

        // Fire controls
        if (ImGui::Button("Fire Projectile") && m_shooter->CanFire()) {
            FireProjectile();
        }

        ImGui::SameLine();
        ImGui::Checkbox("Auto Fire", &m_autoFire);

        if (m_autoFire) {
            ImGui::SliderFloat("Auto Fire Interval", &m_autoFireInterval, 0.1f, 5.0f, "%.1f sec");
        }

        ImGui::Separator();

        // Shooter position controls
        ImGui::Text("Shooter Position:");
        ImGui::SliderFloat3("Position", &m_shooterPosition.x, -10.0f, 10.0f);

        ImGui::Text("Shooter Direction:");
        ImGui::SliderFloat3("Direction", &m_shooterDirection.x, -1.0f, 1.0f);

        // Normalize direction
        if (glm::length(m_shooterDirection) > 0.0f) {
            m_shooterDirection = glm::normalize(m_shooterDirection);
        }

        ImGui::Separator();

        // Clear button
        if (ImGui::Button("Clear All Projectiles")) {
            m_projectileSystem->ClearAll();
        }

        // Instructions
        ImGui::Separator();
        ImGui::Text("Instructions:");
        ImGui::BulletText("Use WASD to move camera");
        ImGui::BulletText("Mouse to look around");
        ImGui::BulletText("Space to fire projectiles");
        ImGui::BulletText("Adjust settings and watch projectiles fly!");

        ImGui::End();
    }

private:
    void CreateShader() {
        const char *vertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aNormal;
            layout (location = 2) in vec2 aTexCoord;

            out vec3 FragPos;
            out vec3 Normal;
            out vec2 TexCoord;

            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;

            void main() {
                FragPos = vec3(model * vec4(aPos, 1.0));
                Normal = mat3(transpose(inverse(model))) * aNormal;
                TexCoord = aTexCoord;

                gl_Position = projection * view * vec4(FragPos, 1.0);
            }
        )";

        const char *fragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;

            in vec3 FragPos;
            in vec3 Normal;
            in vec2 TexCoord;

            uniform vec3 lightPos;
            uniform vec3 lightColor;
            uniform vec3 viewPos;

            // Material properties
            uniform vec3 material_ambient;
            uniform vec3 material_diffuse;
            uniform vec3 material_specular;
            uniform float material_shininess;

            void main() {
                // Ambient
                vec3 ambient = lightColor * material_ambient;

                // Diffuse
                vec3 norm = normalize(Normal);
                vec3 lightDir = normalize(lightPos - FragPos);
                float diff = max(dot(norm, lightDir), 0.0);
                vec3 diffuse = lightColor * (diff * material_diffuse);

                // Specular
                vec3 viewDir = normalize(viewPos - FragPos);
                vec3 reflectDir = reflect(-lightDir, norm);
                float spec = pow(max(dot(viewDir, reflectDir), 0.0), material_shininess);
                vec3 specular = lightColor * (spec * material_specular);

                vec3 result = ambient + diffuse + specular;
                FragColor = vec4(result, 1.0);
            }
        )";

        m_meshShader = agl::ShaderProgram::CreateFromSources(vertexShaderSource, fragmentShaderSource);
    }

    void HandleInput(float deltaTime) {
        auto input = GetInput();

        // Fire projectile with space
        if (input->IsKeyPressed(agl::Keys::Space) && m_shooter->CanFire()) {
            FireProjectile();
        }

        // Change projectile type with number keys
        for (int i = 0; i < 5; ++i) {
            if (input->IsKeyPressed(GLFW_KEY_1 + i)) {
                m_currentProjectileIndex = i;
            }
        }

        // Toggle auto fire with F
        if (input->IsKeyPressed(GLFW_KEY_F)) {
            m_autoFire = !m_autoFire;
            m_autoFireTimer = 0.0f;
        }

        // Clear projectiles with C
        if (input->IsKeyPressed(GLFW_KEY_C)) {
            m_projectileSystem->ClearAll();
        }
    }

    void FireProjectile() {
        agl::ProjectileType type = m_projectileTypes[m_currentProjectileIndex];

        // Calculate shooting direction from camera
        glm::vec3 cameraPos = m_camera->GetPosition();
        glm::vec3 cameraFront = m_camera->GetFront();

        // Fire from slightly in front of camera
        glm::vec3 firePos = cameraPos + cameraFront * 0.5f;

        bool success = m_shooter->Fire(firePos, cameraFront, type);

        if (success) {
            std::cout << "Fired " << m_projectileNames[m_currentProjectileIndex] << " projectile!" << std::endl;
        }
    }

    void RenderTargetGrid(const glm::mat4 &view, const glm::mat4 &projection) {
        // Create a simple grid of cubes as targets
        static std::unique_ptr<agl::Mesh> targetCube = nullptr;
        if (!targetCube) {
            targetCube = std::make_unique<agl::Mesh>(agl::Mesh::CreateCube());

            agl::Material targetMaterial;
            targetMaterial.diffuse = glm::vec3(0.8f, 0.2f, 0.2f); // Red targets
            targetMaterial.specular = glm::vec3(0.5f, 0.5f, 0.5f);
            targetMaterial.shininess = 32.0f;

            targetCube->SetMaterial(targetMaterial);
        }

        m_meshShader->Use();
        m_meshShader->SetUniform("view", view);
        m_meshShader->SetUniform("projection", projection);
        m_meshShader->SetUniform("lightPos", glm::vec3(0.0f, 10.0f, 10.0f));
        m_meshShader->SetUniform("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        m_meshShader->SetUniform("viewPos", m_camera->GetPosition());

        // Render a grid of target cubes
        for (int x = -3; x <= 3; x += 2) {
            for (int y = -3; y <= 3; y += 2) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x * 2.0f, y * 2.0f, -20.0f));

                targetCube->Render(*m_meshShader, model);
            }
        }
    }
};

int main() {
    ProjectileDemoGame game;

    if (game.Initialize()) {
        std::cout << "Starting Projectile Demo game loop..." << std::endl;
        game.Run();
    } else {
        std::cerr << "Failed to initialize the game!" << std::endl;
        return -1;
    }

    return 0;
}
