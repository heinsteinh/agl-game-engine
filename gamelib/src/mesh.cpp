#include "mesh.h"
#include <algorithm>
#include <cmath>

#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

namespace agl {

// ========== Constructors ==========

Mesh::Mesh() : m_isSetup(false) {
    m_VAO = std::make_unique<VertexArray>();
}

Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
    : m_vertices(vertices), m_indices(indices), m_isSetup(false) {
    m_VAO = std::make_unique<VertexArray>();
    SetupMesh();
}

Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, const Material &material)
    : m_vertices(vertices), m_indices(indices), m_material(material), m_isSetup(false) {
    m_VAO = std::make_unique<VertexArray>();
    SetupMesh();
}

Mesh::~Mesh() = default;

Mesh::Mesh(Mesh &&other) noexcept
    : m_vertices(std::move(other.m_vertices)), m_indices(std::move(other.m_indices)),
      m_material(std::move(other.m_material)), m_VAO(std::move(other.m_VAO)), m_VBO(std::move(other.m_VBO)),
      m_EBO(std::move(other.m_EBO)), m_isSetup(other.m_isSetup) {
    other.m_isSetup = false;
}

Mesh &Mesh::operator=(Mesh &&other) noexcept {
    if (this != &other) {
        m_vertices = std::move(other.m_vertices);
        m_indices = std::move(other.m_indices);
        m_material = std::move(other.m_material);
        m_VAO = std::move(other.m_VAO);
        m_VBO = std::move(other.m_VBO);
        m_EBO = std::move(other.m_EBO);
        m_isSetup = other.m_isSetup;
        other.m_isSetup = false;
    }
    return *this;
}

// ========== Data Management ==========

void Mesh::SetVertices(const std::vector<Vertex> &vertices) {
    m_vertices = vertices;
    m_isSetup = false;
    SetupMesh();
}

void Mesh::SetIndices(const std::vector<uint32_t> &indices) {
    m_indices = indices;
    m_isSetup = false;
    SetupMesh();
}

void Mesh::SetMaterial(const Material &material) {
    m_material = material;
}

void Mesh::UpdateVertices(const std::vector<Vertex> &vertices) {
    if (vertices.size() != m_vertices.size()) {
        SetVertices(vertices);
        return;
    }

    m_vertices = vertices;

    if (m_VBO) {
        m_VBO->Bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
        m_VBO->Unbind();
    }
}

void Mesh::UpdateVertices(const std::vector<Vertex> &vertices, size_t offset) {
    if (offset + vertices.size() > m_vertices.size()) {
        return; // Invalid range
    }

    // Update local data
    std::copy(vertices.begin(), vertices.end(), m_vertices.begin() + offset);

    // Update GPU buffer
    if (m_VBO) {
        m_VBO->Bind();
        glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(Vertex), vertices.size() * sizeof(Vertex), vertices.data());
        m_VBO->Unbind();
    }
}

// ========== Rendering ==========

void Mesh::Render() {
    if (!m_isSetup || m_vertices.empty()) {
        return;
    }

    m_VAO->Bind();

    if (HasIndices()) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));
    }

    m_VAO->Unbind();
}

void Mesh::Render(ShaderProgram &shader) {
    if (!m_isSetup || m_vertices.empty()) {
        return;
    }

    shader.Use();
    BindMaterialTextures(shader);

    // Set material properties
    shader.SetUniform("material.ambient", m_material.ambient);
    shader.SetUniform("material.diffuse", m_material.diffuse);
    shader.SetUniform("material.specular", m_material.specular);
    shader.SetUniform("material.shininess", m_material.shininess);

    Render();
}

