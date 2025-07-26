#include "buffer.h"
#include <iostream>

namespace agl {

// ===== Base Buffer Class =====

Buffer::Buffer() : m_bufferID(0), m_target(0), m_size(0), m_usage(GL_STATIC_DRAW) {}

Buffer::~Buffer() {
    if (m_bufferID != 0) {
        glDeleteBuffers(1, &m_bufferID);
    }
}

Buffer::Buffer(Buffer &&other) noexcept
    : m_bufferID(other.m_bufferID), m_target(other.m_target), m_size(other.m_size), m_usage(other.m_usage) {
    other.m_bufferID = 0;
    other.m_target = 0;
    other.m_size = 0;
    other.m_usage = GL_STATIC_DRAW;
}

Buffer &Buffer::operator=(Buffer &&other) noexcept {
    if (this != &other) {
        // Clean up current buffer
        if (m_bufferID != 0) {
            glDeleteBuffers(1, &m_bufferID);
        }

        // Move data
        m_bufferID = other.m_bufferID;
        m_target = other.m_target;
        m_size = other.m_size;
        m_usage = other.m_usage;

        // Reset other
        other.m_bufferID = 0;
        other.m_target = 0;
        other.m_size = 0;
        other.m_usage = GL_STATIC_DRAW;
    }
    return *this;
}

void Buffer::Bind() const {
    if (m_bufferID != 0 && m_target != 0) {
        glBindBuffer(m_target, m_bufferID);
    }
}

void Buffer::Unbind() const {
    if (m_target != 0) {
        glBindBuffer(m_target, 0);
    }
}

void Buffer::CreateBuffer(GLenum target) {
    m_target = target;
    if (m_bufferID == 0) {
        glGenBuffers(1, &m_bufferID);
    }
}

void Buffer::SetData(const void *data, size_t size, GLenum usage) {
    m_size = size;
    m_usage = usage;

    Bind();
    glBufferData(m_target, size, data, usage);
}

void Buffer::SetSubData(const void *data, size_t offset, size_t size) {
    Bind();
    glBufferSubData(m_target, offset, size, data);
}

// ===== VertexBuffer Class =====

VertexBuffer::VertexBuffer() {
    CreateBuffer(GL_ARRAY_BUFFER);
}

VertexBuffer::VertexBuffer(const void *data, size_t size, GLenum usage) {
    CreateBuffer(GL_ARRAY_BUFFER);
    SetData(data, size, usage);
}

void VertexBuffer::SetData(const void *data, size_t size, GLenum usage) {
    Buffer::SetData(data, size, usage);
}

void VertexBuffer::SetSubData(const void *data, size_t offset, size_t size) {
    Buffer::SetSubData(data, offset, size);
}

// ===== IndexBuffer Class =====

IndexBuffer::IndexBuffer() : m_count(0) {
    CreateBuffer(GL_ELEMENT_ARRAY_BUFFER);
}

IndexBuffer::IndexBuffer(const uint32_t *indices, uint32_t count, GLenum usage) : m_count(0) {
    CreateBuffer(GL_ELEMENT_ARRAY_BUFFER);
    SetData(indices, count, usage);
}

IndexBuffer::IndexBuffer(const std::vector<uint32_t> &indices, GLenum usage) : m_count(0) {
    CreateBuffer(GL_ELEMENT_ARRAY_BUFFER);
    SetData(indices, usage);
}

void IndexBuffer::SetData(const uint32_t *indices, uint32_t count, GLenum usage) {
    m_count = count;
    Buffer::SetData(indices, count * sizeof(uint32_t), usage);
}

void IndexBuffer::SetData(const std::vector<uint32_t> &indices, GLenum usage) {
    m_count = static_cast<uint32_t>(indices.size());
    Buffer::SetData(indices.data(), indices.size() * sizeof(uint32_t), usage);
}

void IndexBuffer::SetSubData(const uint32_t *indices, uint32_t offset, uint32_t count) {
    Buffer::SetSubData(indices, offset * sizeof(uint32_t), count * sizeof(uint32_t));
}

// ===== UniformBuffer Class =====

UniformBuffer::UniformBuffer() {
    CreateBuffer(GL_UNIFORM_BUFFER);
}

UniformBuffer::UniformBuffer(const void *data, size_t size, GLenum usage) {
    CreateBuffer(GL_UNIFORM_BUFFER);
    SetData(data, size, usage);
}

void UniformBuffer::SetData(const void *data, size_t size, GLenum usage) {
    Buffer::SetData(data, size, usage);
}

void UniformBuffer::SetSubData(const void *data, size_t offset, size_t size) {
    Buffer::SetSubData(data, offset, size);
}

void UniformBuffer::BindToBindingPoint(uint32_t bindingPoint) const {
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_bufferID);
}

void UniformBuffer::BindRange(uint32_t bindingPoint, size_t offset, size_t size) const {
    glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, m_bufferID, offset, size);
}

} // namespace agl
