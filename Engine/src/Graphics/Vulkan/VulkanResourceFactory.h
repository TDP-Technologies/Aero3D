#ifndef AERO3D_GRAPHICS_VULKAN_VULKANRESOURCEFACTORY_H_
#define AERO3D_GRAPHICS_VULKAN_VULKANRESOURCEFACTORY_H_

#include "Utils/Common.h"
#include "Graphics/ResourceFactory.h"

namespace aero3d {

class VulkanGraphicsDevice;

class VulkanResourceFactory : public ResourceFactory {
public:
    VulkanResourceFactory(VulkanGraphicsDevice* gd);
    ~VulkanResourceFactory();

    virtual Ref<Shader> CreateShader(ShaderDesc& desc) override;
    virtual Ref<Pipeline> CreatePipeline(PipelineDesc& desc) override;
    virtual Ref<DeviceBuffer> CreateBuffer(BufferDesc& desc) override;
    virtual Ref<Texture> CreateTexture(TextureDesc& desc) override;
    virtual Ref<TextureView> CreateTextureView(TextureViewDesc& desc) override;
    virtual Ref<Sampler> CreateSampler(SamplerDesc& desc) override;
    virtual Ref<ResourceLayout> CreateResourceLayout(ResourceLayoutDesc& desc) override;
    virtual Ref<ResourceSet> CreateResourceSet(ResourceSetDesc& desc) override;
    
private:
    VulkanGraphicsDevice* m_GraphicsDevice = nullptr;
    
};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_VULKAN_VULKANRESOURCEFACTORY_H_