void Mesh::Render(ShaderProgram &shader, const glm::mat4 &modelMatrix) {
    if (!m_isSetup || m_vertices.empty()) {
        return;
    }

    shader.Use();
    shader.SetUniform("model", modelMatrix);

    // Calculate normal matrix
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
    shader.SetUniform("normalMatrix", normalMatrix);

    BindMaterialTextures(shader);

    // Set material properties
    shader.SetUniform("material.ambient", m_material.ambient);
    shader.SetUniform("material.diffuse", m_material.diffuse);
    shader.SetUniform("material.specular", m_material.specular);
    shader.SetUniform("material.shininess", m_material.shininess);

    Render();
}

// ========== Utility Functions ==========

void Mesh::CalculateNormals() {
    if (m_vertices.empty())
        return;

    // Reset all normals
    for (auto &vertex : m_vertices) {
        vertex.normal = glm::vec3(0.0f);
    }

    // Calculate normals based on triangles
    if (HasIndices()) {
        // Use indices
        for (size_t i = 0; i < m_indices.size(); i += 3) {
            if (i + 2 < m_indices.size()) {
                uint32_t i0 = m_indices[i];
                uint32_t i1 = m_indices[i + 1];
                uint32_t i2 = m_indices[i + 2];

                if (i0 < m_vertices.size() && i1 < m_vertices.size() && i2 < m_vertices.size()) {
                    glm::vec3 v0 = m_vertices[i0].position;
                    glm::vec3 v1 = m_vertices[i1].position;
                    glm::vec3 v2 = m_vertices[i2].position;

                    glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

                    m_vertices[i0].normal += normal;
                    m_vertices[i1].normal += normal;
                    m_vertices[i2].normal += normal;
                }
            }
        }
    } else {
        // Use vertex order
        for (size_t i = 0; i < m_vertices.size(); i += 3) {
            if (i + 2 < m_vertices.size()) {
                glm::vec3 v0 = m_vertices[i].position;
                glm::vec3 v1 = m_vertices[i + 1].position;
                glm::vec3 v2 = m_vertices[i + 2].position;

                glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

                m_vertices[i].normal += normal;
                m_vertices[i + 1].normal += normal;
                m_vertices[i + 2].normal += normal;
            }
        }
    }

    // Normalize all normals
    for (auto &vertex : m_vertices) {
        if (glm::length(vertex.normal) > 0.0f) {
            vertex.normal = glm::normalize(vertex.normal);
        }
    }

    // Update GPU buffer if already set up
    if (m_isSetup) {
        UpdateVertices(m_vertices);
    }
}

void Mesh::CalculateTangents() {
    if (m_vertices.empty())
        return;

    // Reset tangents and bitangents
    for (auto &vertex : m_vertices) {
        vertex.tangent = glm::vec3(0.0f);
        vertex.bitangent = glm::vec3(0.0f);
    }

    auto calculateTriangleTangents = [&](uint32_t i0, uint32_t i1, uint32_t i2) {
        if (i0 >= m_vertices.size() || i1 >= m_vertices.size() || i2 >= m_vertices.size()) {
            return;
        }

        glm::vec3 pos1 = m_vertices[i0].position;
        glm::vec3 pos2 = m_vertices[i1].position;
        glm::vec3 pos3 = m_vertices[i2].position;

        glm::vec2 uv1 = m_vertices[i0].texCoords;
        glm::vec2 uv2 = m_vertices[i1].texCoords;
        glm::vec2 uv3 = m_vertices[i2].texCoords;

        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        glm::vec3 tangent{f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
                          f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
                          f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)};

        glm::vec3 bitangent{f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x),
                            f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y),
                            f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z)};

        m_vertices[i0].tangent += tangent;
        m_vertices[i1].tangent += tangent;
        m_vertices[i2].tangent += tangent;

        m_vertices[i0].bitangent += bitangent;
        m_vertices[i1].bitangent += bitangent;
        m_vertices[i2].bitangent += bitangent;
    };

    // Calculate tangents for each triangle
    if (HasIndices()) {
        for (size_t i = 0; i < m_indices.size(); i += 3) {
            if (i + 2 < m_indices.size()) {
                calculateTriangleTangents(m_indices[i], m_indices[i + 1], m_indices[i + 2]);
            }
        }
    } else {
        for (size_t i = 0; i < m_vertices.size(); i += 3) {
            if (i + 2 < m_vertices.size()) {
                calculateTriangleTangents(static_cast<uint32_t>(i), static_cast<uint32_t>(i + 1),
                                          static_cast<uint32_t>(i + 2));
            }
        }
    }

    // Normalize tangents and bitangents
    for (auto &vertex : m_vertices) {
        if (glm::length(vertex.tangent) > 0.0f) {
            vertex.tangent = glm::normalize(vertex.tangent);
        }
        if (glm::length(vertex.bitangent) > 0.0f) {
            vertex.bitangent = glm::normalize(vertex.bitangent);
        }
    }

    // Update GPU buffer if already set up
    if (m_isSetup) {
        UpdateVertices(m_vertices);
    }
}

