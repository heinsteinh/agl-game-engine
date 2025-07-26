#ifndef SHADER_H
#define SHADER_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace agl {

// Shader types
enum class ShaderType {
    Vertex = GL_VERTEX_SHADER,
    Fragment = GL_FRAGMENT_SHADER,
    Geometry = GL_GEOMETRY_SHADER,
    // Compute = GL_COMPUTE_SHADER
};

// Individual shader object
class Shader {
public:
    Shader(ShaderType type, const std::string &source);
    ~Shader();

    // Non-copyable but movable
    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;
    Shader(Shader &&other) noexcept;
    Shader &operator=(Shader &&other) noexcept;

    uint32_t GetID() const {
        return m_shaderID;
    }
    ShaderType GetType() const {
        return m_type;
    }
    bool IsCompiled() const {
        return m_compiled;
    }
    const std::string &GetErrorLog() const {
        return m_errorLog;
    }

    // Static factory methods
    static std::unique_ptr<Shader> CreateFromFile(ShaderType type, const std::string &filepath);
    static std::unique_ptr<Shader> CreateFromSource(ShaderType type, const std::string &source);

private:
    uint32_t m_shaderID;
    ShaderType m_type;
    bool m_compiled;
    std::string m_errorLog;

    bool Compile(const std::string &source);
    static std::string ReadFile(const std::string &filepath);
};

// Shader program that links multiple shaders
class ShaderProgram {
public:
    ShaderProgram();
    ~ShaderProgram();

    // Non-copyable but movable
    ShaderProgram(const ShaderProgram &) = delete;
    ShaderProgram &operator=(const ShaderProgram &) = delete;
    ShaderProgram(ShaderProgram &&other) noexcept;
    ShaderProgram &operator=(ShaderProgram &&other) noexcept;

    // Shader attachment and linking
    void AttachShader(const Shader &shader);
    void AttachShader(std::unique_ptr<Shader> shader);
    bool Link();

    // Program usage
    void Use() const;
    void Unuse() const;

    // Getters
    uint32_t GetID() const {
        return m_programID;
    }
    bool IsLinked() const {
        return m_linked;
    }
    const std::string &GetErrorLog() const {
        return m_errorLog;
    }

    // Uniform setting methods
    void SetUniform(const std::string &name, bool value);
    void SetUniform(const std::string &name, int value);
    void SetUniform(const std::string &name, float value);
    void SetUniform(const std::string &name, const glm::vec2 &value);
    void SetUniform(const std::string &name, const glm::vec3 &value);
    void SetUniform(const std::string &name, const glm::vec4 &value);
    void SetUniform(const std::string &name, const glm::mat2 &value);
    void SetUniform(const std::string &name, const glm::mat3 &value);
    void SetUniform(const std::string &name, const glm::mat4 &value);

    // Array uniforms
    void SetUniform(const std::string &name, const std::vector<int> &values);
    void SetUniform(const std::string &name, const std::vector<float> &values);
    void SetUniform(const std::string &name, const std::vector<glm::vec2> &values);
    void SetUniform(const std::string &name, const std::vector<glm::vec3> &values);
    void SetUniform(const std::string &name, const std::vector<glm::vec4> &values);

    // Uniform block binding
    void BindUniformBlock(const std::string &blockName, uint32_t bindingPoint);

    // Static factory methods
    static std::unique_ptr<ShaderProgram> CreateFromFiles(const std::string &vertexPath,
                                                          const std::string &fragmentPath);

    static std::unique_ptr<ShaderProgram>
    CreateFromFiles(const std::string &vertexPath, const std::string &fragmentPath, const std::string &geometryPath);

    static std::unique_ptr<ShaderProgram> CreateFromSources(const std::string &vertexSource,
                                                            const std::string &fragmentSource);

    static std::unique_ptr<ShaderProgram> CreateFromSources(const std::string &vertexSource,
                                                            const std::string &fragmentSource,
                                                            const std::string &geometrySource);

    // Common shader presets
    static std::unique_ptr<ShaderProgram> CreateBasicColorShader();
    static std::unique_ptr<ShaderProgram> CreateBasicTextureShader();
    static std::unique_ptr<ShaderProgram> CreatePhongShader();

private:
    uint32_t m_programID;
    bool m_linked;
    std::string m_errorLog;
    std::vector<std::unique_ptr<Shader>> m_attachedShaders;

    // Uniform location caching
    mutable std::unordered_map<std::string, int> m_uniformLocationCache;

    int GetUniformLocation(const std::string &name) const;
    void CheckCompileErrors(uint32_t shader, const std::string &type);
};

// Shader manager for caching and reusing shaders
class ShaderManager {
public:
    static ShaderManager &Instance();

    // Load and cache shaders
    std::shared_ptr<ShaderProgram> LoadShader(const std::string &name, const std::string &vertexPath,
                                              const std::string &fragmentPath);

    std::shared_ptr<ShaderProgram> LoadShader(const std::string &name, const std::string &vertexPath,
                                              const std::string &fragmentPath, const std::string &geometryPath);

    // Get cached shader
    std::shared_ptr<ShaderProgram> GetShader(const std::string &name);

    // Remove shader from cache
    void RemoveShader(const std::string &name);

    // Clear all shaders
    void Clear();

private:
    std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> m_shaders;

    ShaderManager() = default;
    ~ShaderManager() = default;
    ShaderManager(const ShaderManager &) = delete;
    ShaderManager &operator=(const ShaderManager &) = delete;
};

} // namespace agl

#endif // SHADER_H
