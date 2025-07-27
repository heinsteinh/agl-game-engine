#ifndef MESH_H
#define MESH_H

#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "buffer.h"

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace agl {

// Projectile types for the shooter system
enum class ProjectileType {
    Bullet,  // Small, fast projectile
    Missile, // Medium-sized explosive projectile
    Laser,   // Thin, energy-based projectile
    Plasma,  // Spherical energy projectile
    Default  // Generic projectile
};

// Vertex data structure for standard mesh rendering
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;

    // Default constructor
    Vertex() = default;

    // Constructor for position only
    Vertex(const glm::vec3 &pos) : position(pos), normal(0.0f), texCoords(0.0f), tangent(0.0f), bitangent(0.0f) {}

    // Constructor for position and texture coordinates
    Vertex(const glm::vec3 &pos, const glm::vec2 &uv)
        : position(pos), normal(0.0f), texCoords(uv), tangent(0.0f), bitangent(0.0f) {}

    // Full constructor
    Vertex(const glm::vec3 &pos, const glm::vec3 &norm, const glm::vec2 &uv)
        : position(pos), normal(norm), texCoords(uv), tangent(0.0f), bitangent(0.0f) {}
};

// Material properties for mesh rendering
struct Material {
    glm::vec3 ambient{0.1f, 0.1f, 0.1f};
    glm::vec3 diffuse{0.8f, 0.8f, 0.8f};
    glm::vec3 specular{1.0f, 1.0f, 1.0f};
    float shininess{32.0f};

    // Texture pointers (optional)
    std::shared_ptr<Texture> diffuseTexture;
    std::shared_ptr<Texture> specularTexture;
    std::shared_ptr<Texture> normalTexture;

    Material() = default;

    Material(const glm::vec3 &diff) : diffuse(diff) {}

    Material(const glm::vec3 &diff, const glm::vec3 &spec, float shine)
        : diffuse(diff), specular(spec), shininess(shine) {}
};

/**
 * @brief A mesh represents a collection of vertices and indices that can be rendered
 *
 * The Mesh class encapsulates vertex data, index data, and material properties
 * for 3D geometry. It handles the creation and management of OpenGL buffer objects
 * and provides methods for rendering with different shaders and materials.
 */
class Mesh {
public:
    // ========== Constructors ==========

    /**
     * @brief Default constructor - creates an empty mesh
     */
    Mesh();

