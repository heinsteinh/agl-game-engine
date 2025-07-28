#include "ShadowSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace agl {

ShadowSystem::ShadowSystem(int shadowMapSize) : m_shadowMapSize(shadowMapSize), m_shadowMap(0), m_shadowMapFBO(0) {}

ShadowSystem::~ShadowSystem() {
    Cleanup();
}

ShadowSystem::ShadowSystem(ShadowSystem &&other) noexcept
    : m_shadowMapSize(other.m_shadowMapSize), m_shadowMap(other.m_shadowMap), m_shadowMapFBO(other.m_shadowMapFBO),
      m_shadowShader(std::move(other.m_shadowShader)), m_shadowedShader(std::move(other.m_shadowedShader)),
      m_light(other.m_light), m_lightSpaceMatrix(other.m_lightSpaceMatrix), m_lightProjection(other.m_lightProjection),
      m_lightView(other.m_lightView), m_shadowBias(other.m_shadowBias), m_pcfEnabled(other.m_pcfEnabled),
      m_orthographicSize(other.m_orthographicSize), m_viewMatrix(other.m_viewMatrix),
      m_projectionMatrix(other.m_projectionMatrix) {

    // Clear other object
    other.m_shadowMap = 0;
    other.m_shadowMapFBO = 0;
}

ShadowSystem &ShadowSystem::operator=(ShadowSystem &&other) noexcept {
    if (this != &other) {
        Cleanup();

        m_shadowMapSize = other.m_shadowMapSize;
        m_shadowMap = other.m_shadowMap;
        m_shadowMapFBO = other.m_shadowMapFBO;
        m_shadowShader = std::move(other.m_shadowShader);
        m_shadowedShader = std::move(other.m_shadowedShader);
        m_light = other.m_light;
        m_lightSpaceMatrix = other.m_lightSpaceMatrix;
        m_lightProjection = other.m_lightProjection;
        m_lightView = other.m_lightView;
        m_shadowBias = other.m_shadowBias;
        m_pcfEnabled = other.m_pcfEnabled;
        m_orthographicSize = other.m_orthographicSize;
        m_viewMatrix = other.m_viewMatrix;
        m_projectionMatrix = other.m_projectionMatrix;

        other.m_shadowMap = 0;
        other.m_shadowMapFBO = 0;
    }
    return *this;
}

bool ShadowSystem::Initialize() {
    if (!CreateShadowMap()) {
        std::cerr << "Failed to create shadow map" << std::endl;
        return false;
    }

    if (!CreateShaders()) {
        std::cerr << "Failed to create shadow shaders" << std::endl;
        return false;
    }

    // Set default directional light
    m_light = Light(LightType::Directional, glm::vec3(0.0f, 10.0f, 5.0f), glm::vec3(0.0f, -1.0f, -0.5f));
    CalculateLightSpaceMatrix();

    std::cout << "ShadowSystem initialized with " << m_shadowMapSize << "x" << m_shadowMapSize << " shadow map"
              << std::endl;
    return true;
}

bool ShadowSystem::CreateShadowMap() {
    // Generate framebuffer for shadow map
    glGenFramebuffers(1, &m_shadowMapFBO);

    // Generate depth texture for shadow map
    glGenTextures(1, &m_shadowMap);
    glBindTexture(GL_TEXTURE_2D, m_shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_shadowMapSize, m_shadowMapSize, 0, GL_DEPTH_COMPONENT,
                 GL_FLOAT, nullptr);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // Set border color to white (outside shadow map = no shadow)
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Attach depth texture to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMap, 0);

    // Disable color buffer (we only need depth)
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Shadow map framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

