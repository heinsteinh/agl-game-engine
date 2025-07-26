#include "agl.h"
#include "Renderer.h"
#include "Texture.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

// Texture demo showcasing the AGL texture system
class TextureDemo : public agl::Game {
public:
    TextureDemo()
        : m_rotation(0.0f)
        , m_currentTextureIndex(0)
        , m_regenerateRandom(false) {
    }

    bool Initialize(int width = 1024, int height = 768, const char* title = "AGL Texture Demo")
     {
        if (!agl::Game::Initialize(width, height, title)) {
            return false;
        }

        // Initialize the AGL renderer
        agl::Renderer::Initialize();

        // Create a basic texture shader
        m_shader = agl::ShaderProgram::CreateBasicTextureShader();
        if (!m_shader) {
            std::cerr << "Failed to create texture shader!" << std::endl;
            return false;
        }

        // Create quad vertices with texture coordinates
        float quadVertices[] = {
            // Positions        // Texture Coords
            -0.8f, -0.8f, 0.0f, 0.0f, 0.0f,  // Bottom-left
             0.8f, -0.8f, 0.0f, 1.0f, 0.0f,  // Bottom-right
             0.8f,  0.8f, 0.0f, 1.0f, 1.0f,  // Top-right
            -0.8f,  0.8f, 0.0f, 0.0f, 1.0f   // Top-left
        };

        uint32_t quadIndices[] = {
            0, 1, 2,  // First triangle
            2, 3, 0   // Second triangle
        };

        // Create vertex buffer and layout
        m_vertexBuffer = std::make_shared<agl::VertexBuffer>(quadVertices, sizeof(quadVertices));
        m_indexBuffer = std::make_shared<agl::IndexBuffer>(quadIndices, 6);

        agl::VertexBufferLayout layout = agl::VertexBufferLayout::PositionTexture3D();

        // Create vertex array
        m_vertexArray = agl::VertexArray::Create();
        m_vertexArray->AddVertexBuffer(m_vertexBuffer, layout);
        m_vertexArray->SetIndexBuffer(m_indexBuffer);

        // Create various textures
        CreateTextures();

        // Set up camera matrices
        UpdateMatrices();

        std::cout << "AGL Texture Demo initialized!" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  1-6: Switch between different textures" << std::endl;
        std::cout << "  R: Regenerate random texture" << std::endl;
        return true;
    }

    void OnUpdate(float deltaTime) override {
        // Update rotation
        m_rotation += deltaTime * 30.0f; // 30 degrees per second

        // Handle input for texture switching
        if (GetInput()->IsKeyPressed(GLFW_KEY_1)) m_currentTextureIndex = 0;
        if (GetInput()->IsKeyPressed(GLFW_KEY_2)) m_currentTextureIndex = 1;
        if (GetInput()->IsKeyPressed(GLFW_KEY_3)) m_currentTextureIndex = 2;
        if (GetInput()->IsKeyPressed(GLFW_KEY_4)) m_currentTextureIndex = 3;
        if (GetInput()->IsKeyPressed(GLFW_KEY_5)) m_currentTextureIndex = 4;
        if (GetInput()->IsKeyPressed(GLFW_KEY_6)) m_currentTextureIndex = 5;

        if (GetInput()->IsKeyPressed(GLFW_KEY_R)) {
            m_regenerateRandom = true;
        }

        // Regenerate random texture if requested
        if (m_regenerateRandom) {
            m_textures[1] = agl::Texture2D::CreateRandomColorTexture(256, 256);
            m_regenerateRandom = false;
        }

        // Clamp texture index
        if (m_currentTextureIndex >= m_textures.size()) {
            m_currentTextureIndex = 0;
        }

        // Update matrices
        UpdateMatrices();
    }

