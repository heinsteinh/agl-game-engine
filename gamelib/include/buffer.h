#ifndef BUFFER_H
#define BUFFER_H

#include <cstdint>
#include <vector>

#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

namespace agl {

// Base Buffer class for all OpenGL buffer types
class Buffer {
public:
    Buffer();
    virtual ~Buffer();

    // Non-copyable but movable
    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;
    Buffer(Buffer &&other) noexcept;
    Buffer &operator=(Buffer &&other) noexcept;

    void Bind() const;
    void Unbind() const;

    uint32_t GetID() const {
        return m_bufferID;
    }
    size_t GetSize() const {
        return m_size;
    }
    GLenum GetTarget() const {
        return m_target;
    }

protected:
    void CreateBuffer(GLenum target);
    void SetData(const void *data, size_t size, GLenum usage = GL_STATIC_DRAW);
    void SetSubData(const void *data, size_t offset, size_t size);

    uint32_t m_bufferID;
    GLenum m_target;
    size_t m_size;
    GLenum m_usage;
};

// Vertex Buffer Object
class VertexBuffer : public Buffer {
public:
    VertexBuffer();
    VertexBuffer(const void *data, size_t size, GLenum usage = GL_STATIC_DRAW);
    ~VertexBuffer() = default;

    template <typename T>
    VertexBuffer(const std::vector<T> &vertices, GLenum usage = GL_STATIC_DRAW)
        : VertexBuffer(vertices.data(), vertices.size() * sizeof(T), usage) {}

    void SetData(const void *data, size_t size, GLenum usage = GL_STATIC_DRAW);
    void SetSubData(const void *data, size_t offset, size_t size);

    template <typename T>
    void SetData(const std::vector<T> &vertices, GLenum usage = GL_STATIC_DRAW) {
        SetData(vertices.data(), vertices.size() * sizeof(T), usage);
    }
};

// Index Buffer Object (Element Buffer Object)
class IndexBuffer : public Buffer {
public:
    IndexBuffer();
    IndexBuffer(const uint32_t *indices, uint32_t count, GLenum usage = GL_STATIC_DRAW);
    IndexBuffer(const std::vector<uint32_t> &indices, GLenum usage = GL_STATIC_DRAW);
    ~IndexBuffer() = default;

    void SetData(const uint32_t *indices, uint32_t count, GLenum usage = GL_STATIC_DRAW);
    void SetData(const std::vector<uint32_t> &indices, GLenum usage = GL_STATIC_DRAW);
    void SetSubData(const uint32_t *indices, uint32_t offset, uint32_t count);

    uint32_t GetCount() const {
        return m_count;
    }

private:
    uint32_t m_count;
};

// Uniform Buffer Object
class UniformBuffer : public Buffer {
public:
    UniformBuffer();
    UniformBuffer(const void *data, size_t size, GLenum usage = GL_DYNAMIC_DRAW);
    ~UniformBuffer() = default;

    void SetData(const void *data, size_t size, GLenum usage = GL_DYNAMIC_DRAW);
    void SetSubData(const void *data, size_t offset, size_t size);

    void BindToBindingPoint(uint32_t bindingPoint) const;
    void BindRange(uint32_t bindingPoint, size_t offset, size_t size) const;

    template <typename T>
    void SetData(const T &data, GLenum usage = GL_DYNAMIC_DRAW) {
        SetData(&data, sizeof(T), usage);
    }

    template <typename T>
    void SetSubData(const T &data, size_t offset = 0) {
        SetSubData(&data, offset, sizeof(T));
    }
};

} // namespace agl

#endif // BUFFER_H
