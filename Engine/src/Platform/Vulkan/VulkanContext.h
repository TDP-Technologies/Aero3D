#ifndef AERO3D_PLATFORM_VULKAN_VULKANCONTEXT_H_
#define AERO3D_PLATFORM_VULKAN_VULKANCONTEXT_H_

#include <SDL3/SDL.h>

#include "Graphics/GraphicsContext.h"

namespace aero3d {

class VulkanContext : public GraphicsContext
{
public:
    VulkanContext();
    ~VulkanContext();

    virtual bool Init(SDL_Window* window) override;
    virtual void Shutdown() override;

    virtual void SwapBuffers() override;

};

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_VULKANCONTEXT_H_