    void OnRender() override {
        // Clear with a dark background
        agl::Renderer::SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        agl::Renderer::Clear();

        if (m_shader && m_vertexArray && !m_textures.empty()) {
            // Create model matrix with rotation
            glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 mvp = m_projection * m_view * model;

            // Bind current texture
            m_textures[m_currentTextureIndex]->Bind(0);

            // Draw textured quad
            m_shader->Use();
            m_shader->SetUniform("u_MVP", mvp);
            m_shader->SetUniform("u_Texture", 0); // Texture slot 0
            m_shader->SetUniform("u_Color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // White tint

            agl::Renderer::DrawElements(*m_vertexArray, *m_shader);
        }
    }

    void OnImGuiRender() override {
        ImGui::Begin("AGL Texture Demo");

        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);

        ImGui::Separator();

        const char* textureNames[] = {
            "Solid Red",
            "Random Colors",
            "Checkerboard",
            "Noise",
            "Horizontal Gradient",
            "Vertical Gradient"
        };

        ImGui::Text("Current Texture: %s", textureNames[m_currentTextureIndex]);

        if (ImGui::Button("Previous Texture")) {
            if (m_currentTextureIndex == 0) {
                m_currentTextureIndex = static_cast<uint32_t>(m_textures.size() - 1);
            } else {
                m_currentTextureIndex--;
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Next Texture")) {
            m_currentTextureIndex = (m_currentTextureIndex + 1) % static_cast<uint32_t>(m_textures.size());
        }

        ImGui::Separator();

        if (ImGui::Button("Regenerate Random Texture")) {
            m_regenerateRandom = true;
        }

        ImGui::Separator();
        ImGui::Text("Keyboard Controls:");
        ImGui::Text("1-6: Select texture directly");
        ImGui::Text("R: Regenerate random texture");

        ImGui::Separator();
        ImGui::Text("Texture Info:");
        if (!m_textures.empty() && m_currentTextureIndex < m_textures.size()) {
            auto& currentTexture = m_textures[m_currentTextureIndex];
            ImGui::Text("Size: %ux%u", currentTexture->GetWidth(), currentTexture->GetHeight());
            ImGui::Text("ID: %u", currentTexture->GetID());
        }

        ImGui::Separator();
        ImGui::Text("Render Statistics:");
        ImGui::Text("Draw Calls: %u", agl::Renderer::GetDrawCallCount());
        ImGui::Text("Vertices: %u", agl::Renderer::GetVertexCount());
        ImGui::Text("Triangles: %u", agl::Renderer::GetTriangleCount());

        ImGui::End();

        // Reset stats each frame
        agl::Renderer::ResetStats();
    }

    void Shutdown() {
        // Clean up resources
        m_textures.clear();
        m_shader.reset();
        m_vertexArray.reset();
        m_vertexBuffer.reset();
        m_indexBuffer.reset();

        // Shutdown renderer
        agl::Renderer::Shutdown();

        // Call parent shutdown
        agl::Game::Shutdown();
    }

private:
    // Rendering objects
    std::unique_ptr<agl::ShaderProgram> m_shader;
    std::unique_ptr<agl::VertexArray> m_vertexArray;
    std::shared_ptr<agl::VertexBuffer> m_vertexBuffer;
    std::shared_ptr<agl::IndexBuffer> m_indexBuffer;

    // Textures
    std::vector<std::unique_ptr<agl::Texture2D>> m_textures;
    uint32_t m_currentTextureIndex;
    bool m_regenerateRandom;

    // Camera matrices
    glm::mat4 m_projection;
    glm::mat4 m_view;

    // Demo state
    float m_rotation;

    void CreateTextures() {
        // 1. Solid red texture
        m_textures.push_back(agl::Texture2D::CreateSolidColorTexture(64, 64, 1.0f, 0.2f, 0.2f, 1.0f));

        // 2. Random color texture (256x256)
        m_textures.push_back(agl::Texture2D::CreateRandomColorTexture(256, 256));

        // 3. Checkerboard texture
        auto checkerboard = std::make_unique<agl::Texture2D>();
        checkerboard->CreateCheckerboard(128, 128, 16,
                                       1.0f, 1.0f, 1.0f,   // White
                                       0.0f, 0.0f, 0.0f);  // Black
        m_textures.push_back(std::move(checkerboard));

        // 4. Noise texture
        auto noise = std::make_unique<agl::Texture2D>();
        noise->CreateNoise(256, 256, 12345);
        m_textures.push_back(std::move(noise));

        // 5. Horizontal gradient (red to blue)
        auto hGradient = std::make_unique<agl::Texture2D>();
        hGradient->CreateGradient(256, 64,
                                1.0f, 0.0f, 0.0f,   // Red
                                0.0f, 0.0f, 1.0f,   // Blue
                                true);              // Horizontal
        m_textures.push_back(std::move(hGradient));

        // 6. Vertical gradient (yellow to green)
        auto vGradient = std::make_unique<agl::Texture2D>();
        vGradient->CreateGradient(64, 256,
                                1.0f, 1.0f, 0.0f,   // Yellow
                                0.0f, 1.0f, 0.0f,   // Green
                                false);             // Vertical
        m_textures.push_back(std::move(vGradient));

        // Set filtering for all textures
        for (auto& texture : m_textures) {
            texture->SetFilter(agl::TextureFilter::Nearest, agl::TextureFilter::Nearest);
            texture->SetWrap(agl::TextureWrap::Repeat, agl::TextureWrap::Repeat);
        }
    }

    void UpdateMatrices() {
        // Perspective projection
        float aspect = static_cast<float>(GetWindow()->GetWidth()) / static_cast<float>(GetWindow()->GetHeight());
        m_projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

        // View matrix (camera)
        m_view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 3.0f),   // Camera position
            glm::vec3(0.0f, 0.0f, 0.0f),   // Look at origin
            glm::vec3(0.0f, 1.0f, 0.0f)    // Up vector
        );
    }
};

int main_Texture() {
    std::cout << "Starting AGL Texture Demo..." << std::endl;

    TextureDemo demo;

    if (demo.Initialize(1024, 768, "AGL Texture Demo - Procedural Textures")) {
        std::cout << "Demo initialized successfully!" << std::endl;
        demo.Run();
    } else {
        std::cerr << "Failed to initialize demo!" << std::endl;
        return -1;
    }

    return 0;
}