bool ShadowSystem::CreateShaders() {
    // Shadow map generation shader (vertex)
    const char *shadowVertexSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;

        uniform mat4 lightSpaceMatrix;
        uniform mat4 model;

        void main() {
            gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
        }
    )";

    // Shadow map generation shader (fragment) - empty, depth is written automatically
    const char *shadowFragmentSource = R"(
        #version 330 core

        void main() {
            // Empty - depth is written automatically
        }
    )";

    // Create shadow generation shader
    m_shadowShader = ShaderProgram::CreateFromSources(shadowVertexSource, shadowFragmentSource);
    if (!m_shadowShader) {
        std::cerr << "Failed to load shadow shader" << std::endl;
        return false;
    }

    // Main rendering shader with shadow mapping
    const char *shadowedVertexSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoord;

        out vec3 FragPos;
        out vec3 Normal;
        out vec2 TexCoord;
        out vec4 FragPosLightSpace;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform mat4 lightSpaceMatrix;
        uniform mat3 normalMatrix;

        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = normalMatrix * aNormal;
            TexCoord = aTexCoord;
            FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";

    const char *shadowedFragmentSource = R"(
        #version 330 core
        out vec4 FragColor;

        in vec3 FragPos;
        in vec3 Normal;
        in vec2 TexCoord;
        in vec4 FragPosLightSpace;

        // Material
        uniform vec3 material_ambient;
        uniform vec3 material_diffuse;
        uniform vec3 material_specular;
        uniform float material_shininess;

        // Light
        uniform vec3 lightPos;
        uniform vec3 lightColor;
        uniform vec3 lightDirection;
        uniform int lightType; // 0 = directional, 1 = point, 2 = spot

        // Camera
        uniform vec3 viewPos;

        // Shadow
        uniform sampler2D shadowMap;
        uniform float shadowBias;
        uniform bool pcfEnabled;

        float ShadowCalculation(vec4 fragPosLightSpace) {
            // Perform perspective divide
            vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

            // Transform to [0,1] range
            projCoords = projCoords * 0.5 + 0.5;

            // Get closest depth value from light's perspective
            float closestDepth = texture(shadowMap, projCoords.xy).r;

            // Get depth of current fragment from light's perspective
            float currentDepth = projCoords.z;

            // Check whether current frag pos is in shadow
            float shadow = 0.0;

            if (pcfEnabled) {
                // PCF (Percentage Closer Filtering) for softer shadows
                vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
                for(int x = -1; x <= 1; ++x) {
                    for(int y = -1; y <= 1; ++y) {
                        float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                        shadow += currentDepth - shadowBias > pcfDepth ? 1.0 : 0.0;
                    }
                }
                shadow /= 9.0;
            } else {
                // Simple shadow mapping
                shadow = currentDepth - shadowBias > closestDepth ? 1.0 : 0.0;
            }

            // Keep the shadow at 0.0 when outside the far_plane region of the light's frustum
            if(projCoords.z > 1.0)
                shadow = 0.0;

            return shadow;
        }

        void main() {
            vec3 color = material_diffuse;
            vec3 normal = normalize(Normal);
            vec3 lightColor_actual = lightColor;

            // Ambient
            vec3 ambient = 0.15 * color;

            // Diffuse
            vec3 lightDir;
            if (lightType == 0) { // Directional light
                lightDir = normalize(-lightDirection);
            } else { // Point or spot light
                lightDir = normalize(lightPos - FragPos);
            }

            float diff = max(dot(lightDir, normal), 0.0);
            vec3 diffuse = diff * lightColor_actual;

            // Specular
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 reflectDir = reflect(-lightDir, normal);
            float spec = 0.0;
            vec3 halfwayDir = normalize(lightDir + viewDir);
            spec = pow(max(dot(normal, halfwayDir), 0.0), material_shininess);
            vec3 specular = spec * lightColor_actual * material_specular;

            // Calculate shadow
            float shadow = ShadowCalculation(FragPosLightSpace);
            vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);

            FragColor = vec4(lighting * color, 1.0);
        }
    )";

    // Create shadowed rendering shader
    m_shadowedShader = ShaderProgram::CreateFromSources(shadowedVertexSource, shadowedFragmentSource);
    if (!m_shadowedShader) {
        std::cerr << "Failed to load shadowed shader" << std::endl;
        return false;
    }

    return true;
}

void ShadowSystem::SetLight(const Light &light) {
    m_light = light;
    CalculateLightSpaceMatrix();
}