std::pair<glm::vec3, glm::vec3> Mesh::GetBoundingBox() const {
    if (m_vertices.empty()) {
        return {glm::vec3(0.0f), glm::vec3(0.0f)};
    }

    glm::vec3 min = m_vertices[0].position;
    glm::vec3 max = m_vertices[0].position;

    for (const auto &vertex : m_vertices) {
        min = glm::min(min, vertex.position);
        max = glm::max(max, vertex.position);
    }

    return {min, max};
}

glm::vec3 Mesh::GetCenter() const {
    auto [min, max] = GetBoundingBox();
    return (min + max) * 0.5f;
}

// ========== Static Primitive Creation ==========

Mesh Mesh::CreateTriangle() {
    std::vector<Vertex> vertices = {
        Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.5f, 1.0f))};

    return Mesh(vertices);
}

Mesh Mesh::CreateQuad() {
    std::vector<Vertex> vertices = {
        Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f))};

    std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

    return Mesh(vertices, indices);
}

Mesh Mesh::CreateCube() {
    std::vector<Vertex> vertices = {
        // Front face
        Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),

        // Back face
        Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)),

        // Left face
        Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)),

        // Right face
        Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)),

        // Bottom face
        Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),

        // Top face
        Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f))};

    std::vector<uint32_t> indices = {
        0,  1,  2,  2,  3,  0,  // front
        4,  5,  6,  6,  7,  4,  // back
        8,  9,  10, 10, 11, 8,  // left
        12, 13, 14, 14, 15, 12, // right
        16, 17, 18, 18, 19, 16, // bottom
        20, 21, 22, 22, 23, 20  // top
    };

    return Mesh(vertices, indices);
}

Mesh Mesh::CreateSphere(float radius, int segments, int rings) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Generate vertices
    for (int ring = 0; ring <= rings; ++ring) {
        float phi = M_PI * static_cast<float>(ring) / static_cast<float>(rings);
        float y = std::cos(phi);
        float ringRadius = std::sin(phi);

        for (int segment = 0; segment <= segments; ++segment) {
            float theta = 2.0f * M_PI * static_cast<float>(segment) / static_cast<float>(segments);
            float x = ringRadius * std::cos(theta);
            float z = ringRadius * std::sin(theta);

            Vertex vertex;
            vertex.position = glm::vec3(x, y, z) * radius;
            vertex.normal = glm::vec3(x, y, z);
            vertex.texCoords = glm::vec2(static_cast<float>(segment) / static_cast<float>(segments),
                                         static_cast<float>(ring) / static_cast<float>(rings));

            vertices.push_back(vertex);
        }
    }

    // Generate indices
    for (int ring = 0; ring < rings; ++ring) {
        for (int segment = 0; segment < segments; ++segment) {
            int current = ring * (segments + 1) + segment;
            int next = current + segments + 1;

            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);

            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }

    return Mesh(vertices, indices);
}

