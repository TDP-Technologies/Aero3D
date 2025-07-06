#include "Graphics/Vulkan/VulkanResourceFactory.h"
#include "Graphics/Vulkan/VulkanResources.h"

namespace aero3d {

VulkanResourceFactory::VulkanResourceFactory(VulkanGraphicsDevice* gd) 
{
    m_GraphicsDevice = gd;
}

VulkanResourceFactory::~VulkanResourceFactory() 
{

}

Ref<Shader> VulkanResourceFactory::CreateShader(ShaderDesc& desc) 
{
    return std::make_shared<VulkanShader>(m_GraphicsDevice, desc);
}

Ref<Pipeline> VulkanResourceFactory::CreatePipeline(PipelineDesc& desc) 
{
    return std::make_shared<VulkanPipeline>(m_GraphicsDevice, desc);
}

Ref<DeviceBuffer> VulkanResourceFactory::CreateBuffer(BufferDesc& desc) 
{
    return std::make_shared<VulkanDeviceBuffer>(m_GraphicsDevice, desc);
}

Ref<Texture> VulkanResourceFactory::CreateTexture(TextureDesc& desc) 
{
    return std::make_shared<VulkanTexture>(m_GraphicsDevice, desc);;
}

Ref<TextureView> VulkanResourceFactory::CreateTextureView(TextureViewDesc& desc) 
{
    return std::make_shared<VulkanTextureView>(m_GraphicsDevice, desc);
}

Ref<Sampler> VulkanResourceFactory::CreateSampler(SamplerDesc& desc) 
{
    return std::make_shared<VulkanSampler>(m_GraphicsDevice, desc);
}

Ref<ResourceLayout> VulkanResourceFactory::CreateResourceLayout(ResourceLayoutDesc& desc)
{
    return std::make_shared<VulkanResourceLayout>(m_GraphicsDevice, desc);
}

Ref<ResourceSet> VulkanResourceFactory::CreateResourceSet(ResourceSetDesc& desc) 
{
    return std::make_shared<VulkanResourceSet>(m_GraphicsDevice, desc);
}

} // namespace aero3d
