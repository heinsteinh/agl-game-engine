#include "VertexArray.h"
#include <iostream>

namespace agl {

VertexArray::VertexArray() : m_vaoID(0), m_vertexBufferIndex(0) {
    glGenVertexArrays(1, &m_vaoID);
}

VertexArray::~VertexArray() {
    if (m_vaoID != 0) {
        glDeleteVertexArrays(1, &m_vaoID);
    }
}

VertexArray::VertexArray(VertexArray &&other) noexcept
    : m_vaoID(other.m_vaoID), m_vertexBuffers(std::move(other.m_vertexBuffers)),
      m_indexBuffer(std::move(other.m_indexBuffer)), m_vertexBufferIndex(other.m_vertexBufferIndex) {
    other.m_vaoID = 0;
    other.m_vertexBufferIndex = 0;
}

VertexArray &VertexArray::operator=(VertexArray &&other) noexcept {
    if (this != &other) {
        if (m_vaoID != 0) {
            glDeleteVertexArrays(1, &m_vaoID);
        }

        m_vaoID = other.m_vaoID;
        m_vertexBuffers = std::move(other.m_vertexBuffers);
        m_indexBuffer = std::move(other.m_indexBuffer);
        m_vertexBufferIndex = other.m_vertexBufferIndex;

        other.m_vaoID = 0;
        other.m_vertexBufferIndex = 0;
    }
    return *this;
}

void VertexArray::Bind() const {
    glBindVertexArray(m_vaoID);
}

void VertexArray::Unbind() const {
    glBindVertexArray(0);
}

void VertexArray::AddVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer, const VertexBufferLayout &layout) {
    Bind();
    vertexBuffer->Bind();

    SetupVertexAttributes(layout);

    m_vertexBuffers.push_back(vertexBuffer);
}

void VertexArray::SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer) {
    Bind();
    indexBuffer->Bind();
    m_indexBuffer = indexBuffer;
}

void VertexArray::SetupVertexAttributes(const VertexBufferLayout &layout) {
    const auto &elements = layout.GetElements();

    for (const auto &element : elements) {
        glEnableVertexAttribArray(m_vertexBufferIndex);

        glVertexAttribPointer(m_vertexBufferIndex, element.count, static_cast<GLenum>(element.type),
                              element.normalized ? GL_TRUE : GL_FALSE, layout.GetStride(),
                              reinterpret_cast<const void *>(element.offset));

        m_vertexBufferIndex++;
    }
}

void VertexArray::DrawArrays(GLenum mode, uint32_t first, uint32_t count) const {
    Bind();
    glDrawArrays(mode, first, count);
}

void VertexArray::DrawElements(GLenum mode) const {
    if (m_indexBuffer) {
        DrawElements(mode, m_indexBuffer->GetCount());
    }
}

void VertexArray::DrawElements(GLenum mode, uint32_t count) const {
    Bind();
    glDrawElements(mode, count, GL_UNSIGNED_INT, nullptr);
}

std::unique_ptr<VertexArray> VertexArray::Create() {
    return std::make_unique<VertexArray>();
}

} // namespace agl