    /**
     * @brief Constructor with vertex and index data
     * @param vertices Vector of vertex data
     * @param indices Vector of index data (optional)
     */
    Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices = {});

    /**
     * @brief Constructor with vertex data, indices, and material
     * @param vertices Vector of vertex data
     * @param indices Vector of index data
     * @param material Material properties for rendering
     */
    Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, const Material &material);

    /**
     * @brief Destructor
     */
    ~Mesh();

    // Non-copyable but movable
    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;
    Mesh(Mesh &&other) noexcept;
    Mesh &operator=(Mesh &&other) noexcept;

    // ========== Data Management ==========

    /**
     * @brief Set the vertex data for this mesh
     * @param vertices Vector of vertex data
     */
    void SetVertices(const std::vector<Vertex> &vertices);

    /**
     * @brief Set the index data for this mesh
     * @param indices Vector of index data
     */
    void SetIndices(const std::vector<uint32_t> &indices);

    /**
     * @brief Set the material for this mesh
     * @param material Material properties
     */
    void SetMaterial(const Material &material);

    /**
     * @brief Update vertex data (for dynamic meshes)
     * @param vertices New vertex data
     */
    void UpdateVertices(const std::vector<Vertex> &vertices);

    /**
     * @brief Update a specific range of vertices
     * @param vertices New vertex data
     * @param offset Starting offset in the vertex buffer
     */
    void UpdateVertices(const std::vector<Vertex> &vertices, size_t offset);

    // ========== Rendering ==========

    /**
     * @brief Render the mesh with the currently bound shader
     */
    void Render();

    /**
     * @brief Render the mesh with a specific shader
     * @param shader Shader to use for rendering
     */
    void Render(ShaderProgram &shader);

    /**
     * @brief Render the mesh with a shader and model matrix
     * @param shader Shader to use for rendering
     * @param modelMatrix Model transformation matrix
     */
    void Render(ShaderProgram &shader, const glm::mat4 &modelMatrix);

    // ========== Utility Functions ==========

    /**
     * @brief Calculate normals for the mesh (if not provided)
     */
    void CalculateNormals();

    /**
     * @brief Calculate tangents and bitangents for normal mapping
     */
    void CalculateTangents();

    /**
     * @brief Get the bounding box of the mesh
     * @return Pair of min and max points
     */
    std::pair<glm::vec3, glm::vec3> GetBoundingBox() const;

    /**
     * @brief Get the center point of the mesh
     * @return Center point
     */
    glm::vec3 GetCenter() const;

    // ========== Static Primitive Creation ==========

    /**
     * @brief Create a triangle mesh
     * @return Triangle mesh
     */
    static Mesh CreateTriangle();

    /**
     * @brief Create a quad mesh
     * @return Quad mesh
     */
    static Mesh CreateQuad();

    /**
     * @brief Create a cube mesh
     * @return Cube mesh
     */
    static Mesh CreateCube();

    /**
     * @brief Create a sphere mesh
     * @param radius Sphere radius
     * @param segments Number of horizontal segments
     * @param rings Number of vertical rings
     * @return Sphere mesh
     */
    static Mesh CreateSphere(float radius = 1.0f, int segments = 32, int rings = 16);

    /**
     * @brief Create a plane mesh
     * @param width Plane width
     * @param height Plane height
     * @param widthSegments Number of width segments
     * @param heightSegments Number of height segments
     * @return Plane mesh
     */
    static Mesh CreatePlane(float width = 1.0f, float height = 1.0f, int widthSegments = 1, int heightSegments = 1);

    /**
     * @brief Create a cylinder mesh
     * @param radius Cylinder radius
     * @param height Cylinder height
     * @param segments Number of radial segments
     * @param rings Number of height rings
     * @return Cylinder mesh
     */
    static Mesh CreateCylinder(float radius = 1.0f, float height = 2.0f, int segments = 32, int rings = 1);

    /**
     * @brief Create a capsule mesh (cylinder with hemispherical ends)
     * @param radius Capsule radius
     * @param height Capsule cylindrical height (not including hemispheres)
     * @param segments Number of radial segments
     * @param rings Number of height rings for the cylindrical part
     * @return Capsule mesh
     */
    static Mesh CreateCapsule(float radius = 1.0f, float height = 2.0f, int segments = 32, int rings = 4);

    // ========== Projectile/Shooter System ==========

    /**
     * @brief Create a bullet mesh (small cylindrical projectile with pointed tip)
     * @param radius Bullet radius
     * @param length Bullet length
     * @return Bullet mesh
     */
    static Mesh CreateBullet(float radius = 0.05f, float length = 0.2f);

    /**
     * @brief Create a projectile mesh based on type
     * @param type Type of projectile to create
     * @param scale Scale factor for the projectile
     * @return Projectile mesh
     */
    static Mesh CreateProjectile(ProjectileType type = ProjectileType::Default, float scale = 1.0f);

    // ========== Getters ==========

    /**
     * @brief Get the vertex count
     * @return Number of vertices
     */
    size_t GetVertexCount() const {
        return m_vertices.size();
    }

    /**
     * @brief Get the triangle count
     * @return Number of triangles
     */
    size_t GetTriangleCount() const {
        return m_indices.size() / 3;
    }

    /**
     * @brief Get the index count
     * @return Number of indices
     */
    size_t GetIndexCount() const {
        return m_indices.size();
    }

    /**
     * @brief Check if the mesh has indices
     * @return True if mesh has indices
     */
    bool HasIndices() const {
        return !m_indices.empty();
    }

    /**
     * @brief Get the material
     * @return Reference to the material
     */
    const Material &GetMaterial() const {
        return m_material;
    }

    /**
     * @brief Get mutable reference to the material
     * @return Mutable reference to the material
     */
    Material &GetMaterial() {
        return m_material;
    }

    /**
     * @brief Check if the mesh is valid (has vertices)
     * @return True if mesh is valid
     */
    bool IsValid() const {
        return !m_vertices.empty();
    }

private:
    // ========== Private Methods ==========

    /**
     * @brief Set up the vertex array object and buffers
     */
    void SetupMesh();

    /**
     * @brief Bind material textures to shader uniforms
     * @param shader Shader to bind textures to
     */
    void BindMaterialTextures(ShaderProgram &shader);

    // ========== Data Members ==========

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    Material m_material;

    // OpenGL objects
    std::unique_ptr<VertexArray> m_VAO;
    std::shared_ptr<VertexBuffer> m_VBO;
    std::shared_ptr<IndexBuffer> m_EBO;

    // State
    bool m_isSetup{false};
};

} // namespace agl

#endif // MESH_H
