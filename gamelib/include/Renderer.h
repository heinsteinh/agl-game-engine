#ifndef RENDERER_H
#define RENDERER_H

// AGL Renderer - Main header file for the AGL rendering system
// Include this file to get access to all rendering components

#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "buffer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace agl {

// Basic renderer class for common rendering operations
class Renderer {
public:
    Renderer();
    ~Renderer();

    // Initialize renderer (call after OpenGL context is created)
    static void Initialize();
    static void Shutdown();

    // Clear screen
    static void Clear();
    static void SetClearColor(float r, float g, float b, float a = 1.0f);
    static void SetClearColor(const glm::vec4 &color);

    // Viewport
    static void SetViewport(int x, int y, int width, int height);

    // Enable/disable features
    static void EnableDepthTest();
    static void DisableDepthTest();
    static void EnableBlending();
    static void DisableBlending();
    static void EnableWireframe();
    static void DisableWireframe();

    // Drawing
    static void DrawArrays(const VertexArray &vao, const ShaderProgram &shader, GLenum mode = GL_TRIANGLES,
                           uint32_t count = 0);
    static void DrawElements(const VertexArray &vao, const ShaderProgram &shader, GLenum mode = GL_TRIANGLES);

    // Textured drawing
    static void DrawElements(const VertexArray &vao, const ShaderProgram &shader, const Texture2D &texture,
                             GLenum mode = GL_TRIANGLES);

    // Immediate mode helpers (for simple rendering)
    static void DrawQuad(const glm::mat4 &transform, const glm::vec4 &color);
    static void DrawTexturedQuad(const glm::mat4 &transform, const Texture2D &texture,
                                 const glm::vec4 &tint = glm::vec4(1.0f));
    static void DrawCube(const glm::mat4 &transform, const glm::vec4 &color);

    // Statistics
    static uint32_t GetDrawCallCount() {
        return s_stats.drawCalls;
    }
    static uint32_t GetVertexCount() {
        return s_stats.vertexCount;
    }
    static uint32_t GetTriangleCount() {
        return s_stats.triangleCount;
    }
    static void ResetStats();

private:
    struct RenderStats {
        uint32_t drawCalls = 0;
        uint32_t vertexCount = 0;
        uint32_t triangleCount = 0;
    };

    static RenderStats s_stats;
    static std::unique_ptr<VertexArray> s_quadVAO;
    static std::unique_ptr<VertexArray> s_cubeVAO;
    static std::unique_ptr<VertexArray> s_texturedQuadVAO;
    static std::unique_ptr<ShaderProgram> s_colorShader;
    static std::unique_ptr<ShaderProgram> s_textureShader;

    static void CreateQuadVAO();
    static void CreateTexturedQuadVAO();
    static void CreateCubeVAO();
};

// Utility functions for common vertex data
namespace VertexData {
// Quad vertices (position + texture coordinates)
extern const float QuadVertices[];
extern const uint32_t QuadIndices[];
extern const size_t QuadVertexCount;
extern const size_t QuadIndexCount;

// Cube vertices (position + normal + texture coordinates)
extern const float CubeVertices[];
extern const uint32_t CubeIndices[];
extern const size_t CubeVertexCount;
extern const size_t CubeIndexCount;
} // namespace VertexData

} // namespace agl

#endif // RENDERER_H