void ShadowSystem::CalculateLightSpaceMatrix() {
    float near_plane = 1.0f, far_plane = 50.0f;

    if (m_light.type == LightType::Directional) {
        // Orthographic projection for directional light
        m_lightProjection = glm::ortho(-m_orthographicSize, m_orthographicSize, -m_orthographicSize, m_orthographicSize,
                                       near_plane, far_plane);

        // Look from light position towards light direction
        glm::vec3 lightPos = -m_light.direction * 25.0f; // Position light far away
        m_lightView = glm::lookAt(lightPos, lightPos + m_light.direction, glm::vec3(0.0f, 1.0f, 0.0f));
    } else {
        // Perspective projection for point/spot lights
        m_lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, near_plane, far_plane);
        m_lightView = glm::lookAt(m_light.position, m_light.position + m_light.direction, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    m_lightSpaceMatrix = m_lightProjection * m_lightView;
}

void ShadowSystem::BeginShadowPass() {
    // Bind shadow map framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFBO);
    glViewport(0, 0, m_shadowMapSize, m_shadowMapSize);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Use shadow shader
    m_shadowShader->Use();
    m_shadowShader->SetUniform("lightSpaceMatrix", m_lightSpaceMatrix);

    // Configure rendering for shadow pass
    glCullFace(GL_FRONT); // Helps reduce shadow acne
}

void ShadowSystem::EndShadowPass() {
    glCullFace(GL_BACK); // Restore normal culling
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowSystem::BeginMainPass(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) {
    m_viewMatrix = viewMatrix;
    m_projectionMatrix = projectionMatrix;

    // Reset viewport (assuming full screen, this should be set by the caller)
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, viewport[2], viewport[3]);

    // Use shadowed shader
    m_shadowedShader->Use();
    m_shadowedShader->SetUniform("view", m_viewMatrix);
    m_shadowedShader->SetUniform("projection", m_projectionMatrix);
    m_shadowedShader->SetUniform("lightSpaceMatrix", m_lightSpaceMatrix);

    // Set light properties
    m_shadowedShader->SetUniform("lightPos", m_light.position);
    m_shadowedShader->SetUniform("lightColor", m_light.color * m_light.intensity);
    m_shadowedShader->SetUniform("lightDirection", m_light.direction);
    m_shadowedShader->SetUniform("lightType", static_cast<int>(m_light.type));

    // Set shadow parameters
    m_shadowedShader->SetUniform("shadowBias", m_shadowBias);
    m_shadowedShader->SetUniform("pcfEnabled", m_pcfEnabled);

    // Bind shadow map texture
    glActiveTexture(GL_TEXTURE0 + 1); // Use texture unit 1 for shadow map
    glBindTexture(GL_TEXTURE_2D, m_shadowMap);
    m_shadowedShader->SetUniform("shadowMap", 1);
}

void ShadowSystem::RenderShadowCaster(Mesh &mesh, const glm::mat4 &modelMatrix) {
    m_shadowShader->SetUniform("model", modelMatrix);
    mesh.Render();
}

void ShadowSystem::RenderWithShadows(Mesh &mesh, const glm::mat4 &modelMatrix) {
    // Set model matrix and normal matrix
    m_shadowedShader->SetUniform("model", modelMatrix);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
    m_shadowedShader->SetUniform("normalMatrix", normalMatrix);

    // Set camera position (extract from view matrix)
    glm::vec3 viewPos = glm::vec3(glm::inverse(m_viewMatrix)[3]);
    m_shadowedShader->SetUniform("viewPos", viewPos);

    // Set material properties (these should come from the mesh)
    // For now, use default values - in a real implementation,
    // these would come from the mesh's material
    m_shadowedShader->SetUniform("material_ambient", glm::vec3(0.1f, 0.1f, 0.1f));
    m_shadowedShader->SetUniform("material_diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
    m_shadowedShader->SetUniform("material_specular", glm::vec3(1.0f, 1.0f, 1.0f));
    m_shadowedShader->SetUniform("material_shininess", 32.0f);

    mesh.Render();
}

void ShadowSystem::Cleanup() {
    if (m_shadowMap != 0) {
        glDeleteTextures(1, &m_shadowMap);
        m_shadowMap = 0;
    }

    if (m_shadowMapFBO != 0) {
        glDeleteFramebuffers(1, &m_shadowMapFBO);
        m_shadowMapFBO = 0;
    }
}

} // namespace agl
