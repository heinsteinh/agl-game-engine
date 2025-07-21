#include "Texture.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <unordered_map>

// Include STB for image loading
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace agl {

// ===== Base Texture Class =====

Texture::Texture(GLenum target)
    : m_textureID(0)
    , m_target(target)
    , m_width(0)
    , m_height(0)
    , m_format(TextureFormat::RGBA)
    , m_hasMipmaps(false) {
    CreateTexture();
}

Texture::~Texture() {
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
    }
}

Texture::Texture(Texture&& other) noexcept
    : m_textureID(other.m_textureID)
    , m_target(other.m_target)
    , m_width(other.m_width)
    , m_height(other.m_height)
    , m_format(other.m_format)
    , m_hasMipmaps(other.m_hasMipmaps) {
    other.m_textureID = 0;
    other.m_width = 0;
    other.m_height = 0;
    other.m_hasMipmaps = false;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        if (m_textureID != 0) {
            glDeleteTextures(1, &m_textureID);
        }
        
        m_textureID = other.m_textureID;
        m_target = other.m_target;
        m_width = other.m_width;
        m_height = other.m_height;
        m_format = other.m_format;
        m_hasMipmaps = other.m_hasMipmaps;
        
        other.m_textureID = 0;
        other.m_width = 0;
        other.m_height = 0;
        other.m_hasMipmaps = false;
    }
    return *this;
}

void Texture::CreateTexture() {
    glGenTextures(1, &m_textureID);
}

void Texture::Bind(uint32_t slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(m_target, m_textureID);
}

void Texture::Unbind() const {
    glBindTexture(m_target, 0);
}

void Texture::SetFilter(TextureFilter minFilter, TextureFilter magFilter) {
    Bind();
    glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(minFilter));
    glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(magFilter));
}

void Texture::SetWrap(TextureWrap wrapS, TextureWrap wrapT) {
    Bind();
    glTexParameteri(m_target, GL_TEXTURE_WRAP_S, static_cast<GLint>(wrapS));
    glTexParameteri(m_target, GL_TEXTURE_WRAP_T, static_cast<GLint>(wrapT));
}

void Texture::SetBorderColor(float r, float g, float b, float a) {
    Bind();
    float borderColor[] = { r, g, b, a };
    glTexParameterfv(m_target, GL_TEXTURE_BORDER_COLOR, borderColor);
}

void Texture::GenerateMipmaps() {
    Bind();
    glGenerateMipmap(m_target);
    m_hasMipmaps = true;
}

// ===== Texture2D Class =====

Texture2D::Texture2D()
    : Texture(GL_TEXTURE_2D)
    , m_dataType(TextureDataType::UnsignedByte) {
    // Set default filtering
    SetFilter(TextureFilter::Linear, TextureFilter::Linear);
    SetWrap(TextureWrap::Repeat, TextureWrap::Repeat);
}

void Texture2D::CreateFromData(uint32_t width, uint32_t height, 
                             TextureFormat format, TextureDataType dataType, 
                             const void* data) {
    m_width = width;
    m_height = height;
    m_format = format;
    m_dataType = dataType;
    
    Bind();
    
    // Determine internal format and format based on the enum
    GLenum internalFormat = static_cast<GLenum>(format);
    GLenum glFormat;
    
    switch (format) {
        case TextureFormat::RGB:
        case TextureFormat::RGB16F:
        case TextureFormat::RGB32F:
            glFormat = GL_RGB;
            break;
        case TextureFormat::RGBA:
        case TextureFormat::RGBA16F:
        case TextureFormat::RGBA32F:
            glFormat = GL_RGBA;
            break;
        case TextureFormat::R:
            glFormat = GL_RED;
            break;
        case TextureFormat::RG:
            glFormat = GL_RG;
            break;
        case TextureFormat::Depth:
            glFormat = GL_DEPTH_COMPONENT;
            break;
        case TextureFormat::DepthStencil:
            glFormat = GL_DEPTH_STENCIL;
            break;
        default:
            glFormat = GL_RGBA;
            break;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, 
                 glFormat, static_cast<GLenum>(dataType), data);
}

