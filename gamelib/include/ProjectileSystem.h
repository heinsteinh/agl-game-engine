#ifndef PROJECTILE_SYSTEM_H
#define PROJECTILE_SYSTEM_H

#include "Shader.h"
#include "mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>

namespace agl {

/**
 * @brief Individual projectile data
 */
struct Projectile {
    glm::vec3 position{0.0f};
    glm::vec3 velocity{0.0f};
    glm::vec3 direction{0.0f, 1.0f, 0.0f};
    ProjectileType type{ProjectileType::Default};
    float speed{10.0f};
    float lifetime{5.0f};
    float maxLifetime{5.0f};
    float scale{1.0f};
    bool active{true};

    // Optional rotation for visual effects
    float rotation{0.0f};
    float rotationSpeed{0.0f};

    Projectile() = default;

    Projectile(const glm::vec3 &pos, const glm::vec3 &dir, ProjectileType projType = ProjectileType::Default)
        : position(pos), direction(glm::normalize(dir)), type(projType) {
        velocity = direction * speed;
    }
};

/**
 * @brief Projectile system for managing bullets, missiles, etc. in games
 */
class ProjectileSystem {
public:
    /**
     * @brief Constructor
     */
    ProjectileSystem();

    /**
     * @brief Destructor
     */
    ~ProjectileSystem() = default;

    /**
     * @brief Initialize the projectile system
     * @param maxProjectiles Maximum number of projectiles to manage
     */
    void Initialize(size_t maxProjectiles = 1000);

    /**
     * @brief Update all active projectiles
     * @param deltaTime Time elapsed since last update
     */
    void Update(float deltaTime);

    /**
     * @brief Render all active projectiles
     * @param shader Shader to use for rendering
     * @param viewMatrix Camera view matrix
     * @param projectionMatrix Camera projection matrix
     */
    void Render(ShaderProgram &shader, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix);

    /**
     * @brief Fire a projectile
     * @param position Starting position
     * @param direction Firing direction (will be normalized)
     * @param type Type of projectile
     * @param speed Projectile speed
     * @param lifetime How long the projectile lives (seconds)
     * @return True if projectile was fired, false if system is full
     */
    bool FireProjectile(const glm::vec3 &position, const glm::vec3 &direction,
                        ProjectileType type = ProjectileType::Default, float speed = 10.0f, float lifetime = 5.0f);

    /**
     * @brief Get all active projectiles (for collision detection)
     * @return Vector of active projectiles
     */
    const std::vector<Projectile> &GetActiveProjectiles() const {
        return m_projectiles;
    }

    /**
     * @brief Remove projectile at index
     * @param index Index of projectile to remove
     */
    void RemoveProjectile(size_t index);

    /**
     * @brief Clear all projectiles
     */
    void ClearAll();

    /**
     * @brief Get number of active projectiles
     * @return Number of active projectiles
     */
    size_t GetActiveCount() const;

    /**
     * @brief Get maximum number of projectiles
     * @return Maximum projectile capacity
     */
    size_t GetMaxProjectiles() const {
        return m_maxProjectiles;
    }

    /**
     * @brief Set default projectile properties
     */
    void SetDefaultSpeed(float speed) {
        m_defaultSpeed = speed;
    }
    void SetDefaultLifetime(float lifetime) {
        m_defaultLifetime = lifetime;
    }
    void SetDefaultScale(float scale) {
        m_defaultScale = scale;
    }

private:
    std::vector<Projectile> m_projectiles;
    std::vector<std::unique_ptr<Mesh>> m_projectileMeshes;

    size_t m_maxProjectiles{1000};
    size_t m_nextIndex{0};

    // Default properties
    float m_defaultSpeed{10.0f};
    float m_defaultLifetime{5.0f};
    float m_defaultScale{1.0f};

    /**
     * @brief Create meshes for different projectile types
     */
    void CreateProjectileMeshes();

    /**
     * @brief Get mesh for projectile type
     * @param type Projectile type
     * @return Pointer to mesh
     */
    Mesh *GetMeshForType(ProjectileType type);
};

/**
 * @brief Simple shooter controller for adding shooting capability to objects
 */
class Shooter {
public:
    Shooter(ProjectileSystem *projectileSystem = nullptr) : m_projectileSystem(projectileSystem) {}

    /**
     * @brief Set the projectile system to use
     */
    void SetProjectileSystem(ProjectileSystem *system) {
        m_projectileSystem = system;
    }

    /**
     * @brief Fire a projectile from this shooter
     * @param position Shooter position
     * @param direction Firing direction
     * @param type Type of projectile to fire
     * @return True if fired successfully
     */
    bool Fire(const glm::vec3 &position, const glm::vec3 &direction, ProjectileType type = ProjectileType::Default);

    /**
     * @brief Update shooter (handles cooldown, etc.)
     * @param deltaTime Time elapsed since last update
     */
    void Update(float deltaTime);

    /**
     * @brief Check if shooter can fire
     * @return True if can fire
     */
    bool CanFire() const;

    // Shooter properties
    float fireRate{5.0f};           // Shots per second
    float projectileSpeed{15.0f};   // Speed of fired projectiles
    float projectileLifetime{3.0f}; // Lifetime of fired projectiles
    ProjectileType defaultProjectileType{ProjectileType::Default};

private:
    ProjectileSystem *m_projectileSystem{nullptr};
    float m_lastFireTime{0.0f};
    float m_cooldownTime{0.0f};
};

} // namespace agl

#endif // PROJECTILE_SYSTEM_H
