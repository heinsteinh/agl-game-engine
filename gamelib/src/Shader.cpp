#include "Shader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

namespace agl {

// ===== Shader Class =====

Shader::Shader(ShaderType type, const std::string &source) : m_shaderID(0), m_type(type), m_compiled(false) {

    m_shaderID = glCreateShader(static_cast<GLenum>(type));
    m_compiled = Compile(source);
}

Shader::~Shader() {
    if (m_shaderID != 0) {
        glDeleteShader(m_shaderID);
    }
}

Shader::Shader(Shader &&other) noexcept
    : m_shaderID(other.m_shaderID), m_type(other.m_type), m_compiled(other.m_compiled),
      m_errorLog(std::move(other.m_errorLog)) {
    other.m_shaderID = 0;
    other.m_compiled = false;
}

Shader &Shader::operator=(Shader &&other) noexcept {
    if (this != &other) {
        if (m_shaderID != 0) {
            glDeleteShader(m_shaderID);
        }

        m_shaderID = other.m_shaderID;
        m_type = other.m_type;
        m_compiled = other.m_compiled;
        m_errorLog = std::move(other.m_errorLog);

        other.m_shaderID = 0;
        other.m_compiled = false;
    }
    return *this;
}

std::unique_ptr<Shader> Shader::CreateFromFile(ShaderType type, const std::string &filepath) {
    std::string source = ReadFile(filepath);
    if (source.empty()) {
        std::cerr << "Failed to read shader file: " << filepath << std::endl;
        return nullptr;
    }
    return std::make_unique<Shader>(type, source);
}

std::unique_ptr<Shader> Shader::CreateFromSource(ShaderType type, const std::string &source) {
    return std::make_unique<Shader>(type, source);
}

bool Shader::Compile(const std::string &source) {
    const char *sourceCStr = source.c_str();
    glShaderSource(m_shaderID, 1, &sourceCStr, nullptr);
    glCompileShader(m_shaderID);

    int success;
    glGetShaderiv(m_shaderID, GL_COMPILE_STATUS, &success);

    if (!success) {
        int length;
        glGetShaderiv(m_shaderID, GL_INFO_LOG_LENGTH, &length);

        std::vector<char> errorLog(length);
        glGetShaderInfoLog(m_shaderID, length, &length, errorLog.data());
        m_errorLog = std::string(errorLog.begin(), errorLog.end());

        std::cerr << "Shader compilation failed:\n" << m_errorLog << std::endl;
        return false;
    }

    return true;
}

std::string Shader::ReadFile(const std::string &filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// ===== ShaderProgram Class =====

ShaderProgram::ShaderProgram() : m_programID(0), m_linked(false) {
    m_programID = glCreateProgram();
}

ShaderProgram::~ShaderProgram() {
    if (m_programID != 0) {
        glDeleteProgram(m_programID);
    }
}

ShaderProgram::ShaderProgram(ShaderProgram &&other) noexcept
    : m_programID(other.m_programID), m_linked(other.m_linked), m_errorLog(std::move(other.m_errorLog)),
      m_attachedShaders(std::move(other.m_attachedShaders)),
      m_uniformLocationCache(std::move(other.m_uniformLocationCache)) {
    other.m_programID = 0;
    other.m_linked = false;
}

ShaderProgram &ShaderProgram::operator=(ShaderProgram &&other) noexcept {
    if (this != &other) {
        if (m_programID != 0) {
            glDeleteProgram(m_programID);
        }

        m_programID = other.m_programID;
        m_linked = other.m_linked;
        m_errorLog = std::move(other.m_errorLog);
        m_attachedShaders = std::move(other.m_attachedShaders);
        m_uniformLocationCache = std::move(other.m_uniformLocationCache);

        other.m_programID = 0;
        other.m_linked = false;
    }
    return *this;
}

void ShaderProgram::AttachShader(const Shader &shader) {
    if (shader.IsCompiled()) {
        glAttachShader(m_programID, shader.GetID());
    }
}

void ShaderProgram::AttachShader(std::unique_ptr<Shader> shader) {
    if (shader && shader->IsCompiled()) {
        glAttachShader(m_programID, shader->GetID());
        m_attachedShaders.push_back(std::move(shader));
    }
}

bool ShaderProgram::Link() {
    glLinkProgram(m_programID);

    int success;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);

    if (!success) {
        int length;
        glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &length);

        std::vector<char> errorLog(length);
        glGetProgramInfoLog(m_programID, length, &length, errorLog.data());
        m_errorLog = std::string(errorLog.begin(), errorLog.end());

        std::cerr << "Shader program linking failed:\n" << m_errorLog << std::endl;
        return false;
    }