bool Texture2D::LoadFromFile(const std::string& filepath, bool flipVertically) {
    stbi_set_flip_vertically_on_load(flipVertically);
    
    int width, height, channels;
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    
    if (!data) {
        std::cerr << "Failed to load texture: " << filepath << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        return false;
    }
    
    TextureFormat format;
    switch (channels) {
        case 1: format = TextureFormat::R; break;
        case 2: format = TextureFormat::RG; break;
        case 3: format = TextureFormat::RGB; break;
        case 4: format = TextureFormat::RGBA; break;
        default:
            std::cerr << "Unsupported number of channels: " << channels << std::endl;
            stbi_image_free(data);
            return false;
    }
    
    CreateFromData(width, height, format, TextureDataType::UnsignedByte, data);
    
    stbi_image_free(data);
    
    std::cout << "Loaded texture: " << filepath << " (" << width << "x" << height << ", " << channels << " channels)" << std::endl;
    return true;
}

void Texture2D::SetData(const void* data, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    if (width == 0) width = m_width - x;
    if (height == 0) height = m_height - y;
    
    Bind();
    
    GLenum format;
    switch (m_format) {
        case TextureFormat::RGB:
        case TextureFormat::RGB16F:
        case TextureFormat::RGB32F:
            format = GL_RGB;
            break;
        case TextureFormat::RGBA:
        case TextureFormat::RGBA16F:
        case TextureFormat::RGBA32F:
            format = GL_RGBA;
            break;
        case TextureFormat::R:
            format = GL_RED;
            break;
        case TextureFormat::RG:
            format = GL_RG;
            break;
        default:
            format = GL_RGBA;
            break;
    }
    
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, format, 
                    static_cast<GLenum>(m_dataType), data);
}

void Texture2D::CreateSolidColor(uint32_t width, uint32_t height, float r, float g, float b, float a) {
    std::vector<uint8_t> data(width * height * 4);
    
    uint8_t red = static_cast<uint8_t>(r * 255.0f);
    uint8_t green = static_cast<uint8_t>(g * 255.0f);
    uint8_t blue = static_cast<uint8_t>(b * 255.0f);
    uint8_t alpha = static_cast<uint8_t>(a * 255.0f);
    
    for (uint32_t i = 0; i < width * height; ++i) {
        data[i * 4 + 0] = red;
        data[i * 4 + 1] = green;
        data[i * 4 + 2] = blue;
        data[i * 4 + 3] = alpha;
    }
    
    CreateFromData(width, height, TextureFormat::RGBA, TextureDataType::UnsignedByte, data.data());
}

void Texture2D::CreateRandomColor(uint32_t width, uint32_t height, uint32_t seed, float alpha) {
    auto data = GenerateRandomColorData(width, height, seed, alpha);
    CreateFromData(width, height, TextureFormat::RGBA, TextureDataType::UnsignedByte, data.data());
}

void Texture2D::CreateCheckerboard(uint32_t width, uint32_t height, uint32_t checkerSize,
                                 float color1R, float color1G, float color1B,
                                 float color2R, float color2G, float color2B) {
    auto data = GenerateCheckerboardData(width, height, checkerSize, 
                                       color1R, color1G, color1B, 
                                       color2R, color2G, color2B);
    CreateFromData(width, height, TextureFormat::RGB, TextureDataType::UnsignedByte, data.data());
}

void Texture2D::CreateNoise(uint32_t width, uint32_t height, uint32_t seed) {
    auto data = GenerateNoiseData(width, height, seed);
    CreateFromData(width, height, TextureFormat::RGB, TextureDataType::UnsignedByte, data.data());
}

