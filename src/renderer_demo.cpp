#include "game.h"
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
        , m_wireframeMode(false) {
    }

    bool Initialize(int width = 1024, int height = 768, const char* title = "AGL Renderer Demo") override {
        if (!agl::Game::Initialize(width, height, title)) {
            return false;
        }
        
        // Initialize the AGL renderer
        agl::Renderer::Initialize();
        
        // Create a simple shader program
        m_shader = agl::ShaderProgram::CreateBasicColorShader();
        if (!m_shader) {
            std::cerr << "Failed to create shader program!" << std::endl;
            return false;
        }
        
        // Create triangle vertices
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
        
        std::cout << "AGL Renderer Demo initialized!" << std::endl;
        return true;
    }
    
    void OnUpdate(float deltaTime) override {
        // Update rotation
        m_rotation += deltaTime * 45.0f; // 45 degrees per second
        
        // Handle input
        if (GetInput()->IsKeyPressed(GLFW_KEY_TAB)) {
            m_wireframeMode = !m_wireframeMode;
            if (m_wireframeMode) {
                agl::Renderer::EnableWireframe();
            } else {
                agl::Renderer::DisableWireframe();
            }
        }
        
        // Update matrices
        UpdateMatrices();
    }
    
    void OnRender() override {
        // Clear with a nice blue color
        agl::Renderer::SetClearColor(0.2f, 0.3f, 0.8f, 1.0f);
        agl::Renderer::Clear();
        
        if (m_shader && m_triangleVA) {
            // Create model matrix with rotation
            glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 mvp = m_projection * m_view * model;
            
            // Draw triangle
            m_shader->Use();
            m_shader->SetUniform("u_MVP", mvp);
            m_shader->SetUniform("u_Color", glm::vec4(1.0f, 0.5f, 0.2f, 1.0f)); // Orange color
            
            agl::Renderer::DrawArrays(*m_triangleVA, *m_shader, GL_TRIANGLES, 3);
            
            // Draw built-in quad and cube
            glm::mat4 quadModel = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f));
            quadModel = glm::scale(quadModel, glm::vec3(0.8f));
            glm::mat4 quadMVP = m_projection * m_view * quadModel;
            
            agl::Renderer::DrawQuad(quadMVP, glm::vec4(0.8f, 0.2f, 0.8f, 1.0f)); // Purple
            
            glm::mat4 cubeModel = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, -2.0f));
            cubeModel = glm::rotate(cubeModel, glm::radians(m_rotation * 0.5f), glm::vec3(1.0f, 1.0f, 0.0f));
            cubeModel = glm::scale(cubeModel, glm::vec3(0.6f));
            glm::mat4 cubeMVP = m_projection * m_view * cubeModel;
            
            agl::Renderer::DrawCube(cubeMVP, glm::vec4(0.2f, 0.8f, 0.2f, 1.0f)); // Green
        }
    }
    
    void OnImGuiRender() override {
        ImGui::Begin("AGL Renderer Demo");
        
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
        
        ImGui::Separator();
        ImGui::Text("Render Statistics:");
        ImGui::Text("Draw Calls: %u", agl::Renderer::GetDrawCallCount());
        ImGui::Text("Vertices: %u", agl::Renderer::GetVertexCount());
        ImGui::Text("Triangles: %u", agl::Renderer::GetTriangleCount());
        
        if (ImGui::Button("Reset Stats")) {
            agl::Renderer::ResetStats();
        }
        
        ImGui::Separator();
        ImGui::Text("Controls:");
        ImGui::Text("TAB - Toggle wireframe mode");
        ImGui::Text("Wireframe: %s", m_wireframeMode ? "ON" : "OFF");
        
        ImGui::Separator();
        ImGui::Text("Objects:");
        ImGui::Text("Orange Triangle - Custom triangle");
        ImGui::Text("Purple Quad - Built-in quad");
        ImGui::Text("Green Cube - Built-in cube");
        
        ImGui::End();
        
        // Reset stats each frame for accurate per-frame statistics
        agl::Renderer::ResetStats();
    }
    
    void Shutdown() override {
        // Clean up renderer resources
        m_shader.reset();
        m_triangleVA.reset();
        m_triangleVB.reset();
        
        // Shutdown renderer
        agl::Renderer::Shutdown();
        
        // Call parent shutdown
        agl::Game::Shutdown();
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
    
    void UpdateMatrices() {
        // Perspective projection
        float aspect = static_cast<float>(GetWindow()->GetWidth()) / static_cast<float>(GetWindow()->GetHeight());
        m_projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        
        // View matrix (camera)
        m_view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 5.0f),   // Camera position
            glm::vec3(0.0f, 0.0f, 0.0f),   // Look at origin
            glm::vec3(0.0f, 1.0f, 0.0f)    // Up vector
        );
    }
};

int main() {
    std::cout << "Starting AGL Renderer Demo..." << std::endl;
    
    RendererDemo demo;
    
    if (demo.Initialize(1024, 768, "AGL Renderer Demo - Encapsulated OpenGL")) {
        std::cout << "Demo initialized successfully!" << std::endl;
        demo.Run();
    } else {
        std::cerr << "Failed to initialize demo!" << std::endl;
        return -1;
    }
    
    return 0;
}
