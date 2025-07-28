#ifndef SHADOW_SYSTEM_H
#define SHADOW_SYSTEM_H

#include "Shader.h"
#include "mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>

#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

namespace agl {

/**
 * @brief Light types for shadow casting
 */
enum class LightType {
    Directional, // Sun-like light (parallel rays)
    Point,       // Point light source
    Spot         // Spotlight with cone
};

/**
 * @brief Light structure for shadow mapping
 */
struct Light {
    LightType type = LightType::Directional;
    glm::vec3 position{0.0f, 10.0f, 0.0f};
    glm::vec3 direction{0.0f, -1.0f, 0.0f};
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;

    // For spot lights
    float cutoff = 45.0f;
    float outerCutoff = 55.0f;

    // Attenuation for point/spot lights
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    Light() = default;
    Light(LightType t, const glm::vec3 &pos, const glm::vec3 &dir = glm::vec3(0.0f, -1.0f, 0.0f))
        : type(t), position(pos), direction(glm::normalize(dir)) {}
};

/**
 * @brief Shadow mapping system for real-time shadows
 */
class ShadowSystem {
public:
    /**
     * @brief Constructor
     * @param shadowMapSize Size of the shadow map texture (default 2048x2048)
     */
    ShadowSystem(int shadowMapSize = 2048);

    /**
     * @brief Destructor
     */
    ~ShadowSystem();

    // Non-copyable but movable
    ShadowSystem(const ShadowSystem &) = delete;
    ShadowSystem &operator=(const ShadowSystem &) = delete;
    ShadowSystem(ShadowSystem &&other) noexcept;
    ShadowSystem &operator=(ShadowSystem &&other) noexcept;

    /**
     * @brief Initialize the shadow system
     * @return True if initialization succeeded
     */
    bool Initialize();

    /**
     * @brief Set the main light for shadow casting
     * @param light The light to cast shadows
     */
    void SetLight(const Light &light);

    /**
     * @brief Begin shadow map rendering phase
     * Call this before rendering shadow casters
     */
    void BeginShadowPass();

    /**
     * @brief End shadow map rendering phase
     */
    void EndShadowPass();

    /**
     * @brief Begin main rendering phase with shadows
     * Call this before rendering the main scene
     * @param viewMatrix Camera view matrix
     * @param projectionMatrix Camera projection matrix
     */
    void BeginMainPass(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix);

    /**
     * @brief Render a mesh during shadow pass
     * @param mesh Mesh to render
     * @param modelMatrix Model transformation matrix
     */
    void RenderShadowCaster(Mesh &mesh, const glm::mat4 &modelMatrix);

    /**
     * @brief Render a mesh during main pass with shadows
     * @param mesh Mesh to render
     * @param modelMatrix Model transformation matrix
     */
    void RenderWithShadows(Mesh &mesh, const glm::mat4 &modelMatrix);

    /**
     * @brief Get the light-space matrix for manual shader setup
     * @return Light-space transformation matrix
     */
    glm::mat4 GetLightSpaceMatrix() const {
        return m_lightSpaceMatrix;
    }

    /**
     * @brief Get the shadow map texture ID for manual binding
     * @return OpenGL texture ID
     */
    GLuint GetShadowMapTexture() const {
        return m_shadowMap;
    }

    /**
     * @brief Set shadow bias to reduce shadow acne
     * @param bias Shadow bias value (default 0.005f)
     */
    void SetShadowBias(float bias) {
        m_shadowBias = bias;
    }

    /**
     * @brief Enable/disable PCF (Percentage Closer Filtering) for softer shadows
     * @param enable True to enable PCF
     */
    void SetPCFEnabled(bool enable) {
        m_pcfEnabled = enable;
    }

    /**
     * @brief Set the size of the orthographic projection for directional lights
     * @param size Half-size of the orthographic box
     */
    void SetOrthographicSize(float size) {
        m_orthographicSize = size;
    }

private:
    // Shadow map properties
    int m_shadowMapSize;
    GLuint m_shadowMap;
    GLuint m_shadowMapFBO;

    // Shaders
    std::unique_ptr<ShaderProgram> m_shadowShader;
    std::unique_ptr<ShaderProgram> m_shadowedShader;

    // Light and matrices
    Light m_light;
    glm::mat4 m_lightSpaceMatrix;
    glm::mat4 m_lightProjection;
    glm::mat4 m_lightView;

    // Shadow parameters
    float m_shadowBias = 0.005f;
    bool m_pcfEnabled = true;
    float m_orthographicSize = 20.0f;

    // Camera matrices for main pass
    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;

    /**
     * @brief Create shadow map framebuffer
     * @return True if creation succeeded
     */
    bool CreateShadowMap();

    /**
     * @brief Create shadow mapping shaders
     * @return True if creation succeeded
     */
    bool CreateShaders();

    /**
     * @brief Calculate light space matrix based on current light
     */
    void CalculateLightSpaceMatrix();

    /**
     * @brief Cleanup resources
     */
    void Cleanup();
};

} // namespace agl

#endif // SHADOW_SYSTEM_H
