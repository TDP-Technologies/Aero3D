#ifndef AERO3D_PLATFORM_VULKAN_VULKANRENDERER_H_
#define AERO3D_PLATFORM_VULKAN_VULKANRENDERER_H_

#include "Graphics/Renderer.h"

namespace aero3d {

class VulkanRenderer : public Renderer
{
public:
    VulkanRenderer() = default;
    ~VulkanRenderer() = default;

    virtual bool Init() override;
    virtual void Shutdown() override;

    virtual void SubmitCommand(RenderCommand command) override;
    virtual void ResizeBuffers() override;

};

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_VULKANRENDERER_H_