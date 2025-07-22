#include "VertexBufferLayout.h"

namespace agl {

void VertexBufferLayout::Push(const std::string& name, VertexDataType type, uint32_t count, bool normalized) {
    m_elements.emplace_back(name, type, count, normalized);
    CalculateOffsetsAndStride();
}

void VertexBufferLayout::CalculateOffsetsAndStride() {
    uint32_t offset = 0;
    m_stride = 0;

    for (auto& element : m_elements) {
        element.offset = offset;
        offset += element.GetSize();
        m_stride += element.GetSize();
    }
}

} // namespace agl
