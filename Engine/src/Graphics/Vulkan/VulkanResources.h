#ifndef AERO3D_GRAPHICS_VULKAN_VULKANRESOURCES_H_
#define AERO3D_GRAPHICS_VULKAN_VULKANRESOURCES_H_

#include <vector>

#include <volk.h>
#include <shaderc/shaderc.hpp>

#include "Graphics/Resources.h"

namespace aero3d {

class VulkanGraphicsDevice;

class VulkanDeviceBuffer : public DeviceBuffer {
public:
    VulkanDeviceBuffer(VulkanGraphicsDevice* gd, BufferDesc desc);
    ~VulkanDeviceBuffer();

public:
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkMemoryRequirements memoryRequirements;
    uint32_t size = 0;

private:
    VulkanGraphicsDevice* m_GraphicsDevice = nullptr;

};

inline TextureFormat FromVkFormat(VkFormat format) 
{
    switch (format) 
    {
        case VK_FORMAT_R8G8B8A8_UNORM:     return TextureFormat::RGBA8;
        case VK_FORMAT_B8G8R8A8_UNORM:     return TextureFormat::BGRA8;
        case VK_FORMAT_R8G8B8A8_SRGB:      return TextureFormat::RGBA8_SRGB;
        case VK_FORMAT_B8G8R8A8_SRGB:      return TextureFormat::BGRA8_SRGB;
        case VK_FORMAT_R32_SFLOAT:         return TextureFormat::R32FLOAT;
        case VK_FORMAT_D32_SFLOAT:         return TextureFormat::D32FLOAT;
        case VK_FORMAT_D24_UNORM_S8_UINT:    return TextureFormat::D24S8;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:   return TextureFormat::D32S8;
        default:                           return TextureFormat::RGBA8;
    }
}

class VulkanTexture : public Texture {
public:
    VulkanTexture(VulkanGraphicsDevice* gd, TextureDesc desc);
    VulkanTexture(VulkanGraphicsDevice* gd, TextureDesc desc, VkImage existingImage);
    ~VulkanTexture();

public:
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkFormat vkFormat = VK_FORMAT_UNDEFINED;

    uint32_t width = 0;
    uint32_t height = 0;
    TextureFormat format = TextureFormat::RGBA8;
    TextureUsage usage = TextureUsage::Storage;

    bool fromExisting = false;

private:
    VulkanGraphicsDevice* m_GraphicsDevice = nullptr;

};

class VulkanTextureView : public TextureView
{
public:
    VulkanTextureView(VulkanGraphicsDevice* gd, TextureViewDesc desc);
    ~VulkanTextureView();

    virtual Ref<Texture> GetTargetTexture() override;
    
public:
    Ref<VulkanTexture> texture;
    VkImageView imageView = VK_NULL_HANDLE;

private:
    VulkanGraphicsDevice* m_GraphicsDevice = nullptr;
    
};

class VulkanSampler : public Sampler
{
public:
    VulkanSampler(VulkanGraphicsDevice* gd, SamplerDesc& desc);
    ~VulkanSampler();

public:
    VkSampler sampler = VK_NULL_HANDLE;

private:
    VulkanGraphicsDevice* m_GraphicsDevice = nullptr;

};

class VulkanFramebuffer : public Framebuffer {
public:
    VulkanFramebuffer(VulkanGraphicsDevice* gd, FramebufferDesc desc);
    ~VulkanFramebuffer();

public:
    VkExtent2D renderArea;

    std::vector<Ref<VulkanTexture>> frames;
    std::vector<VkImageView> imageViews;
    Ref<VulkanTexture> depthStencil = nullptr;
    VkImageView depthStencilImageView = VK_NULL_HANDLE;

private:
    VulkanGraphicsDevice* m_GraphicsDevice = nullptr;

};

class VulkanShader : public Shader {
public:
    VulkanShader(VulkanGraphicsDevice* gd, ShaderDesc desc);
    ~VulkanShader();

public:
    VkShaderModule shaderModule = VK_NULL_HANDLE;

private:
    std::vector<uint32_t> CompileGLSL(const std::string& source, shaderc_shader_kind kind, const std::string& name);

private:
    VulkanGraphicsDevice* m_GraphicsDevice = nullptr;

};

class VulkanResourceLayout : public ResourceLayout {
public:
    VulkanResourceLayout(VulkanGraphicsDevice* gd, ResourceLayoutDesc desc);
    ~VulkanResourceLayout();

public:
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    std::vector<ResourceBinding> bindings;

private:
    VulkanGraphicsDevice* m_GraphicsDevice = nullptr;

};

class VulkanResourceSet : public ResourceSet {
public:
    VulkanResourceSet(VulkanGraphicsDevice* gd, ResourceSetDesc desc);
    ~VulkanResourceSet();

public:
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

private:
    void PrepareBufferWrite(const ResourceBinding& binding, void* resource,
        VkWriteDescriptorSet& write, VkDescriptorBufferInfo& bufferInfo);

    void PrepareImageWrite(const ResourceBinding& binding, void* resource,
        VkWriteDescriptorSet& write, VkDescriptorImageInfo& imageInfo);

    void PrepareSamplerWrite(const ResourceBinding& binding, void* resource,
        VkWriteDescriptorSet& write, VkDescriptorImageInfo& imageInfo);

private:
    VulkanGraphicsDevice* m_GraphicsDevice = nullptr;
    
};

class VulkanPipeline : public Pipeline {
public:
    VulkanPipeline(VulkanGraphicsDevice* gd, PipelineDesc desc);
    ~VulkanPipeline();

public:
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

private:
    VulkanGraphicsDevice* m_GraphicsDevice = nullptr;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_VULKAN_VULKANRESOURCES_H_