Mesh Mesh::CreatePlane(float width, float height, int widthSegments, int heightSegments) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Generate vertices
    for (int j = 0; j <= heightSegments; ++j) {
        float v = static_cast<float>(j) / static_cast<float>(heightSegments);
        float y = (v - 0.5f) * height;

        for (int i = 0; i <= widthSegments; ++i) {
            float u = static_cast<float>(i) / static_cast<float>(widthSegments);
            float x = (u - 0.5f) * width;

            Vertex vertex;
            vertex.position = glm::vec3(x, 0.0f, y);
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            vertex.texCoords = glm::vec2(u, v);

            vertices.push_back(vertex);
        }
    }

    // Generate indices
    for (int j = 0; j < heightSegments; ++j) {
        for (int i = 0; i < widthSegments; ++i) {
            int bottomLeft = j * (widthSegments + 1) + i;
            int bottomRight = bottomLeft + 1;
            int topLeft = bottomLeft + (widthSegments + 1);
            int topRight = topLeft + 1;

            // First triangle
            indices.push_back(bottomLeft);
            indices.push_back(topLeft);
            indices.push_back(bottomRight);

            // Second triangle
            indices.push_back(bottomRight);
            indices.push_back(topLeft);
            indices.push_back(topRight);
        }
    }

    return Mesh(vertices, indices);
}

Mesh Mesh::CreateCylinder(float radius, float height, int segments, int rings) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Generate vertices for the cylinder body
    for (int ring = 0; ring <= rings; ++ring) {
        float y = (static_cast<float>(ring) / static_cast<float>(rings) - 0.5f) * height;
        float v = static_cast<float>(ring) / static_cast<float>(rings);

        for (int segment = 0; segment <= segments; ++segment) {
            float theta = static_cast<float>(segment) / static_cast<float>(segments) * 2.0f * M_PI;
            float x = radius * cos(theta);
            float z = radius * sin(theta);
            float u = static_cast<float>(segment) / static_cast<float>(segments);

            Vertex vertex;
            vertex.position = glm::vec3(x, y, z);
            vertex.normal = glm::normalize(glm::vec3(x, 0.0f, z));
            vertex.texCoords = glm::vec2(u, v);

            vertices.push_back(vertex);
        }
    }

    // Generate indices for cylinder body
    for (int ring = 0; ring < rings; ++ring) {
        for (int segment = 0; segment < segments; ++segment) {
            int current = ring * (segments + 1) + segment;
            int next = current + segments + 1;

            // First triangle
            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);

            // Second triangle
            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }

    // Add top cap
    int topCenterIndex = vertices.size();
    Vertex topCenter;
    topCenter.position = glm::vec3(0.0f, height * 0.5f, 0.0f);
    topCenter.normal = glm::vec3(0.0f, 1.0f, 0.0f);
    topCenter.texCoords = glm::vec2(0.5f, 0.5f);
    vertices.push_back(topCenter);

    for (int segment = 0; segment < segments; ++segment) {
        float theta = static_cast<float>(segment) / static_cast<float>(segments) * 2.0f * M_PI;
        float x = radius * cos(theta);
        float z = radius * sin(theta);

        Vertex vertex;
        vertex.position = glm::vec3(x, height * 0.5f, z);
        vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        vertex.texCoords = glm::vec2(0.5f + 0.5f * cos(theta), 0.5f + 0.5f * sin(theta));

        vertices.push_back(vertex);
    }

    // Top cap indices
    for (int segment = 0; segment < segments; ++segment) {
        int current = topCenterIndex + 1 + segment;
        int next = topCenterIndex + 1 + ((segment + 1) % segments);

        indices.push_back(topCenterIndex);
        indices.push_back(current);
        indices.push_back(next);
    }

    // Add bottom cap
    int bottomCenterIndex = vertices.size();
    Vertex bottomCenter;
    bottomCenter.position = glm::vec3(0.0f, -height * 0.5f, 0.0f);
    bottomCenter.normal = glm::vec3(0.0f, -1.0f, 0.0f);
    bottomCenter.texCoords = glm::vec2(0.5f, 0.5f);
    vertices.push_back(bottomCenter);

    for (int segment = 0; segment < segments; ++segment) {
        float theta = static_cast<float>(segment) / static_cast<float>(segments) * 2.0f * M_PI;
        float x = radius * cos(theta);
        float z = radius * sin(theta);

        Vertex vertex;
        vertex.position = glm::vec3(x, -height * 0.5f, z);
        vertex.normal = glm::vec3(0.0f, -1.0f, 0.0f);
        vertex.texCoords = glm::vec2(0.5f + 0.5f * cos(theta), 0.5f - 0.5f * sin(theta));

        vertices.push_back(vertex);
    }

    // Bottom cap indices
    for (int segment = 0; segment < segments; ++segment) {
        int current = bottomCenterIndex + 1 + segment;
        int next = bottomCenterIndex + 1 + ((segment + 1) % segments);

        indices.push_back(bottomCenterIndex);
        indices.push_back(next);
        indices.push_back(current);
    }

    return Mesh(vertices, indices);
}

