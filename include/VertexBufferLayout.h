#pragma once

#include <vector>
#include <string>
#include <cstdint>

#if defined(__APPLE__)
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl3.h>
#else
    #include <GL/gl.h>
#endif

namespace agl {

    // Enum for vertex attribute data types
    enum class VertexDataType {
        Float = GL_FLOAT,
        Int = GL_INT,
        UnsignedInt = GL_UNSIGNED_INT,
        Byte = GL_BYTE,
        UnsignedByte = GL_UNSIGNED_BYTE,
        Short = GL_SHORT,
        UnsignedShort = GL_UNSIGNED_SHORT
    };

    // Vertex attribute element
    struct VertexElement {
        std::string name;
        VertexDataType type;
        uint32_t count;
        uint32_t offset;
        bool normalized;

        VertexElement(const std::string& name, VertexDataType type, uint32_t count, bool normalized = false)
            : name(name), type(type), count(count), offset(0), normalized(normalized) {}

        uint32_t GetSize() const {
            switch (type) {
                case VertexDataType::Float:      return count * sizeof(float);
                case VertexDataType::Int:        return count * sizeof(int32_t);
                case VertexDataType::UnsignedInt: return count * sizeof(uint32_t);
                case VertexDataType::Byte:       return count * sizeof(int8_t);
                case VertexDataType::UnsignedByte: return count * sizeof(uint8_t);
                case VertexDataType::Short:      return count * sizeof(int16_t);
                case VertexDataType::UnsignedShort: return count * sizeof(uint16_t);
            }
            return 0;
        }
    };

    // Vertex buffer layout manager
    class VertexBufferLayout {
    public:
        VertexBufferLayout() : m_stride(0) {}

        // Add vertex attributes
        void Push(const std::string& name, VertexDataType type, uint32_t count, bool normalized = false);
        
        // Convenience methods for common types
        void PushFloat(const std::string& name, uint32_t count = 1) {
            Push(name, VertexDataType::Float, count);
        }
        
        void PushInt(const std::string& name, uint32_t count = 1) {
            Push(name, VertexDataType::Int, count);
        }
        
        void PushUInt(const std::string& name, uint32_t count = 1) {
            Push(name, VertexDataType::UnsignedInt, count);
        }
        
        void PushByte(const std::string& name, uint32_t count = 1, bool normalized = false) {
            Push(name, VertexDataType::UnsignedByte, count, normalized);
        }

        // Common vertex layouts
        static VertexBufferLayout Position3D() {
            VertexBufferLayout layout;
            layout.PushFloat("a_Position", 3);
            return layout;
        }
        
        static VertexBufferLayout Position2D() {
            VertexBufferLayout layout;
            layout.PushFloat("a_Position", 2);
            return layout;
        }
        
        static VertexBufferLayout PositionTexture2D() {
            VertexBufferLayout layout;
            layout.PushFloat("a_Position", 2);
            layout.PushFloat("a_TexCoord", 2);
            return layout;
        }
        
        static VertexBufferLayout PositionTexture3D() {
            VertexBufferLayout layout;
            layout.PushFloat("a_Position", 3);
            layout.PushFloat("a_TexCoord", 2);
            return layout;
        }
        
        static VertexBufferLayout PositionNormalTexture3D() {
            VertexBufferLayout layout;
            layout.PushFloat("a_Position", 3);
            layout.PushFloat("a_Normal", 3);
            layout.PushFloat("a_TexCoord", 2);
            return layout;
        }
        
        static VertexBufferLayout PositionColorTexture3D() {
            VertexBufferLayout layout;
            layout.PushFloat("a_Position", 3);
            layout.PushFloat("a_Color", 4);
            layout.PushFloat("a_TexCoord", 2);
            return layout;
        }

        // Getters
        const std::vector<VertexElement>& GetElements() const { return m_elements; }
        uint32_t GetStride() const { return m_stride; }
        uint32_t GetElementCount() const { return static_cast<uint32_t>(m_elements.size()); }

        // Iterators for range-based loops
        std::vector<VertexElement>::const_iterator begin() const { return m_elements.begin(); }
        std::vector<VertexElement>::const_iterator end() const { return m_elements.end(); }

        // Clear the layout
        void Clear() {
            m_elements.clear();
            m_stride = 0;
        }

    private:
        std::vector<VertexElement> m_elements;
        uint32_t m_stride;

        void CalculateOffsetsAndStride();
    };

} // namespace agl
