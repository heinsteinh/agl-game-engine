#ifndef TEXTURE_H
#define TEXTURE_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#if defined(__APPLE__)
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl3.h>
#else
    #include <GL/gl.h>
#endif

namespace agl {

    // Texture filtering modes
    enum class TextureFilter {
        Nearest = GL_NEAREST,
        Linear = GL_LINEAR,
        NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
        LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
        NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,
        LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR
    };

    // Texture wrapping modes
    enum class TextureWrap {
        Repeat = GL_REPEAT,
        MirroredRepeat = GL_MIRRORED_REPEAT,
        ClampToEdge = GL_CLAMP_TO_EDGE,
        ClampToBorder = GL_CLAMP_TO_BORDER
    };

    // Texture formats
    enum class TextureFormat {
        RGB = GL_RGB,
        RGBA = GL_RGBA,
        R = GL_RED,
        RG = GL_RG,
        RGB16F = GL_RGB16F,
        RGBA16F = GL_RGBA16F,
        RGB32F = GL_RGB32F,
        RGBA32F = GL_RGBA32F,
        Depth = GL_DEPTH_COMPONENT,
        DepthStencil = GL_DEPTH_STENCIL
    };

    // Texture data types
    enum class TextureDataType {
        UnsignedByte = GL_UNSIGNED_BYTE,
        Float = GL_FLOAT,
        UnsignedInt = GL_UNSIGNED_INT,
        UnsignedInt248 = GL_UNSIGNED_INT_24_8
    };

    // Base Texture class
    class Texture {
    public:
        Texture(GLenum target = GL_TEXTURE_2D);
        virtual ~Texture();

        // Non-copyable but movable
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;

        // Binding
        void Bind(uint32_t slot = 0) const;
        void Unbind() const;

        // Getters
        uint32_t GetID() const { return m_textureID; }
        GLenum GetTarget() const { return m_target; }
        uint32_t GetWidth() const { return m_width; }
        uint32_t GetHeight() const { return m_height; }
        TextureFormat GetFormat() const { return m_format; }

        // Texture parameters
        void SetFilter(TextureFilter minFilter, TextureFilter magFilter);
        void SetWrap(TextureWrap wrapS, TextureWrap wrapT);
        void SetBorderColor(float r, float g, float b, float a = 1.0f);

        // Generate mipmaps
        void GenerateMipmaps();

    protected:
        uint32_t m_textureID;
        GLenum m_target;
        uint32_t m_width;
        uint32_t m_height;
        TextureFormat m_format;
        bool m_hasMipmaps;

        void CreateTexture();
    };

    // 2D Texture class
    class Texture2D : public Texture {
    public:
        Texture2D();
        ~Texture2D() = default;

        // Create texture from data
        void CreateFromData(uint32_t width, uint32_t height,
                          TextureFormat format, TextureDataType dataType,
                          const void* data = nullptr);

        // Create texture from file
        bool LoadFromFile(const std::string& filepath, bool flipVertically = true);

        // Update texture data
        void SetData(const void* data, uint32_t x = 0, uint32_t y = 0,
                    uint32_t width = 0, uint32_t height = 0);

        // Create colored texture
        void CreateSolidColor(uint32_t width, uint32_t height,
                            float r, float g, float b, float a = 1.0f);

        // Create random color texture
        void CreateRandomColor(uint32_t width, uint32_t height,
                             uint32_t seed = 0, float alpha = 1.0f);

        // Create checkerboard pattern
        void CreateCheckerboard(uint32_t width, uint32_t height,
                              uint32_t checkerSize = 32,
                              float color1R = 1.0f, float color1G = 1.0f, float color1B = 1.0f,
                              float color2R = 0.0f, float color2G = 0.0f, float color2B = 0.0f);

        // Create noise texture
        void CreateNoise(uint32_t width, uint32_t height, uint32_t seed = 0);

        // Create gradient texture
        void CreateGradient(uint32_t width, uint32_t height,
                          float startR, float startG, float startB,
                          float endR, float endG, float endB,
                          bool horizontal = true);

        // Static factory methods
        static std::unique_ptr<Texture2D> Create(uint32_t width, uint32_t height,
                                               TextureFormat format = TextureFormat::RGBA);
        static std::unique_ptr<Texture2D> LoadFromFileStatic(const std::string& filepath);
        static std::unique_ptr<Texture2D> CreateSolidColorTexture(uint32_t width, uint32_t height, float r, float g, float b, float a = 1.0f);
        static std::unique_ptr<Texture2D> CreateRandomColorTexture(uint32_t width, uint32_t height, uint32_t seed = 0, float alpha = 1.0f);
        static std::unique_ptr<Texture2D> CreateWhite(uint32_t size = 1);
        static std::unique_ptr<Texture2D> CreateBlack(uint32_t size = 1);

    private:
        TextureDataType m_dataType;

        // Helper methods
        std::vector<uint8_t> GenerateRandomColorData(uint32_t width, uint32_t height,
                                                   uint32_t seed, float alpha);
        std::vector<uint8_t> GenerateCheckerboardData(uint32_t width, uint32_t height,
                                                     uint32_t checkerSize,
                                                     float color1R, float color1G, float color1B,
                                                     float color2R, float color2G, float color2B);
        std::vector<uint8_t> GenerateNoiseData(uint32_t width, uint32_t height, uint32_t seed);
        std::vector<uint8_t> GenerateGradientData(uint32_t width, uint32_t height,
                                                 float startR, float startG, float startB,
                                                 float endR, float endG, float endB,
                                                 bool horizontal);
    };

    // Texture manager for caching and reusing textures
    class TextureManager {
    public:
        static TextureManager& Instance();

        // Load and cache texture
        std::shared_ptr<Texture2D> LoadTexture(const std::string& name, const std::string& filepath);

        // Create and cache procedural texture
        std::shared_ptr<Texture2D> CreateSolidColorTexture(const std::string& name,
                                                         uint32_t width, uint32_t height,
                                                         float r, float g, float b, float a = 1.0f);

        std::shared_ptr<Texture2D> CreateRandomColorTexture(const std::string& name,
                                                          uint32_t width, uint32_t height,
                                                          uint32_t seed = 0, float alpha = 1.0f);

        // Get cached texture
        std::shared_ptr<Texture2D> GetTexture(const std::string& name);

        // Remove texture from cache
        void RemoveTexture(const std::string& name);

        // Clear all textures
        void Clear();

        // Get default textures
        std::shared_ptr<Texture2D> GetWhiteTexture();
        std::shared_ptr<Texture2D> GetBlackTexture();

    private:
        std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_textures;
        std::shared_ptr<Texture2D> m_whiteTexture;
        std::shared_ptr<Texture2D> m_blackTexture;

        TextureManager();
        ~TextureManager() = default;
        TextureManager(const TextureManager&) = delete;
        TextureManager& operator=(const TextureManager&) = delete;

        void CreateDefaultTextures();
    };

} // namespace agl

#endif // TEXTURE_H