Mesh Mesh::CreateCapsule(float radius, float height, int segments, int rings) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Create the cylindrical body (similar to cylinder but without caps)
    int bodyRings = std::max(1, rings);
    for (int ring = 0; ring <= bodyRings; ++ring) {
        float y = (static_cast<float>(ring) / static_cast<float>(bodyRings) - 0.5f) * height;
        float v = static_cast<float>(ring) / static_cast<float>(bodyRings);

        for (int segment = 0; segment <= segments; ++segment) {
            float theta = static_cast<float>(segment) / static_cast<float>(segments) * 2.0f * M_PI;
            float x = radius * cos(theta);
            float z = radius * sin(theta);
            float u = static_cast<float>(segment) / static_cast<float>(segments);

            Vertex vertex;
            vertex.position = glm::vec3(x, y, z);
            vertex.normal = glm::normalize(glm::vec3(x, 0.0f, z));
            vertex.texCoords = glm::vec2(u, 0.5f + v * 0.3f); // Scale V to leave room for hemispheres

            vertices.push_back(vertex);
        }
    }

    // Generate indices for cylinder body
    for (int ring = 0; ring < bodyRings; ++ring) {
        for (int segment = 0; segment < segments; ++segment) {
            int current = ring * (segments + 1) + segment;
            int next = current + segments + 1;

            // First triangle
            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);

            // Second triangle
            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }

    // Add top hemisphere
    int topHemisphereStart = vertices.size();
    int hemisphereRings = std::max(4, rings / 2);

    for (int ring = 1; ring <= hemisphereRings; ++ring) {
        float phi = static_cast<float>(ring) / static_cast<float>(hemisphereRings) * M_PI * 0.5f;
        float y = height * 0.5f + radius * sin(phi);
        float ringRadius = radius * cos(phi);
        float v = 0.8f + 0.2f * static_cast<float>(ring) / static_cast<float>(hemisphereRings);

        for (int segment = 0; segment <= segments; ++segment) {
            float theta = static_cast<float>(segment) / static_cast<float>(segments) * 2.0f * M_PI;
            float x = ringRadius * cos(theta);
            float z = ringRadius * sin(theta);
            float u = static_cast<float>(segment) / static_cast<float>(segments);

            Vertex vertex;
            vertex.position = glm::vec3(x, y, z);
            vertex.normal = glm::normalize(glm::vec3(x, y - height * 0.5f, z));
            vertex.texCoords = glm::vec2(u, v);

            vertices.push_back(vertex);
        }
    }

    // Connect body to top hemisphere
    int bodyTopRing = bodyRings * (segments + 1);
    int hemisphereFirstRing = topHemisphereStart;
    for (int segment = 0; segment < segments; ++segment) {
        int bodyIdx = bodyTopRing + segment;
        int hemisphereIdx = hemisphereFirstRing + segment;

        indices.push_back(bodyIdx);
        indices.push_back(hemisphereIdx);
        indices.push_back(bodyIdx + 1);

        indices.push_back(bodyIdx + 1);
        indices.push_back(hemisphereIdx);
        indices.push_back(hemisphereIdx + 1);
    }

    // Generate indices for top hemisphere
    for (int ring = 0; ring < hemisphereRings - 1; ++ring) {
        for (int segment = 0; segment < segments; ++segment) {
            int current = topHemisphereStart + ring * (segments + 1) + segment;
            int next = current + segments + 1;

            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);

            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }

    // Add bottom hemisphere
    int bottomHemisphereStart = vertices.size();

    for (int ring = 1; ring <= hemisphereRings; ++ring) {
        float phi = static_cast<float>(ring) / static_cast<float>(hemisphereRings) * M_PI * 0.5f;
        float y = -height * 0.5f - radius * sin(phi);
        float ringRadius = radius * cos(phi);
        float v = 0.2f - 0.2f * static_cast<float>(ring) / static_cast<float>(hemisphereRings);

        for (int segment = 0; segment <= segments; ++segment) {
            float theta = static_cast<float>(segment) / static_cast<float>(segments) * 2.0f * M_PI;
            float x = ringRadius * cos(theta);
            float z = ringRadius * sin(theta);
            float u = static_cast<float>(segment) / static_cast<float>(segments);

            Vertex vertex;
            vertex.position = glm::vec3(x, y, z);
            vertex.normal = glm::normalize(glm::vec3(x, y + height * 0.5f, z));
            vertex.texCoords = glm::vec2(u, v);

            vertices.push_back(vertex);
        }
    }

    // Connect body to bottom hemisphere
    int bodyBottomRing = 0;
    int bottomHemisphereFirstRing = bottomHemisphereStart;
    for (int segment = 0; segment < segments; ++segment) {
        int bodyIdx = bodyBottomRing + segment;
        int hemisphereIdx = bottomHemisphereFirstRing + segment;

        indices.push_back(bodyIdx);
        indices.push_back(bodyIdx + 1);
        indices.push_back(hemisphereIdx);

        indices.push_back(bodyIdx + 1);
        indices.push_back(hemisphereIdx + 1);
        indices.push_back(hemisphereIdx);
    }

    // Generate indices for bottom hemisphere
    for (int ring = 0; ring < hemisphereRings - 1; ++ring) {
        for (int segment = 0; segment < segments; ++segment) {
            int current = bottomHemisphereStart + ring * (segments + 1) + segment;
            int next = current + segments + 1;

            indices.push_back(current);
            indices.push_back(current + 1);
            indices.push_back(next);

            indices.push_back(current + 1);
            indices.push_back(next + 1);
            indices.push_back(next);
        }
    }

    return Mesh(vertices, indices);
}

