#include "agl.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

class MeshDemoGame : public agl::Game {
private:
    std::shared_ptr<agl::Camera> m_camera;
    std::unique_ptr<agl::CameraController> m_controller;

    // Shader for 3D rendering
    std::unique_ptr<agl::ShaderProgram> m_meshShader;

    // Different mesh objects
    std::unique_ptr<agl::Mesh> m_cube;
    std::unique_ptr<agl::Mesh> m_sphere;
    std::unique_ptr<agl::Mesh> m_plane;
    std::unique_ptr<agl::Mesh> m_cylinder;
    std::unique_ptr<agl::Mesh> m_capsule;

    // Current mesh index for switching
    int m_currentMeshIndex = 0;
    const char *m_meshNames[5] = {"Cube", "Sphere", "Plane", "Cylinder", "Capsule"};

    // Wireframe mode
    bool m_wireframe = false;

    // Material
    agl::Material m_material;

public:
    bool Initialize(int width = 1200, int height = 800, const char *title = "AGL Mesh Demo") {
        if (!agl::Game::Initialize(width, height, title)) {
            return false;
        }

        std::cout << "Initializing Mesh Demo..." << std::endl;

        // Setup camera
        m_camera = std::make_shared<agl::Camera>();
        m_camera->SetPosition(glm::vec3(0.0f, 0.0f, 5.0f));
        m_camera->SetPerspective(45.0f, (float)width / height, 0.1f, 100.0f);

        m_controller = std::make_unique<agl::CameraController>(m_camera);
        m_controller->Initialize(GetInput());
        m_controller->SetMode(agl::CameraMode::FirstPerson);

        // Create shader
        CreateShader();

        // Create mesh
        CreateMeshes();

        // Enable depth testing
        glEnable(GL_DEPTH_TEST);

        return true;
    }

    void CreateShader() {
        // Basic vertex shader
        const char *vertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec3 a_Position;
            layout (location = 1) in vec3 a_Normal;
            layout (location = 2) in vec2 a_TexCoords;

            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;
            uniform mat3 normalMatrix;

            out vec3 FragPos;
            out vec3 Normal;
            out vec2 TexCoords;

            void main() {
                FragPos = vec3(model * vec4(a_Position, 1.0));
                Normal = normalMatrix * a_Normal;
                TexCoords = a_TexCoords;

                gl_Position = projection * view * vec4(FragPos, 1.0);
            }
        )";

        // Basic fragment shader with lighting
        const char *fragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;

            in vec3 FragPos;
            in vec3 Normal;
            in vec2 TexCoords;

            uniform vec3 lightPos;
            uniform vec3 lightColor;
            uniform vec3 viewPos;

            struct Material {
                vec3 ambient;
                vec3 diffuse;
                vec3 specular;
                float shininess;
                bool hasDiffuseTexture;
                sampler2D diffuseTexture;
            };

            uniform Material material;

