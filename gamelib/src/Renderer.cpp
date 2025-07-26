#include "Renderer.h"
#include <iostream>

namespace agl {

// ===== VertexData Namespace =====

namespace VertexData {
// Quad vertices (x, y, z)
const float QuadVertices[] = {
    -0.5f, -0.5f, 0.0f, // Bottom-left
    0.5f,  -0.5f, 0.0f, // Bottom-right
    0.5f,  0.5f,  0.0f, // Top-right
    -0.5f, 0.5f,  0.0f  // Top-left
};

const uint32_t QuadIndices[] = {
    0, 1, 2, // First triangle
    2, 3, 0  // Second triangle
};

const size_t QuadVertexCount = sizeof(QuadVertices) / sizeof(float);
const size_t QuadIndexCount = sizeof(QuadIndices) / sizeof(uint32_t);

// Textured Quad vertices (x, y, z, u, v)
const float QuadTexturedVertices[] = {
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // Bottom-left
    0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, // Bottom-right
    0.5f,  0.5f,  0.0f, 1.0f, 1.0f, // Top-right
    -0.5f, 0.5f,  0.0f, 0.0f, 1.0f  // Top-left
};

const size_t QuadTexturedVertexCount = sizeof(QuadTexturedVertices) / sizeof(float);

// Cube vertices (x, y, z) - 8 vertices
const float CubeVertices[] = {
    // Front face
    -0.5f, -0.5f, 0.5f, // 0
    0.5f, -0.5f, 0.5f,  // 1
    0.5f, 0.5f, 0.5f,   // 2
    -0.5f, 0.5f, 0.5f,  // 3

    // Back face
    -0.5f, -0.5f, -0.5f, // 4
    0.5f, -0.5f, -0.5f,  // 5
    0.5f, 0.5f, -0.5f,   // 6
    -0.5f, 0.5f, -0.5f   // 7
};

const uint32_t CubeIndices[] = {
    // Front face
    0, 1, 2, 2, 3, 0,
    // Back face
    4, 5, 6, 6, 7, 4,
    // Left face
    7, 3, 0, 0, 4, 7,
    // Right face
    1, 5, 6, 6, 2, 1,
    // Bottom face
    4, 0, 1, 1, 5, 4,
    // Top face
    3, 7, 6, 6, 2, 3};

const size_t CubeVertexCount = sizeof(CubeVertices) / sizeof(float);
const size_t CubeIndexCount = sizeof(CubeIndices) / sizeof(uint32_t);
} // namespace VertexData

// ===== Renderer Class =====

Renderer::RenderStats Renderer::s_stats;
std::unique_ptr<VertexArray> Renderer::s_quadVAO;
std::unique_ptr<VertexArray> Renderer::s_cubeVAO;
std::unique_ptr<VertexArray> Renderer::s_texturedQuadVAO;
std::unique_ptr<ShaderProgram> Renderer::s_colorShader;
std::unique_ptr<ShaderProgram> Renderer::s_textureShader;

void Renderer::Initialize() {
    // Enable depth testing by default
    EnableDepthTest();

    // Enable blending for transparency
    EnableBlending();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create basic shaders
    s_colorShader = ShaderProgram::CreateBasicColorShader();
    s_textureShader = ShaderProgram::CreateBasicTextureShader();

    // Create basic geometry
    CreateQuadVAO();
    CreateTexturedQuadVAO();
    CreateCubeVAO();

    std::cout << "AGL Renderer initialized successfully!" << std::endl;
}

void Renderer::Shutdown() {
    s_quadVAO.reset();
    s_cubeVAO.reset();
    s_texturedQuadVAO.reset();
    s_colorShader.reset();
    s_textureShader.reset();
    std::cout << "AGL Renderer shutdown complete." << std::endl;
}

void Renderer::Clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::SetClearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

void Renderer::SetClearColor(const glm::vec4 &color) {
    glClearColor(color.r, color.g, color.b, color.a);
}

void Renderer::SetViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void Renderer::EnableDepthTest() {
    glEnable(GL_DEPTH_TEST);
}

void Renderer::DisableDepthTest() {
    glDisable(GL_DEPTH_TEST);
}

void Renderer::EnableBlending() {
    glEnable(GL_BLEND);
}

void Renderer::DisableBlending() {
    glDisable(GL_BLEND);
}

void Renderer::EnableWireframe() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void Renderer::DisableWireframe() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer::DrawArrays(const VertexArray &vao, const ShaderProgram &shader, GLenum mode, uint32_t count) {
    shader.Use();
    vao.Bind();

    if (count == 0) {
        // Estimate vertex count from first vertex buffer
        const auto &vbs = vao.GetVertexBuffers();
        if (!vbs.empty()) {
            count = static_cast<uint32_t>(vbs[0]->GetSize() / sizeof(float) / 3); // Assume 3 floats per vertex
        }
    }

    glDrawArrays(mode, 0, count);

    s_stats.drawCalls++;
    s_stats.vertexCount += count;
    if (mode == GL_TRIANGLES) {
        s_stats.triangleCount += count / 3;
    }
}

void Renderer::DrawElements(const VertexArray &vao, const ShaderProgram &shader, GLenum mode) {
    shader.Use();
    vao.Bind();

    auto indexBuffer = vao.GetIndexBuffer();
    if (indexBuffer) {
        uint32_t indexCount = indexBuffer->GetCount();
        glDrawElements(mode, indexCount, GL_UNSIGNED_INT, nullptr);

        s_stats.drawCalls++;
        s_stats.vertexCount += indexCount;
        if (mode == GL_TRIANGLES) {
            s_stats.triangleCount += indexCount / 3;
        }
    }
}

void Renderer::DrawElements(const VertexArray &vao, const ShaderProgram &shader, const Texture2D &texture,
                            GLenum mode) {
    texture.Bind(0);
    DrawElements(vao, shader, mode);
}

void Renderer::DrawQuad(const glm::mat4 &transform, const glm::vec4 &color) {
    if (s_quadVAO && s_colorShader) {
        s_colorShader->Use();
        s_colorShader->SetUniform("u_MVP", transform);
        s_colorShader->SetUniform("u_Color", color);

        DrawElements(*s_quadVAO, *s_colorShader);
    }
}

void Renderer::DrawTexturedQuad(const glm::mat4 &transform, const Texture2D &texture, const glm::vec4 &tint) {
    if (s_texturedQuadVAO && s_textureShader) {
        texture.Bind(0);
        s_textureShader->Use();
        s_textureShader->SetUniform("u_MVP", transform);
        s_textureShader->SetUniform("u_Texture", 0);
        s_textureShader->SetUniform("u_Color", tint);

        DrawElements(*s_texturedQuadVAO, *s_textureShader);
    }
}

void Renderer::DrawCube(const glm::mat4 &transform, const glm::vec4 &color) {
    if (s_cubeVAO && s_colorShader) {
        s_colorShader->Use();
        s_colorShader->SetUniform("u_MVP", transform);
        s_colorShader->SetUniform("u_Color", color);

        DrawElements(*s_cubeVAO, *s_colorShader);
    }
}

void Renderer::ResetStats() {
    s_stats.drawCalls = 0;
    s_stats.vertexCount = 0;
    s_stats.triangleCount = 0;
}

void Renderer::CreateQuadVAO() {
    s_quadVAO = VertexArray::Create();

    // Create vertex buffer
    auto vertexBuffer =
        std::make_shared<VertexBuffer>(VertexData::QuadVertices, VertexData::QuadVertexCount * sizeof(float));

    // Create index buffer
    auto indexBuffer =
        std::make_shared<IndexBuffer>(VertexData::QuadIndices, static_cast<uint32_t>(VertexData::QuadIndexCount));

    // Set up layout (position only for basic shader)
    VertexBufferLayout layout = VertexBufferLayout::Position3D();

    s_quadVAO->AddVertexBuffer(vertexBuffer, layout);
    s_quadVAO->SetIndexBuffer(indexBuffer);
}

void Renderer::CreateTexturedQuadVAO() {
    s_texturedQuadVAO = VertexArray::Create();

    // Create vertex buffer with texture coordinates
    auto vertexBuffer = std::make_shared<VertexBuffer>(agl::VertexData::QuadTexturedVertices,
                                                       agl::VertexData::QuadTexturedVertexCount * sizeof(float));

    // Create index buffer
    auto indexBuffer = std::make_shared<IndexBuffer>(agl::VertexData::QuadIndices,
                                                     static_cast<uint32_t>(agl::VertexData::QuadIndexCount));

    // Set up layout (position + texture coordinates)
    VertexBufferLayout layout = VertexBufferLayout::PositionTexture3D();

    s_texturedQuadVAO->AddVertexBuffer(vertexBuffer, layout);
    s_texturedQuadVAO->SetIndexBuffer(indexBuffer);
}

void Renderer::CreateCubeVAO() {
    s_cubeVAO = VertexArray::Create();

    // Create vertex buffer
    auto vertexBuffer =
        std::make_shared<VertexBuffer>(VertexData::CubeVertices, VertexData::CubeVertexCount * sizeof(float));

    // Create index buffer
    auto indexBuffer =
        std::make_shared<IndexBuffer>(VertexData::CubeIndices, static_cast<uint32_t>(VertexData::CubeIndexCount));

    // Set up layout (position only for basic shader)
    VertexBufferLayout layout = VertexBufferLayout::Position3D();

    s_cubeVAO->AddVertexBuffer(vertexBuffer, layout);
    s_cubeVAO->SetIndexBuffer(indexBuffer);
}

} // namespace agl