void Texture2D::CreateGradient(uint32_t width, uint32_t height,
                              float startR, float startG, float startB,
                              float endR, float endG, float endB,
                              bool horizontal) {
    auto data = GenerateGradientData(width, height, startR, startG, startB, 
                                   endR, endG, endB, horizontal);
    CreateFromData(width, height, TextureFormat::RGB, TextureDataType::UnsignedByte, data.data());
}

std::vector<uint8_t> Texture2D::GenerateRandomColorData(uint32_t width, uint32_t height, uint32_t seed, float alpha) {
    std::vector<uint8_t> data(width * height * 4);
    
    std::mt19937 rng(seed == 0 ? std::random_device{}() : seed);
    std::uniform_int_distribution<int> colorDist(0, 255);
    
    uint8_t alphaValue = static_cast<uint8_t>(alpha * 255.0f);
    
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            uint32_t index = (y * width + x) * 4;
            data[index + 0] = static_cast<uint8_t>(colorDist(rng)); // Red
            data[index + 1] = static_cast<uint8_t>(colorDist(rng)); // Green
            data[index + 2] = static_cast<uint8_t>(colorDist(rng)); // Blue
            data[index + 3] = alphaValue;                           // Alpha
        }
    }
    
    return data;
}

std::vector<uint8_t> Texture2D::GenerateCheckerboardData(uint32_t width, uint32_t height, 
                                                        uint32_t checkerSize,
                                                        float color1R, float color1G, float color1B,
                                                        float color2R, float color2G, float color2B) {
    std::vector<uint8_t> data(width * height * 3);
    
    uint8_t color1[3] = {
        static_cast<uint8_t>(color1R * 255.0f),
        static_cast<uint8_t>(color1G * 255.0f),
        static_cast<uint8_t>(color1B * 255.0f)
    };
    
    uint8_t color2[3] = {
        static_cast<uint8_t>(color2R * 255.0f),
        static_cast<uint8_t>(color2G * 255.0f),
        static_cast<uint8_t>(color2B * 255.0f)
    };
    
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            uint32_t index = (y * width + x) * 3;
            
            bool useColor1 = ((x / checkerSize) + (y / checkerSize)) % 2 == 0;
            uint8_t* color = useColor1 ? color1 : color2;
            
            data[index + 0] = color[0];
            data[index + 1] = color[1];
            data[index + 2] = color[2];
        }
    }
    
    return data;
}

std::vector<uint8_t> Texture2D::GenerateNoiseData(uint32_t width, uint32_t height, uint32_t seed) {
    std::vector<uint8_t> data(width * height * 3);
    
    std::mt19937 rng(seed == 0 ? std::random_device{}() : seed);
    std::uniform_int_distribution<int> noiseDist(0, 255);
    
    for (uint32_t i = 0; i < width * height * 3; ++i) {
        data[i] = static_cast<uint8_t>(noiseDist(rng));
    }
    
    return data;
}

std::vector<uint8_t> Texture2D::GenerateGradientData(uint32_t width, uint32_t height,
                                                    float startR, float startG, float startB,
                                                    float endR, float endG, float endB,
                                                    bool horizontal) {
    std::vector<uint8_t> data(width * height * 3);
    
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            uint32_t index = (y * width + x) * 3;
            
            float t = horizontal ? static_cast<float>(x) / static_cast<float>(width - 1)
                                : static_cast<float>(y) / static_cast<float>(height - 1);
            
            float r = startR + t * (endR - startR);
            float g = startG + t * (endG - startG);
            float b = startB + t * (endB - startB);
            
            data[index + 0] = static_cast<uint8_t>(std::clamp(r, 0.0f, 1.0f) * 255.0f);
            data[index + 1] = static_cast<uint8_t>(std::clamp(g, 0.0f, 1.0f) * 255.0f);
            data[index + 2] = static_cast<uint8_t>(std::clamp(b, 0.0f, 1.0f) * 255.0f);
        }
    }
    
    return data;
}