            void main() {
                // Ambient
                vec3 ambient = lightColor * material.ambient;

                // Diffuse
                vec3 norm = normalize(Normal);
                vec3 lightDir = normalize(lightPos - FragPos);
                float diff = max(dot(norm, lightDir), 0.0);
                vec3 diffuse = lightColor * (diff * material.diffuse);

                // Specular
                vec3 viewDir = normalize(viewPos - FragPos);
                vec3 reflectDir = reflect(-lightDir, norm);
                float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
                vec3 specular = lightColor * (spec * material.specular);

                vec3 result = ambient + diffuse + specular;
                FragColor = vec4(result, 1.0);
            }
        )";

        m_meshShader = agl::ShaderProgram::CreateFromSources(vertexShaderSource, fragmentShaderSource);
    }

    void CreateMeshes() {
        // Create material
        m_material.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
        m_material.diffuse = glm::vec3(0.8f, 0.6f, 0.4f);
        m_material.specular = glm::vec3(1.0f, 1.0f, 1.0f);
        m_material.shininess = 32.0f;

        // Create primitive meshes
        m_cube = std::make_unique<agl::Mesh>(agl::Mesh::CreateCube());
        m_cube->SetMaterial(m_material);

        m_sphere = std::make_unique<agl::Mesh>(agl::Mesh::CreateSphere(1.0f, 32, 16));
        m_sphere->SetMaterial(m_material);

        m_plane = std::make_unique<agl::Mesh>(agl::Mesh::CreatePlane(2.0f, 2.0f));
        m_plane->SetMaterial(m_material);

        m_cylinder = std::make_unique<agl::Mesh>(agl::Mesh::CreateCylinder(1.0f, 2.0f, 32, 1));
        m_cylinder->SetMaterial(m_material);

        m_capsule = std::make_unique<agl::Mesh>(agl::Mesh::CreateCapsule(1.0f, 1.5f, 32, 4));
        m_capsule->SetMaterial(m_material);

        std::cout << "Created all mesh primitives!" << std::endl;
    }

    void OnUpdate(float deltaTime) override {
        // Update camera
        if (m_controller) {
            m_controller->Update(deltaTime);
        }
    }

    void OnRender() override {
        // Clear screen
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set up rendering matrices
        if (m_meshShader && m_camera) {
            m_meshShader->Use();

            glm::mat4 view = m_camera->GetViewMatrix();
            glm::mat4 projection = m_camera->GetProjectionMatrix();
            m_meshShader->SetUniform("view", view);
            m_meshShader->SetUniform("projection", projection);

            // Set lighting uniforms
            glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
            glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
            m_meshShader->SetUniform("lightPos", lightPos);
            m_meshShader->SetUniform("lightColor", lightColor);
            m_meshShader->SetUniform("viewPos", m_camera->GetPosition());

            // Render current mesh
            agl::Mesh *currentMesh = GetCurrentMesh();
            if (currentMesh) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::rotate(model, (float)glfwGetTime() * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
                currentMesh->Render(*m_meshShader, model);
            }
        }
    }

    void OnImGuiRender() override {
        RenderUI();
    }

    void RenderUI() {
        ImGui::Begin("Mesh Demo Controls");

        ImGui::Text("Current Mesh:");
        if (ImGui::Combo("Mesh Type", &m_currentMeshIndex, m_meshNames, 5)) {
            // Mesh changed - update material if needed
            agl::Mesh *mesh = GetCurrentMesh();
            if (mesh) {
                mesh->SetMaterial(m_material);
            }
        }

        ImGui::Separator();

        ImGui::Text("Rendering Options:");
        if (ImGui::Checkbox("Wireframe", &m_wireframe)) {
            // Toggle wireframe mode
            glPolygonMode(GL_FRONT_AND_BACK, m_wireframe ? GL_LINE : GL_FILL);
        }

        ImGui::Separator();

        ImGui::Text("Material Properties:");
        bool materialChanged = false;

        if (ImGui::ColorEdit3("Ambient", &m_material.ambient[0])) {
            materialChanged = true;
        }
        if (ImGui::ColorEdit3("Diffuse", &m_material.diffuse[0])) {
            materialChanged = true;
        }
        if (ImGui::ColorEdit3("Specular", &m_material.specular[0])) {
            materialChanged = true;
        }
        if (ImGui::SliderFloat("Shininess", &m_material.shininess, 1.0f, 256.0f)) {
            materialChanged = true;
        }

        // Update material if changed
        if (materialChanged) {
            agl::Mesh *mesh = GetCurrentMesh();
            if (mesh) {
                mesh->SetMaterial(m_material);
            }
        }

        ImGui::Separator();

        ImGui::Text("Mesh Info:");
        agl::Mesh *currentMesh = GetCurrentMesh();
        if (currentMesh) {
            ImGui::Text("Vertices: %zu", currentMesh->GetVertexCount());
            ImGui::Text("Triangles: %zu", currentMesh->GetTriangleCount());
        }

        ImGui::Separator();

        ImGui::Text("Camera Controls:");
        ImGui::Text("WASD - Move camera");
        ImGui::Text("Mouse - Look around");

        ImGui::End();
    }

    agl::Mesh *GetCurrentMesh() {
        switch (m_currentMeshIndex) {
        case 0:
            return m_cube.get();
        case 1:
            return m_sphere.get();
        case 2:
            return m_plane.get();
        case 3:
            return m_cylinder.get();
        case 4:
            return m_capsule.get();
        default:
            return m_cube.get();
        }
    }

    void OnShutdown() {
        std::cout << "Shutting down Mesh Demo..." << std::endl;
    }
};

int main() {
    try {
        MeshDemoGame game;
        if (!game.Initialize()) {
            return -1;
        }
        game.Run();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
