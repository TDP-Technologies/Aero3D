#ifndef AERO3D_GRAPHICS_RESOURCES_H_
#define AERO3D_GRAPHICS_RESOURCES_H_

#include <string>
#include <vector>
#include <cstdint>
#include <variant>

#include "Utils/Common.h"

namespace aero3d {

enum BufferUsage 
{
    USAGE_VERTEX    = 1 << 0,
    USAGE_INDEX     = 1 << 1,
    USAGE_UNIFORM   = 1 << 2,
    USAGE_STORAGE   = 1 << 3,
    USAGE_STAGING   = 1 << 4
};

enum class IndexFormat
{
    UnsignedShort, 
    UnsignedInt
};

struct BufferDesc {
    size_t size;
    BufferUsage usage;
    bool dynamic = false;
};

class DeviceBuffer 
{
public:
    virtual ~DeviceBuffer() = default;

    BufferDesc& GetDescription() { return m_Description; };

protected:
    BufferDesc m_Description;

};

enum class TextureUsage 
{
    Sampled,
    Storage,
    RenderTarget,
    DepthStencil
};

enum class TextureFormat 
{
    RGBA8,
    BGRA8,
    RGBA8_SRGB,
    BGRA8_SRGB,
    R32FLOAT,
    D32FLOAT,
    D24S8,
    D32S8
};

struct TextureDesc 
{
    uint32_t width;
    uint32_t height;
    uint32_t depth = 1;
    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;
    TextureFormat format;
    TextureUsage usage;
    bool generateMipmaps = false;
};

class Texture 
{
public:
    virtual ~Texture() = default;

    TextureDesc& GetDescription() { return m_Description; };

protected:
    TextureDesc m_Description;

};

struct TextureViewDesc 
{
    Ref<Texture> texture;
    TextureFormat format;
    uint32_t baseMipLevel = 0;
    uint32_t mipLevels = 1;
    uint32_t baseArrayLayer = 0;
    uint32_t arrayLayers = 1;
};

class TextureView 
{
public:
    virtual ~TextureView() = default;

    virtual Ref<Texture> GetTargetTexture() = 0;

    TextureViewDesc& GetDescription() { return m_Description; }

protected:
    TextureViewDesc m_Description;

};

enum class SamplerFilter
{
    Nearest,
    Linear
};

enum class SamplerAddressMode
{
    Repeat,
    ClampToEdge,
    ClampToBorder
};

struct SamplerDesc
{
    SamplerFilter filter = SamplerFilter::Linear;
    SamplerAddressMode addressModeU = SamplerAddressMode::Repeat;
    SamplerAddressMode addressModeV = SamplerAddressMode::Repeat;
    SamplerAddressMode addressModeW = SamplerAddressMode::Repeat;
    float maxAnisotropy = 1.0f;
};

class Sampler
{
public:
    virtual ~Sampler() = default;

    SamplerDesc GetDescription() { return m_Description; }

protected:
    SamplerDesc m_Description;

};

struct FramebufferDesc 
{
    std::vector<Ref<Texture>> colorTargets;
    Ref<Texture> depthTarget = nullptr;
};

class Framebuffer 
{
public:
    virtual ~Framebuffer() = default;

    FramebufferDesc& GetDescription() { return m_Description; }

protected:
    FramebufferDesc m_Description;

};

enum ShaderStages
{
    STAGE_NONE          = 0,
    STAGE_VERTEX        = 1 << 0,
    STAGE_FRAGMENT      = 1 << 1,
    STAGE_COMPUTE       = 1 << 2,
    STAGE_GEOMETRY      = 1 << 3,
    STAGE_TESSCONTROL   = 1 << 4,
    STAGE_TESSEVAL      = 1 << 5
};

struct ShaderDesc 
{
    ShaderStages stage;
    std::string path;
    std::string entryPoint = "main";
};

class Shader 
{
public:
    virtual ~Shader() = default;

    ShaderDesc& GetDescription() { return m_Description; }

protected:
    ShaderDesc m_Description;

};

enum class ResourceKind 
{
    UniformBuffer,
    StorageBuffer,
    TextureReadOnly,
    TextureReadWrite,
    Sampler,
    CombinedImageSampler,
    TextureReadOnlyArray,
    SamplerArray,
    CombinedImageSamplerArray
};

struct ResourceBinding 
{
    uint32_t binding;
    ResourceKind kind;
    ShaderStages stages;
    uint32_t count = 1;
};

struct ResourceLayoutDesc 
{
    std::vector<ResourceBinding> bindings;
};

class ResourceLayout 
{
public:
    virtual ~ResourceLayout() = default;

    ResourceLayoutDesc& GetDescription() { return m_Description; }

protected:
    ResourceLayoutDesc m_Description;

};

using ResourceRef = std::variant<
    Ref<DeviceBuffer>,
    Ref<TextureView>,
    Ref<Sampler>,
    std::pair<Ref<TextureView>, Ref<Sampler>>,
    std::vector<Ref<TextureView>>,
    std::vector<Ref<Sampler>>,
    std::vector<std::pair<Ref<TextureView>, Ref<Sampler>>>
>;

struct ResourceSetDesc 
{
    Ref<ResourceLayout> layout;
    std::vector<ResourceRef> resources;
};

class ResourceSet 
{
public:
    virtual ~ResourceSet() = default;

    ResourceSetDesc& GetDescription() { return m_Description; }

protected:
    ResourceSetDesc m_Description;

};

enum class PrimitiveTopology 
{
    TriangleList,
    TriangleStrip,
    LineList,
    LineStrip,
    PointList
};

enum class CullMode 
{
    None,
    Front,
    Back
};

enum class FrontFace 
{
    ClockWise,
    CounterClockWise
};

enum class PolygonMode 
{
    Fill,
    Line,
    Point
};

enum class VertexFormat 
{
    Float, Float2, Float3, Float4, Int, Int2, Int3, Int4,
    Bool, Mat2, Mat3, Mat4
};

struct VertexAttributeDesc 
{
    uint32_t location;
    uint32_t binding;
    VertexFormat format;
    uint32_t offset;
};

struct VertexBindingDesc 
{
    uint32_t binding;
    uint32_t stride;
    bool perInstance = false;
};

struct VertexLayoutDesc 
{
    std::vector<VertexBindingDesc> bindings;
    std::vector<VertexAttributeDesc> attributes;
};

struct PipelineDesc 
{
    Ref<Shader> vertexShader;
    Ref<Shader> fragmentShader;

    Ref<ResourceLayout> resourceLayout;

    VertexLayoutDesc vertexLayout;

    PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    CullMode cullMode = CullMode::Back;
    FrontFace frontFace = FrontFace::CounterClockWise;
    PolygonMode polygonMode = PolygonMode::Fill;

    bool depthTest = true;
    bool depthWrite = true;
};

class Pipeline 
{
public:
    virtual ~Pipeline() = default;

    PipelineDesc& GetDescription() { return m_Description; }

protected:
    PipelineDesc m_Description;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_RESOURCES_H_