    m_linked = true;
    return true;
}

void ShaderProgram::Use() const {
    if (m_linked) {
        glUseProgram(m_programID);
    }
}

void ShaderProgram::Unuse() const {
    glUseProgram(0);
}

int ShaderProgram::GetUniformLocation(const std::string &name) const {
    auto it = m_uniformLocationCache.find(name);
    if (it != m_uniformLocationCache.end()) {
        return it->second;
    }

    int location = glGetUniformLocation(m_programID, name.c_str());
    m_uniformLocationCache[name] = location;

    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader program" << std::endl;
    }

    return location;
}

void ShaderProgram::SetUniform(const std::string &name, bool value) {
    glUniform1i(GetUniformLocation(name), static_cast<int>(value));
}

void ShaderProgram::SetUniform(const std::string &name, int value) {
    glUniform1i(GetUniformLocation(name), value);
}

void ShaderProgram::SetUniform(const std::string &name, float value) {
    glUniform1f(GetUniformLocation(name), value);
}

void ShaderProgram::SetUniform(const std::string &name, const glm::vec2 &value) {
    glUniform2fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void ShaderProgram::SetUniform(const std::string &name, const glm::vec3 &value) {
    glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void ShaderProgram::SetUniform(const std::string &name, const glm::vec4 &value) {
    glUniform4fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void ShaderProgram::SetUniform(const std::string &name, const glm::mat2 &value) {
    glUniformMatrix2fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::SetUniform(const std::string &name, const glm::mat3 &value) {
    glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::SetUniform(const std::string &name, const glm::mat4 &value) {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::SetUniform(const std::string &name, const std::vector<int> &values) {
    glUniform1iv(GetUniformLocation(name), static_cast<GLsizei>(values.size()), values.data());
}

void ShaderProgram::SetUniform(const std::string &name, const std::vector<float> &values) {
    glUniform1fv(GetUniformLocation(name), static_cast<GLsizei>(values.size()), values.data());
}

void ShaderProgram::SetUniform(const std::string &name, const std::vector<glm::vec2> &values) {
    glUniform2fv(GetUniformLocation(name), static_cast<GLsizei>(values.size()), glm::value_ptr(values[0]));
}

void ShaderProgram::SetUniform(const std::string &name, const std::vector<glm::vec3> &values) {
    glUniform3fv(GetUniformLocation(name), static_cast<GLsizei>(values.size()), glm::value_ptr(values[0]));
}

void ShaderProgram::SetUniform(const std::string &name, const std::vector<glm::vec4> &values) {
    glUniform4fv(GetUniformLocation(name), static_cast<GLsizei>(values.size()), glm::value_ptr(values[0]));
}

void ShaderProgram::BindUniformBlock(const std::string &blockName, uint32_t bindingPoint) {
    uint32_t blockIndex = glGetUniformBlockIndex(m_programID, blockName.c_str());
    if (blockIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(m_programID, blockIndex, bindingPoint);
    }
}

std::unique_ptr<ShaderProgram> ShaderProgram::CreateFromFiles(const std::string &vertexPath,
                                                              const std::string &fragmentPath) {

    auto program = std::make_unique<ShaderProgram>();

    auto vertexShader = Shader::CreateFromFile(ShaderType::Vertex, vertexPath);
    auto fragmentShader = Shader::CreateFromFile(ShaderType::Fragment, fragmentPath);

    if (!vertexShader || !fragmentShader) {
        return nullptr;
    }

    program->AttachShader(std::move(vertexShader));
    program->AttachShader(std::move(fragmentShader));

    if (!program->Link()) {
        return nullptr;
    }

    return program;
}

std::unique_ptr<ShaderProgram> ShaderProgram::CreateFromFiles(const std::string &vertexPath,
                                                              const std::string &fragmentPath,
                                                              const std::string &geometryPath) {

    auto program = std::make_unique<ShaderProgram>();

    auto vertexShader = Shader::CreateFromFile(ShaderType::Vertex, vertexPath);
    auto fragmentShader = Shader::CreateFromFile(ShaderType::Fragment, fragmentPath);
    auto geometryShader = Shader::CreateFromFile(ShaderType::Geometry, geometryPath);

    if (!vertexShader || !fragmentShader || !geometryShader) {
        return nullptr;
    }

    program->AttachShader(std::move(vertexShader));
    program->AttachShader(std::move(fragmentShader));
    program->AttachShader(std::move(geometryShader));

    if (!program->Link()) {
        return nullptr;
    }

    return program;
}

std::unique_ptr<ShaderProgram> ShaderProgram::CreateFromSources(const std::string &vertexSource,
                                                                const std::string &fragmentSource) {

    auto program = std::make_unique<ShaderProgram>();

    auto vertexShader = Shader::CreateFromSource(ShaderType::Vertex, vertexSource);
    auto fragmentShader = Shader::CreateFromSource(ShaderType::Fragment, fragmentSource);

    if (!vertexShader || !fragmentShader) {
        return nullptr;
    }

    program->AttachShader(std::move(vertexShader));
    program->AttachShader(std::move(fragmentShader));

    if (!program->Link()) {
        return nullptr;
    }

    return program;
}

std::unique_ptr<ShaderProgram> ShaderProgram::CreateFromSources(const std::string &vertexSource,
                                                                const std::string &fragmentSource,
                                                                const std::string &geometrySource) {

    auto program = std::make_unique<ShaderProgram>();

    auto vertexShader = Shader::CreateFromSource(ShaderType::Vertex, vertexSource);
    auto fragmentShader = Shader::CreateFromSource(ShaderType::Fragment, fragmentSource);
    auto geometryShader = Shader::CreateFromSource(ShaderType::Geometry, geometrySource);

    if (!vertexShader || !fragmentShader || !geometryShader) {
        return nullptr;
    }

    program->AttachShader(std::move(vertexShader));
    program->AttachShader(std::move(fragmentShader));
    program->AttachShader(std::move(geometryShader));

    if (!program->Link()) {
        return nullptr;
    }

    return program;
}

std::unique_ptr<ShaderProgram> ShaderProgram::CreateBasicColorShader() {
    const std::string vertexSource = R"(
        #version 330 core

        layout (location = 0) in vec3 a_Position;

        uniform mat4 u_MVP;

        void main() {
            gl_Position = u_MVP * vec4(a_Position, 1.0);
        }
    )";

    const std::string fragmentSource = R"(
        #version 330 core

        out vec4 FragColor;

        uniform vec4 u_Color;

        void main() {
            FragColor = u_Color;
        }
    )";

    return CreateFromSources(vertexSource, fragmentSource);
}

std::unique_ptr<ShaderProgram> ShaderProgram::CreateBasicTextureShader() {
    const std::string vertexSource = R"(
        #version 330 core

        layout (location = 0) in vec3 a_Position;
        layout (location = 1) in vec2 a_TexCoord;

        out vec2 v_TexCoord;

        uniform mat4 u_MVP;

        void main() {
            v_TexCoord = a_TexCoord;
            gl_Position = u_MVP * vec4(a_Position, 1.0);
        }
    )";

    const std::string fragmentSource = R"(
        #version 330 core

        in vec2 v_TexCoord;
        out vec4 FragColor;

        uniform sampler2D u_Texture;
        uniform vec4 u_Color;

        void main() {
            FragColor = texture(u_Texture, v_TexCoord) * u_Color;
        }
    )";

    return CreateFromSources(vertexSource, fragmentSource);
}

std::unique_ptr<ShaderProgram> ShaderProgram::CreatePhongShader() {
    const std::string vertexSource = R"(
        #version 330 core

        layout (location = 0) in vec3 a_Position;
        layout (location = 1) in vec3 a_Normal;
        layout (location = 2) in vec2 a_TexCoord;

        out vec3 v_FragPos;
        out vec3 v_Normal;
        out vec2 v_TexCoord;

        uniform mat4 u_Model;
        uniform mat4 u_View;
        uniform mat4 u_Projection;
        uniform mat3 u_NormalMatrix;

        void main() {
            v_FragPos = vec3(u_Model * vec4(a_Position, 1.0));
            v_Normal = u_NormalMatrix * a_Normal;
            v_TexCoord = a_TexCoord;

            gl_Position = u_Projection * u_View * vec4(v_FragPos, 1.0);
        }
    )";

    const std::string fragmentSource = R"(
        #version 330 core

        in vec3 v_FragPos;
        in vec3 v_Normal;
        in vec2 v_TexCoord;

        out vec4 FragColor;

        uniform vec3 u_LightPos;
        uniform vec3 u_LightColor;
        uniform vec3 u_ViewPos;

        uniform vec3 u_MaterialAmbient;
        uniform vec3 u_MaterialDiffuse;
        uniform vec3 u_MaterialSpecular;
        uniform float u_MaterialShininess;

        uniform sampler2D u_DiffuseTexture;

        void main() {
            // Ambient
            vec3 ambient = u_MaterialAmbient * u_LightColor;

            // Diffuse
            vec3 norm = normalize(v_Normal);
            vec3 lightDir = normalize(u_LightPos - v_FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * u_MaterialDiffuse * u_LightColor;

            // Specular
            vec3 viewDir = normalize(u_ViewPos - v_FragPos);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_MaterialShininess);
            vec3 specular = spec * u_MaterialSpecular * u_LightColor;

            vec3 result = ambient + diffuse + specular;
            vec4 texColor = texture(u_DiffuseTexture, v_TexCoord);

            FragColor = vec4(result, 1.0) * texColor;
        }
    )";

    return CreateFromSources(vertexSource, fragmentSource);
}