// Static factory methods
std::unique_ptr<Texture2D> Texture2D::Create(uint32_t width, uint32_t height, TextureFormat format) {
    auto texture = std::make_unique<Texture2D>();
    texture->CreateFromData(width, height, format, TextureDataType::UnsignedByte);
    return texture;
}

std::unique_ptr<Texture2D> Texture2D::LoadFromFile(const std::string& filepath) {
    auto texture = std::make_unique<Texture2D>();
    if (texture->LoadFromFile(filepath)) {
        return texture;
    }
    return nullptr;
}

std::unique_ptr<Texture2D> Texture2D::CreateSolidColor(uint32_t width, uint32_t height, 
                                                      float r, float g, float b, float a) {
    auto texture = std::make_unique<Texture2D>();
    texture->CreateSolidColor(width, height, r, g, b, a);
    return texture;
}

std::unique_ptr<Texture2D> Texture2D::CreateRandomColor(uint32_t width, uint32_t height, 
                                                       uint32_t seed, float alpha) {
    auto texture = std::make_unique<Texture2D>();
    texture->CreateRandomColor(width, height, seed, alpha);
    return texture;
}

std::unique_ptr<Texture2D> Texture2D::CreateWhite(uint32_t size) {
    return CreateSolidColor(size, size, 1.0f, 1.0f, 1.0f, 1.0f);
}

std::unique_ptr<Texture2D> Texture2D::CreateBlack(uint32_t size) {
    return CreateSolidColor(size, size, 0.0f, 0.0f, 0.0f, 1.0f);
}

// ===== TextureManager Class =====

TextureManager& TextureManager::Instance() {
    static TextureManager instance;
    return instance;
}

TextureManager::TextureManager() {
    CreateDefaultTextures();
}

void TextureManager::CreateDefaultTextures() {
    m_whiteTexture = Texture2D::CreateWhite(1);
    m_blackTexture = Texture2D::CreateBlack(1);
}

std::shared_ptr<Texture2D> TextureManager::LoadTexture(const std::string& name, const std::string& filepath) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        return it->second;
    }
    
    auto texture = Texture2D::LoadFromFile(filepath);
    if (texture) {
        auto sharedTexture = std::shared_ptr<Texture2D>(texture.release());
        m_textures[name] = sharedTexture;
        return sharedTexture;
    }
    
    return nullptr;
}

std::shared_ptr<Texture2D> TextureManager::CreateSolidColorTexture(const std::string& name, 
                                                                 uint32_t width, uint32_t height,
                                                                 float r, float g, float b, float a) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        return it->second;
    }
    
    auto texture = Texture2D::CreateSolidColor(width, height, r, g, b, a);
    if (texture) {
        auto sharedTexture = std::shared_ptr<Texture2D>(texture.release());
        m_textures[name] = sharedTexture;
        return sharedTexture;
    }
    
    return nullptr;
}

std::shared_ptr<Texture2D> TextureManager::CreateRandomColorTexture(const std::string& name,
                                                                  uint32_t width, uint32_t height,
                                                                  uint32_t seed, float alpha) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        return it->second;
    }
    
    auto texture = Texture2D::CreateRandomColor(width, height, seed, alpha);
    if (texture) {
        auto sharedTexture = std::shared_ptr<Texture2D>(texture.release());
        m_textures[name] = sharedTexture;
        return sharedTexture;
    }
    
    return nullptr;
}

std::shared_ptr<Texture2D> TextureManager::GetTexture(const std::string& name) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        return it->second;
    }
    return nullptr;
}

void TextureManager::RemoveTexture(const std::string& name) {
    m_textures.erase(name);
}

void TextureManager::Clear() {
    m_textures.clear();
    CreateDefaultTextures();
}

std::shared_ptr<Texture2D> TextureManager::GetWhiteTexture() {
    return m_whiteTexture;
}

std::shared_ptr<Texture2D> TextureManager::GetBlackTexture() {
    return m_blackTexture;
}

} // namespace agl
