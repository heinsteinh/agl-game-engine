#include "ProjectileSystem.h"
#include <algorithm>
#include <iostream>

#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

namespace agl {

// ========== ProjectileSystem Implementation ==========

ProjectileSystem::ProjectileSystem() {
    m_projectiles.reserve(m_maxProjectiles);
}

void ProjectileSystem::Initialize(size_t maxProjectiles) {
    m_maxProjectiles = maxProjectiles;
    m_projectiles.clear();
    m_projectiles.reserve(maxProjectiles);

    // Create meshes for different projectile types
    CreateProjectileMeshes();

    std::cout << "ProjectileSystem initialized with " << maxProjectiles << " max projectiles" << std::endl;
}

void ProjectileSystem::CreateProjectileMeshes() {
    m_projectileMeshes.clear();

    // Create one mesh for each projectile type
    for (int i = 0; i < static_cast<int>(ProjectileType::Default) + 1; ++i) {
        ProjectileType type = static_cast<ProjectileType>(i);
        auto mesh = std::make_unique<Mesh>(Mesh::CreateProjectile(type, m_defaultScale));

        // Set appropriate materials for different projectile types
        Material material;
        switch (type) {
        case ProjectileType::Bullet:
            material.diffuse = glm::vec3(0.8f, 0.6f, 0.2f); // Brass color
            material.specular = glm::vec3(0.9f, 0.7f, 0.3f);
            material.shininess = 64.0f;
            break;
        case ProjectileType::Missile:
            material.diffuse = glm::vec3(0.3f, 0.3f, 0.3f); // Dark gray
            material.specular = glm::vec3(0.5f, 0.5f, 0.5f);
            material.shininess = 32.0f;
            break;
        case ProjectileType::Laser:
            material.diffuse = glm::vec3(1.0f, 0.1f, 0.1f); // Bright red
            material.specular = glm::vec3(1.0f, 0.5f, 0.5f);
            material.shininess = 128.0f;
            break;
        case ProjectileType::Plasma:
            material.diffuse = glm::vec3(0.1f, 0.8f, 1.0f); // Bright cyan
            material.specular = glm::vec3(0.5f, 0.9f, 1.0f);
            material.shininess = 256.0f;
            break;
        default:
            material.diffuse = glm::vec3(0.7f, 0.7f, 0.7f); // Light gray
            material.specular = glm::vec3(0.8f, 0.8f, 0.8f);
            material.shininess = 32.0f;
            break;
        }

        mesh->SetMaterial(material);
        m_projectileMeshes.push_back(std::move(mesh));
    }
}

void ProjectileSystem::Update(float deltaTime) {
    // Update all active projectiles
    for (auto it = m_projectiles.begin(); it != m_projectiles.end();) {
        Projectile &projectile = *it;

        if (!projectile.active) {
            it = m_projectiles.erase(it);
            continue;
        }

        // Update position
        projectile.position += projectile.velocity * deltaTime;

        // Update rotation for visual effect
        projectile.rotation += projectile.rotationSpeed * deltaTime;

        // Update lifetime
        projectile.lifetime -= deltaTime;
        if (projectile.lifetime <= 0.0f) {
            projectile.active = false;
            it = m_projectiles.erase(it);
            continue;
        }

        ++it;
    }
}

void ProjectileSystem::Render(ShaderProgram &shader, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) {
    shader.Use();
    shader.SetUniform("view", viewMatrix);
    shader.SetUniform("projection", projectionMatrix);

    // Set lighting (simple setup for projectiles)
    shader.SetUniform("lightPos", glm::vec3(0.0f, 10.0f, 0.0f));
    shader.SetUniform("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.SetUniform("viewPos", glm::vec3(0.0f, 0.0f, 5.0f)); // This should come from camera

    for (const Projectile &projectile : m_projectiles) {
        if (!projectile.active)
            continue;

        // Create model matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, projectile.position);

        // Rotate to face direction
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(up, projectile.direction));
        up = glm::normalize(glm::cross(projectile.direction, right));

        glm::mat3 rotation = glm::mat3(right, up, projectile.direction);
        model = model * glm::mat4(rotation);

        // Apply additional rotation for visual effects
        if (projectile.rotationSpeed != 0.0f) {
            model = glm::rotate(model, projectile.rotation, glm::vec3(0.0f, 1.0f, 0.0f));
        }

        // Apply scale
        model = glm::scale(model, glm::vec3(projectile.scale));

        // Get appropriate mesh and render
        Mesh *mesh = GetMeshForType(projectile.type);
        if (mesh) {
            mesh->Render(shader, model);
        }
    }
}

bool ProjectileSystem::FireProjectile(const glm::vec3 &position, const glm::vec3 &direction, ProjectileType type,
                                      float speed, float lifetime) {
    if (m_projectiles.size() >= m_maxProjectiles) {
        return false; // System is full
    }

    Projectile projectile(position, direction, type);
    projectile.speed = speed;
    projectile.velocity = glm::normalize(direction) * speed;
    projectile.lifetime = lifetime;
    projectile.maxLifetime = lifetime;
    projectile.scale = m_defaultScale;

    // Add some rotation for visual effect (except for laser)
    if (type != ProjectileType::Laser) {
        projectile.rotationSpeed = 5.0f; // radians per second
    }

    m_projectiles.push_back(projectile);
    return true;
}

void ProjectileSystem::RemoveProjectile(size_t index) {
    if (index < m_projectiles.size()) {
        m_projectiles.erase(m_projectiles.begin() + index);
    }
}

void ProjectileSystem::ClearAll() {
    m_projectiles.clear();
}

size_t ProjectileSystem::GetActiveCount() const {
    return m_projectiles.size();
}

Mesh *ProjectileSystem::GetMeshForType(ProjectileType type) {
    int index = static_cast<int>(type);
    if (index >= 0 && index < static_cast<int>(m_projectileMeshes.size())) {
        return m_projectileMeshes[index].get();
    }

    // Return default mesh if type is invalid
    if (!m_projectileMeshes.empty()) {
        return m_projectileMeshes.back().get(); // Default is last
    }

    return nullptr;
}

// ========== Shooter Implementation ==========

bool Shooter::Fire(const glm::vec3 &position, const glm::vec3 &direction, ProjectileType type) {
    if (!CanFire() || !m_projectileSystem) {
        return false;
    }

    bool success = m_projectileSystem->FireProjectile(position, direction, type, projectileSpeed, projectileLifetime);

    if (success) {
        m_cooldownTime = 1.0f / fireRate; // Set cooldown based on fire rate
        m_lastFireTime = 0.0f;            // This should be actual game time
    }

    return success;
}

void Shooter::Update(float deltaTime) {
    if (m_cooldownTime > 0.0f) {
        m_cooldownTime -= deltaTime;
        if (m_cooldownTime < 0.0f) {
            m_cooldownTime = 0.0f;
        }
    }

    m_lastFireTime += deltaTime;
}

bool Shooter::CanFire() const {
    return m_cooldownTime <= 0.0f;
}

} // namespace agl
