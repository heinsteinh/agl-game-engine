#pragma once

#include "Buffer.h"
#include "VertexBufferLayout.h"
#include <memory>

#if defined(__APPLE__)
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl3.h>
#else
    #include <GL/gl.h>
#endif

namespace agl {

    // Vertex Array Object - ties together vertex buffers, index buffers, and layouts
    class VertexArray {
    public:
        VertexArray();
        ~VertexArray();

        // Non-copyable but movable
        VertexArray(const VertexArray&) = delete;
        VertexArray& operator=(const VertexArray&) = delete;
        VertexArray(VertexArray&& other) noexcept;
        VertexArray& operator=(VertexArray&& other) noexcept;

        void Bind() const;
        void Unbind() const;

        // Add vertex buffer with layout
        void AddVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer, const VertexBufferLayout& layout);
        
        // Set index buffer
        void SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer);

        // Getters
        uint32_t GetID() const { return m_vaoID; }
        const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const { return m_vertexBuffers; }
        std::shared_ptr<IndexBuffer> GetIndexBuffer() const { return m_indexBuffer; }
        
        // Drawing convenience methods
        void DrawArrays(GLenum mode, uint32_t first, uint32_t count) const;
        void DrawElements(GLenum mode = GL_TRIANGLES) const;
        void DrawElements(GLenum mode, uint32_t count) const;

        // Static factory methods
        static std::unique_ptr<VertexArray> Create();

    private:
        uint32_t m_vaoID;
        std::vector<std::shared_ptr<VertexBuffer>> m_vertexBuffers;
        std::shared_ptr<IndexBuffer> m_indexBuffer;
        uint32_t m_vertexBufferIndex;

        void SetupVertexAttributes(const VertexBufferLayout& layout);
    };

} // namespace agl