Mesh Mesh::CreateBullet(float radius, float length) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    const int segments = 8; // Lower detail for performance
    const float halfLength = length * 0.5f;

    // Create cylindrical body
    for (int i = 0; i <= segments; ++i) {
        float theta = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * M_PI;
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        float u = static_cast<float>(i) / static_cast<float>(segments);

        // Back vertex
        Vertex backVertex;
        backVertex.position = glm::vec3(x, -halfLength, z);
        backVertex.normal = glm::normalize(glm::vec3(x, 0.0f, z));
        backVertex.texCoords = glm::vec2(u, 0.0f);
        vertices.push_back(backVertex);

        // Front vertex
        Vertex frontVertex;
        frontVertex.position = glm::vec3(x, halfLength, z);
        frontVertex.normal = glm::normalize(glm::vec3(x, 0.0f, z));
        frontVertex.texCoords = glm::vec2(u, 1.0f);
        vertices.push_back(frontVertex);
    }

    // Generate indices for cylinder body
    for (int i = 0; i < segments; ++i) {
        int current = i * 2;
        int next = ((i + 1) % (segments + 1)) * 2;

        // First triangle
        indices.push_back(current);
        indices.push_back(next);
        indices.push_back(current + 1);

        // Second triangle
        indices.push_back(current + 1);
        indices.push_back(next);
        indices.push_back(next + 1);
    }

    // Add pointed tip
    int tipIndex = vertices.size();
    Vertex tip;
    tip.position = glm::vec3(0.0f, halfLength + radius, 0.0f);
    tip.normal = glm::vec3(0.0f, 1.0f, 0.0f);
    tip.texCoords = glm::vec2(0.5f, 1.0f);
    vertices.push_back(tip);

    // Connect tip to front ring
    for (int i = 0; i < segments; ++i) {
        int current = i * 2 + 1;
        int next = ((i + 1) % (segments + 1)) * 2 + 1;

        indices.push_back(tipIndex);
        indices.push_back(current);
        indices.push_back(next);
    }

    return Mesh(vertices, indices);
}

