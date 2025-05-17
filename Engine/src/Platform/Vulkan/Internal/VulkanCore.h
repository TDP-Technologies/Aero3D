#ifndef AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCORE_H_
#define AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCORE_H_

#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>

namespace aero3d {

class VulkanCore
{
public:
    VulkanCore();
    ~VulkanCore();

    bool Init(SDL_Window* window);
    void Shutdown();

private:
    bool CreateInstance();

private:
    SDL_Window* m_Window;
    VkInstance m_Instance;

};

extern VulkanCore* g_VulkanCore;

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCORE_H_