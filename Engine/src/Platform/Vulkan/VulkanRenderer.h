#ifndef AERO3D_PLATFORM_VULKAN_VULKANRENDERER_H_
#define AERO3D_PLATFORM_VULKAN_VULKANRENDERER_H_

#include "Graphics/Renderer.h"

namespace aero3d {

class VulkanRenderer : public Renderer
{
public:
    VulkanRenderer() = default;
    ~VulkanRenderer() = default;

    virtual void Init(SDL_Window* window, int width, int height) override;
    virtual void Shutdown() override;

    virtual void BeginFrame() override;
    virtual void EndFrame() override;

    virtual void SubmitCommand(RenderCommand command) override;
    virtual void ResizeBuffers() override;

};

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_VULKANRENDERER_H_