Mesh Mesh::CreateProjectile(ProjectileType type, float scale) {
    switch (type) {
    case ProjectileType::Bullet:
        return CreateBullet(0.05f * scale, 0.2f * scale);
    case ProjectileType::Missile:
        return CreateCylinder(0.1f * scale, 0.5f * scale, 8, 1);
    case ProjectileType::Laser:
        return CreateCylinder(0.02f * scale, 1.0f * scale, 6, 1);
    case ProjectileType::Plasma:
        return CreateSphere(0.15f * scale, 8, 6);
    default:
        return CreateSphere(0.1f * scale, 6, 4);
    }
}

// ========== Private Methods ==========

void Mesh::SetupMesh() {
    if (m_vertices.empty()) {
        return;
    }

    // Create vertex buffer
    m_VBO = std::make_shared<VertexBuffer>(m_vertices.data(), m_vertices.size() * sizeof(Vertex));

    // Create vertex buffer layout
    VertexBufferLayout layout;
    layout.PushFloat("a_Position", 3);  // position
    layout.PushFloat("a_Normal", 3);    // normal
    layout.PushFloat("a_TexCoords", 2); // texCoords
    layout.PushFloat("a_Tangent", 3);   // tangent
    layout.PushFloat("a_Bitangent", 3); // bitangent

    // Add vertex buffer to VAO
    m_VAO->AddVertexBuffer(m_VBO, layout);

    // Create index buffer if we have indices
    if (!m_indices.empty()) {
        m_EBO = std::make_shared<IndexBuffer>(m_indices.data(), m_indices.size());
        m_VAO->SetIndexBuffer(m_EBO);
    }

    m_isSetup = true;
}

void Mesh::BindMaterialTextures(ShaderProgram &shader) {
    int textureUnit = 0;

    if (m_material.diffuseTexture) {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        m_material.diffuseTexture->Bind();
        shader.SetUniform("material.diffuseTexture", textureUnit);
        shader.SetUniform("material.hasDiffuseTexture", true);
        textureUnit++;
    } else {
        shader.SetUniform("material.hasDiffuseTexture", false);
    }

    if (m_material.specularTexture) {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        m_material.specularTexture->Bind();
        shader.SetUniform("material.specularTexture", textureUnit);
        shader.SetUniform("material.hasSpecularTexture", true);
        textureUnit++;
    } else {
        shader.SetUniform("material.hasSpecularTexture", false);
    }

    if (m_material.normalTexture) {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        m_material.normalTexture->Bind();
        shader.SetUniform("material.normalTexture", textureUnit);
        shader.SetUniform("material.hasNormalTexture", true);
        textureUnit++;
    } else {
        shader.SetUniform("material.hasNormalTexture", false);
    }
}

} // namespace agl