// ===== ShaderManager Class =====

ShaderManager &ShaderManager::Instance() {
    static ShaderManager instance;
    return instance;
}

std::shared_ptr<ShaderProgram> ShaderManager::LoadShader(const std::string &name, const std::string &vertexPath,
                                                         const std::string &fragmentPath) {

    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return it->second;
    }

    auto shader = ShaderProgram::CreateFromFiles(vertexPath, fragmentPath);
    if (shader) {
        auto sharedShader = std::shared_ptr<ShaderProgram>(shader.release());
        m_shaders[name] = sharedShader;
        return sharedShader;
    }

    return nullptr;
}

std::shared_ptr<ShaderProgram> ShaderManager::LoadShader(const std::string &name, const std::string &vertexPath,
                                                         const std::string &fragmentPath,
                                                         const std::string &geometryPath) {

    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return it->second;
    }

    auto shader = ShaderProgram::CreateFromFiles(vertexPath, fragmentPath, geometryPath);
    if (shader) {
        auto sharedShader = std::shared_ptr<ShaderProgram>(shader.release());
        m_shaders[name] = sharedShader;
        return sharedShader;
    }

    return nullptr;
}

std::shared_ptr<ShaderProgram> ShaderManager::GetShader(const std::string &name) {
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return it->second;
    }
    return nullptr;
}

void ShaderManager::RemoveShader(const std::string &name) {
    m_shaders.erase(name);
}

void ShaderManager::Clear() {
    m_shaders.clear();
}

} // namespace agl
