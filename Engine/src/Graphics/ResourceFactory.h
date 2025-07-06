#ifndef AERO3D_GRAPHICS_RESOURCEFACTORY_H_
#define AERO3D_GRAPHICS_RESOURCEFACTORY_H_

#include "Graphics/Resources.h"
#include "Utils/Common.h"

namespace aero3d {

class ResourceFactory {
public:
    virtual ~ResourceFactory() = default;

    virtual Ref<Shader> CreateShader(ShaderDesc& desc) = 0;
    virtual Ref<Pipeline> CreatePipeline(PipelineDesc& desc) = 0;
    virtual Ref<DeviceBuffer> CreateBuffer(BufferDesc& desc) = 0;
    virtual Ref<Texture> CreateTexture(TextureDesc& desc) = 0;
    virtual Ref<TextureView> CreateTextureView(TextureViewDesc& desc) = 0;
    virtual Ref<Sampler> CreateSampler(SamplerDesc& desc) = 0;
    virtual Ref<ResourceLayout> CreateResourceLayout(ResourceLayoutDesc& desc) = 0;
    virtual Ref<ResourceSet> CreateResourceSet(ResourceSetDesc& desc) = 0;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_RESOURCEFACTORY_H_